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

#include "CloneDetector.h"

// Construct from a container of all tokens encountered
CloneDetector::CloneDetector(const TokenContainer &tc, unsigned clone_length)
    : token_container(tc), clone_length(clone_length)
{
    SeenTokens::set_token_container(&tc);
    SeenTokens::set_clone_length(clone_length);

    for (auto file : tc.file_view())
        for (auto line : file.line_view()) {

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
    for (auto clone_group : clones) {
        for (auto member : clone_group) {
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
    for (auto it : seen) {
        auto leader = it.first;

        // Extent of clone leader data to line end
        auto leader_extension_begin = token_container.offset_begin(leader.get_file_id(), leader.get_begin_token_offset() + clone_length);
        auto leader_line_end = token_container.line_from_offset_end(leader.get_file_id(), leader.get_begin_token_offset() + clone_length - 1);
        auto leader_extension_length = leader_line_end - leader_extension_begin;
        // Create a group of clones that are the same till the end of the line
        std::list<Clone> group;
        int group_size = 0;
        for (auto member : it.second) {
            auto member_extension_begin = token_container.offset_begin(member.get_file_id(), member.get_begin_token_offset() + clone_length);
            auto member_end_line_offset = member.get_begin_token_offset() + clone_length - 1;
            auto member_line_end = token_container.line_from_offset_end(member.get_file_id(), member_end_line_offset);

            // Unequal line length extensions
            if (member_line_end - member_extension_begin != leader_extension_length)
                continue;
            // Unequal extension contents
            if (!std::equal(leader_extension_begin, leader_line_end, member_extension_begin))
                continue;
            group.emplace_back(Clone(member.get_file_id(),
                        member.get_begin_token_offset(), member_end_line_offset));
            group_size++;
        }
        if (group_size > 1)
            clones.push_back(std::move(group));
    }
}
