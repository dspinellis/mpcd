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

#include <map>
#include <set>
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
private:
    /*
     * Conserve 8 bytes by substituting the actual types
     * TokenContainer::file_id_type file_id;
     * FileData::token_offset_type token_offset;
     * with smaller ones.
     */
    file_id_type file_id;
    token_offset_type token_offset;
public:
    CloneLocation(TokenContainer::file_id_type file_id,
            FileData::token_offset_type token_offset) :
        file_id((file_id_type)file_id),
        token_offset((token_offset_type)token_offset) {}

    friend bool operator<(const CloneLocation& lhs, const CloneLocation& rhs);

    friend std::ostream& operator<<(std::ostream& os, const CloneLocation &l) {
        os << l.file_id << '.' << l.token_offset;
        return os;
    }

    TokenContainer::file_id_type get_file_id() const {
        return TokenContainer::file_id_type(file_id);
    }

    FileData::token_offset_type get_token_offset() const {
        return FileData::token_offset_type(token_offset);
    }
};

inline bool operator<(const CloneLocation& lhs, const CloneLocation& rhs) {
    return lhs.file_id < rhs.file_id || lhs.token_offset < rhs.token_offset;
}

class CloneDetector {
public:
    typedef const std::vector<FileData::token_type> seen_tokens_type;
    typedef std::set<CloneLocation> seen_locations_type;

private:
    // Tokens that have been encountered in the examined code
    std::map<seen_tokens_type, seen_locations_type> seen;

    // Container of all tokens
    const TokenContainer &token_container;

    // Minimum length of clones to be detected
    unsigned clone_length;

    // Add a new token sequence that has been encountered
    void insert(const seen_tokens_type &tokens, const CloneLocation location) {
        auto it = seen.find(tokens);
        if (it == seen.end())
            seen.insert(it, std::make_pair(tokens, seen_locations_type{location}));
        else
            it->second.insert(location);
    }
public:
    CloneDetector(const TokenContainer &tc, unsigned clone_length);
    void prune_non_clones();
    void report() const;

    // Return the number of sites for potential clones (for testing)
    int get_number_of_sites() { return seen.size(); }

    // Return the number of found clones (for testing)
    int get_number_of_clones() {
        int nclones = 0;
        for (auto it : seen) {
            size_t nelem = it.second.size();
            if (nelem > 1)
                nclones += nelem;
        }
        return nclones;
    }
};
