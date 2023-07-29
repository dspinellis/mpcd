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
            SeenTokens clone_candidates(file.get_id(), file.line_offset(line));

            insert(clone_candidates, CloneLocation(file.get_id(), file.line_offset(line)));
        }
}

// Prune-away recorded tokens not associated with clones
void
CloneDetector::prune_non_clones() {
    for (auto it = clone_candidates.begin(); it != clone_candidates.end();)
        if (it->second.size() == 1)
            it = clone_candidates.erase(it);
        else
            ++it;
}

// Report found clones in text format
void
CloneDetector::report_text() const {
    for (const auto& clone_group : clones) {
        std::cout << clone_group.size() << "\t";
        std::cout << clone_group.front().size() << std::endl;
        for (const auto& member : clone_group) {
            auto member_file_id = member.get_file_id();
            std::cout << token_container.get_token_line_number(member_file_id, member.get_begin_token_offset()) + 1 << '\t';
            std::cout << token_container.get_token_line_number(member_file_id, member.get_end_token_offset() - 1) + 1 << '\t';
            std::cout << token_container.get_file_name(member_file_id) << std::endl;
        }
        std::cout << std::endl;
    }
}

// Escape characters to make a string valid JSON string
std::string
escape_json_string(const std::string& input) {
    std::string result;

    for (char c : input) {
        switch (c) {
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        default:
            result += c;
        }
    }
    return result;
}

// Report found clones in JSON format
void
CloneDetector::report_json() const {
    std::cout << "[" << std::endl;
    // For each clone group
    for (auto cg_it = clones.begin(); cg_it != clones.end(); ++cg_it) {
        std::cout << "  {" << std::endl;
        std::cout << "    \"tokens\": "
            << cg_it->front().size() << ',' << std::endl;
        std::cout << "    \"groups\": [" << std::endl;

        // For each member of the clone group
        for (auto member_it = cg_it->begin(); member_it != cg_it->end(); ++member_it) {
            std::cout << "      {" << std::endl;
            std::cout << "        \"start\": "
                << token_container.get_token_line_number(member_it->get_file_id(), member_it->get_begin_token_offset()) + 1
                << ',' << std::endl;

            std::cout << "        \"end\": "
                << token_container.get_token_line_number(member_it->get_file_id(), member_it->get_end_token_offset()) + 1
                << ',' << std::endl;

            std::cout << "        \"filepath\": \""
                << escape_json_string(
                        token_container.get_file_name(member_it->get_file_id()))
                << '"' << std::endl;

            if (std::next(member_it) == cg_it->end())
                std::cout << "      }" << std::endl;
            else
                std::cout << "      }," << std::endl;
        }
        std::cout << "    ]" << std::endl;
        if (std::next(cg_it) == clones.end())
            std::cout << "  }" << std::endl;
        else
            std::cout << "  }," << std::endl;
    }
    std::cout << "]" << std::endl;
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

/*
 * Convert partial candidate clones in "clone_candidates" into full clones
 * in "clone", based on clone lines.
 */
void
CloneDetector::create_line_region_clones()
{
    for (const auto& it : clone_candidates) {
        auto leader = it.first;

        // Extent of clone leader data to line end
        auto leader_file_id = leader.get_file_id();
        auto leader_extension_begin = token_container.offset_begin(leader_file_id, leader.get_begin_token_offset() + clone_length);
        auto leader_line_end = token_container.line_from_offset_end(leader_file_id, leader.get_begin_token_offset() + clone_length - 1);
        auto leader_extension_length = leader_line_end - leader_extension_begin;
        // Create a group of clones that are the same till the end of the line
        std::list<Clone> group;
        for (const auto& member : it.second) {
            auto member_file_id = member.get_file_id();
            auto member_extension_begin = token_container.offset_begin(member_file_id, member.get_begin_token_offset() + clone_length);
            auto offset_in_last_line = member.get_begin_token_offset() + clone_length - 1;
            auto member_line_end = token_container.line_from_offset_end(member_file_id, offset_in_last_line);

            // Unequal line length extensions
            if (member_line_end - member_extension_begin != leader_extension_length)
                continue;
            // Unequal extension contents
            if (!std::equal(leader_extension_begin, leader_line_end, member_extension_begin))
                continue;
            auto member_end_offset = member.get_begin_token_offset() + clone_length + leader_extension_length;
            group.emplace_back(Clone(member_file_id,
                        member.get_begin_token_offset(), member_end_offset));
        }
        if (group.size() > 1)
            clones.push_back(std::move(group));
    }
}

/*
 * Convert partial candidate clones associated with a leader and
 * its members into full clones in "clone", based on clone blocks.
 * Attempt to start the clone "offset" tokens back from the recorded
 * start to cater for clone blocks starting on an (otherwise differing)
 * previous line.
 * Return true on success false on failure to add a clone
 */
bool
CloneDetector::create_block_region_clone(const SeenTokens& leader,
        const seen_locations_type& members, int offset)
{
    auto leader_begin_token_offset = leader.get_begin_token_offset();
    if (leader_begin_token_offset + offset < 0)
        return false;  // Can't deal with this offset

    auto leader_file_id = leader.get_file_id();
    auto leader_begin = token_container.offset_begin(leader_file_id, leader_begin_token_offset);
    auto leader_end = leader_begin + clone_length;
    // Find block begin within the clone region
    auto leader_block_begin = leader_begin + offset;
    for (; leader_block_begin < leader_end; ++leader_block_begin)
        if (*leader_block_begin == '{')
            break;
    if (leader_block_begin == leader_end)
        return false;  // This candidate does not contain a code block; skip it

    // Find matching end
    int block_depth = 0;
    auto leader_block_end = leader_block_begin;
    auto leader_file_end = token_container.file_end(leader_file_id);
    for (; leader_block_end < leader_file_end; ++leader_block_end) {
        switch (*leader_block_end) {
        case '{': ++block_depth; break;
        case '}': --block_depth; break;
        }
        if (block_depth == 0)
            break;
    }

    if (leader_block_end == leader_file_end)
        return false;  // No block end found

    ++leader_block_end;  // Point past closing brace to include it

    if (leader_block_end - leader_block_begin < clone_length)
        return false;  // Block smaller than the specified cline length

    auto block_begin_offset = leader_block_begin - leader_begin;
    auto block_end_offset = leader_block_end - leader_begin;

    /*
     * If the block is within the original detected clone span, just add
     * all its members as clones.
     */
    if (leader_block_begin >= leader_begin && leader_block_end < leader_end) {
        std::list<Clone> group;
        for (const auto& member : members) {
            auto member_begin = member.get_begin_token_offset();
            auto member_file_id = member.get_file_id();

            group.emplace_back(Clone(member_file_id,
                        member_begin + block_begin_offset,
                        member_begin + block_end_offset));
        }
        clones.push_back(std::move(group));
        return true;
    }

    // Create a group of clones that are the same till the end of the block
    std::list<Clone> group;
    auto block_extension_length = leader_block_end - leader_end;
    auto leader_extension_begin = leader_begin + clone_length;
    for (const auto& member : members) {
        auto member_begin_token_offset = member.get_begin_token_offset();
        auto member_file_id = member.get_file_id();

        // Unequal offset contents
        if (offset) {
            if (member_begin_token_offset + offset < 0)
                continue;  // Can't deal with this offset

            auto member_begin = token_container.offset_begin(member_file_id, member_begin_token_offset);
            if (!std::equal(leader_begin + offset, leader_begin, member_begin + offset))
                continue;
        }

        auto member_extension_begin = token_container.offset_begin(member_file_id, member_begin_token_offset + clone_length);

        // Block past member's end
        if (member_extension_begin + block_extension_length > token_container.file_end(member_file_id))
            continue;

        // Unequal extension contents
        if (!std::equal(leader_extension_begin, leader_block_end, member_extension_begin))
            continue;

        auto member_end_offset = member.get_begin_token_offset() + clone_length + block_extension_length;
        group.emplace_back(Clone(member_file_id,
                    member.get_begin_token_offset(), member_end_offset));
    }

    if (group.size() > 1) {
        clones.push_back(std::move(group));
        return true;
    }
    return false;
}

/*
 * Convert partial candidate clones in "clone_candidates" into full clones
 * in "clone", based on clone blocks.
 */
void
CloneDetector::create_block_region_clones()
{
    for (const auto& it : clone_candidates)
        // First try the previous token for blocks starting on an otherwise
        // different previous line
        for (int offset = -1; offset <= 0; ++offset)
            if (create_block_region_clone(it.first, it.second, offset))
                break;
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
            for (++member; member != clone_group.end(); ++member)
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
