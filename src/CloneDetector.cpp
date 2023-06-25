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

#include <algorithm>
#include <set>

#include "CloneDetector.h"

// Construct from a container of all tokens encountered
CloneDetector::CloneDetector(const TokenContainer &tc, unsigned clone_length)
    : token_container(tc), clone_length(clone_length)
{
    SeenTokens::set_token_container(&tc);
    SeenTokens::set_clone_length(clone_length);

    for (const auto& file : tc.file_view())
        for (const auto& line : file.line_view()) {

            // Skip empty lines; nothing to add
            if (file.line_is_empty(line))
                continue;

            // Skip end sequences of insufficient length
            if (file.remaining_tokens(line) < clone_length)
                continue;

            // Create an identifier for the token sequence to add
            SeenTokens seen(file.get_id(), file.line_offset(line));

            insert(seen, CloneLocation(file.get_id(), file.line_offset(line)));
        }
}

// Prune-away recorded tokens not associated with clones
void
CloneDetector::prune_non_clones() {
    for (auto it = seen.begin(); it != seen.end();)
        if (it->second.size() == 1)
            it = seen.erase(it);
        else
            ++it;
}

// Report found clones
void
CloneDetector::report() const {
    for (const auto& clone_group : clones) {
        std::cout << clone_group.size() << "\t";
        std::cout << clone_group.front().size() << std::endl;
        for (const auto& member : clone_group) {
            std::cout << token_container.get_token_line_number(member.get_file_id(), member.get_begin_token_offset()) + 1 << '\t';
            std::cout << token_container.get_token_line_number(member.get_file_id(), member.get_end_token_offset()) + 1 << '\t';
            std::cout << token_container.get_file_name(member.get_file_id()) << std::endl;
        }
        std::cout << std::endl;
    }
}

// Container holding the encountered tokens
const TokenContainer* SeenTokens::token_container;

// Length of identified token sequences
unsigned SeenTokens::clone_length;

// Return true if the tokens identified on the lhs < than the rhs ones
bool
operator<(const SeenTokens& lhs, const SeenTokens& rhs) {
    const TokenContainer* tc = SeenTokens::get_token_container();
    unsigned clone_length  = SeenTokens::get_clone_length();

    auto lhs_it = tc->offset_begin(lhs.get_file_id(), lhs.get_begin_token_offset());
    auto rhs_it = tc->offset_begin(rhs.get_file_id(), rhs.get_begin_token_offset());
    return std::lexicographical_compare(lhs_it, lhs_it + clone_length,
            rhs_it, rhs_it + clone_length);
}

// Create partial candidate clones in "seen" into full clones in "clone"
void
CloneDetector::create_line_region_clones()
{
    for (const auto& it : seen) {
        auto leader = it.first;

        // Extent of clone leader data to line end
        auto leader_extension_begin = token_container.offset_begin(leader.get_file_id(), leader.get_begin_token_offset() + clone_length);
        auto leader_line_end = token_container.line_from_offset_end(leader.get_file_id(), leader.get_begin_token_offset() + clone_length - 1);
        auto leader_extension_length = leader_line_end - leader_extension_begin;
        // Create a group of clones that are the same till the end of the line
        std::list<Clone> group;
        int group_size = 0;
        for (const auto& member : it.second) {
            auto member_extension_begin = token_container.offset_begin(member.get_file_id(), member.get_begin_token_offset() + clone_length);
            auto offset_in_last_line = member.get_begin_token_offset() + clone_length - 1;
            auto member_line_end = token_container.line_from_offset_end(member.get_file_id(), offset_in_last_line);

            // Unequal line length extensions
            if (member_line_end - member_extension_begin != leader_extension_length)
                continue;
            // Unequal extension contents
            if (!std::equal(leader_extension_begin, leader_line_end, member_extension_begin))
                continue;
            auto member_end_offset = member.get_begin_token_offset() + clone_length + leader_extension_length;
            group.emplace_back(Clone(member.get_file_id(),
                        member.get_begin_token_offset(), member_end_offset));
            group_size++;
        }
        if (group_size > 1)
            clones.push_back(std::move(group));
    }
}

// Extend clones to subsequent lines as much as possible
void
CloneDetector::extend_clones()
{
    for (auto& clone_group : clones) {
        // Extend group members as much as possible
        for (;;) {
            auto& leader(clone_group.front());
            auto leader_end_token = get_end_token(leader);
            auto member = clone_group.begin();
            for (++member; member != clone_group.end(); member++)
                if (get_end_token(*member) != leader_end_token)
                    break;
            if (member != clone_group.end())
                break;  // Difference found; stop advancing
            // Extend all group's members by one token
            for (auto& member : clone_group)
                member.extend_by_one();
        }
        // Trim all members to preceding end of line
        for (auto& member : clone_group)
            trim_to_eol(member);
    }
}

/*
 * Remove clone groups whose members are entirely shadowed by others.
 *
 * 1. Create a set ordered by and clone location.
 * 2. Traverse the set, marking elements completely shadowed by their
 *    predecessor in the same file as shadowed.
 * 3. Traverse the clone groups removing those that have all their elements
 *    shadowed.
 */
void
CloneDetector::remove_shadowed_groups()
{
    struct Compare {
        bool operator() (const Clone* lhs, const Clone* rhs) const {
            return *lhs < *rhs;
        }
    };

    std::set<Clone*, Compare> ordered_clones;

    // Create a set ordered by and clone location.
    for (auto& clone_group : clones)
        for (auto& clone : clone_group)
            ordered_clones.insert(&clone);

    // Mark clones
    Clone* shadow = nullptr;
    for (auto& clone : ordered_clones) {
        // Clear shadow when crossing file boundary
        if (shadow && shadow->get_file_id() != clone->get_file_id())
            shadow = nullptr;

        if (shadow && clone->is_shadowed(*shadow))
            clone->set_shadowed();
        shadow = clone;
    }

    // Remove entirely shadowed clone groups
    for (auto group_it = clones.begin(); group_it != clones.end(); ) {
        auto clone_it = group_it->begin();
        for (; clone_it != group_it->end(); ++clone_it)
            if (!clone_it->is_shadowed())
                break;

        // See if entirely shadowed and erase
        if (clone_it == group_it->end())
            group_it = clones.erase(group_it);
        else
            ++group_it;
    }
}
