/*-
 * Copyright 2023 Diomidis Spinellis
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * A data structure and algorithms for detecting clones
 */

#pragma once

#include <list>
#include <map>
#include <vector>
#include <ostream>

#include "TokenContainer.h"

/*
 * The location of a potential clone, identified through the file
 * and token offset.
 */
class CloneLocation {
public:
    typedef unsigned int file_id_type;
    typedef unsigned int token_offset_type;

protected:
    /*
     * Conserve 8 bytes by substituting the actual types
     * TokenContainer::file_id_type file_id;
     * FileData::token_offset_type begin_offset;
     * with smaller ones.
     */
    file_id_type file_id;
    token_offset_type begin_offset;

public:
    // Construct from a file id and token offset
    CloneLocation(TokenContainer::file_id_type file_id,
            FileData::token_offset_type begin_offset) :
        file_id((file_id_type)file_id),
        begin_offset((token_offset_type)begin_offset) {}

    friend bool operator<(const CloneLocation& lhs, const CloneLocation& rhs) {
        return lhs.file_id < rhs.file_id || lhs.begin_offset < rhs.begin_offset;
    }

    friend std::ostream& operator<<(std::ostream& os, const CloneLocation &l) {
        os << l.file_id << '.' << l.begin_offset;
        return os;
    }

    TokenContainer::file_id_type get_file_id() const {
        return TokenContainer::file_id_type(file_id);
    }

    FileData::token_offset_type get_begin_token_offset() const {
        return FileData::token_offset_type(begin_offset);
    }
};

// The location and extent of a clone
class Clone: public CloneLocation {
private:
    // Where the clone ends
    token_offset_type end_offset;

    // True if the clone's region is entirely shadowed by another
    bool shadowed;

public:
    Clone(TokenContainer::file_id_type file_id,
            FileData::token_offset_type begin_offset,
            FileData::token_offset_type end_offset) :
        CloneLocation(file_id, begin_offset),
        end_offset(token_offset_type(end_offset)), shadowed(false) {}

    // Return the clone's size in tokens
    std::size_t size() const {
        return end_offset - begin_offset;
    }

    friend std::ostream& operator<<(std::ostream& os, const Clone &l) {
        os << l.file_id << '.' << l.begin_offset << '-' << l.end_offset;
        return os;
    }

    // Return the clone's end offset
    FileData::token_offset_type get_end_token_offset() const {
        return FileData::token_offset_type(end_offset);
    }

    void set_end_token_offset(FileData::token_offset_type offset) {
        end_offset = offset;
    }

    // Extend the clone's coverage by one token
    void extend_by_one() {
        ++end_offset;
    }

    // Return true if the clone is entirely shadowed by the passed one
    bool is_shadowed(const Clone& shadow) const {
        return shadow.begin_offset <= begin_offset
            && shadow.end_offset >= end_offset;
    }

    // Return true if the clone has been marked shadowed
    bool is_shadowed() const {
        return shadowed;
    }

    void set_shadowed() {
        shadowed = true;
    }
};

/*
 * A single arbitrary clone location acts as a template for identifying
 * all identical to it tokens that have been encountered.
 * It differs from CloneLocation in that its comparison function compares
 * tokens rather than the location, so different locations with the same
 * tokens compare as equal.
 * For the comparison to work its pointer to the token container must be set.
 */
class SeenTokens : public CloneLocation {
private:
    // Container holding the encountered tokens
    static const TokenContainer* token_container;

    // Length of identified token sequences
    static unsigned clone_length;
public:
    // Construct from a file id and token offset
    SeenTokens(TokenContainer::file_id_type file_id,
            FileData::token_offset_type token_offset) :
        CloneLocation(file_id, token_offset) {}

    static void set_token_container(const TokenContainer* tc) {
        token_container = tc;
    }
    static const TokenContainer* get_token_container() {
        return token_container;
    }

    static void set_clone_length(unsigned cl) { clone_length = cl; }
    static unsigned get_clone_length() { return clone_length; }

    friend bool operator<(const SeenTokens& lhs, const SeenTokens& rhs);
};

class CloneDetector {
public:
    typedef std::vector<CloneLocation> seen_locations_type;

private:
    // Container of all tokens
    const TokenContainer &token_container;

    // Tokens that have been encountered in the examined code (token_container)
    std::map<SeenTokens, seen_locations_type> seen;

    // Minimum length of clones to be detected
    unsigned clone_length;

    // List of found clones
    std::list<std::list<Clone>> clones;

    // Add a new token sequence that has been encountered
    void insert(const SeenTokens &tokens, const CloneLocation location) {
        auto it = seen.find(tokens);
        if (it == seen.end())
            seen.insert(it, std::make_pair(tokens, seen_locations_type{location}));
        else
            it->second.push_back(location);
    }

    /*
     * Return the token immediately past the end (i.e. at the end)
     * of the specified clone.
     * If the end coincides with the file's end, return 0.
     */
    FileData::token_type get_end_token(const Clone& clone) {
        return token_container.get_token(clone.get_file_id(),
                clone.get_end_token_offset());
    }

    // Trim the clone extent to the nearest EOL
    void trim_to_eol(Clone& clone) {
        clone.set_end_token_offset(token_container.get_preceding_eol_offset(
                    clone.get_file_id(),
                    clone.get_end_token_offset()));
    }

public:
    CloneDetector(const TokenContainer &tc, unsigned clone_length);

    // Prune-away recorded tokens not associated with clones
    void prune_non_clones();

    // Convert candidate clones from "seen" into "clone"
    void create_line_region_clones();

    // Extend clones to subsequent lines if possible
    void extend_clones();

    // Remove clone groups whose members are entirely shadowed by others
    void remove_shadowed_groups();

    // Return a read-only view of all clones
    ConstCollectionView<decltype(clones)> clone_view() const {
        return clones;
    }

    // Report found clones
    void report() const;

    // Return the number of sites for potential clones (for testing)
    int get_number_of_seen_sites() { return seen.size(); }

    // Return the number of potential clones found (for testing)
    int get_number_of_seen_clones() {
        int nclones = 0;
        for (const auto& it : seen) {
            size_t nelem = it.second.size();
            if (nelem > 1)
                nclones += nelem;
        }
        return nclones;
    }

    // Return the number of actual clone groups
    int get_number_of_clone_groups() { return clones.size(); }

    int get_number_of_clones() {
        int nclones = 0;
        for (const auto& it : clones)
            nclones += it.size();
        return nclones;
    }

    /*
     * Return the total number of clone tokens covered by clone groups.
     * Each clone group is counted once.
     */
    std::size_t get_number_of_clone_tokens()
    {
        std::size_t ntokens = 0;
        for (auto& clone_group : clones)
            ntokens += clone_group.front().size();
        return ntokens;
    }
};
