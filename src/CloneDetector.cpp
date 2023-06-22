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

#include "CloneDetector.h"

// Construct from a container of all tokens encountered
CloneDetector::CloneDetector(const TokenContainer &tc, unsigned clone_size)
{
    for (auto file : tc.file_view())
        for (auto line : file.line_view()) {

            // Skip empty lines; nothing to add
            if (file.line_is_empty(line))
                continue;

            // Skip end sequences of insufficient length
            if (file.remaining_tokens(line) < clone_size)
                continue;

            // Create vector of token sequence to add
            auto begin = file.line_begin(line);
            seen_tokens_type seen(begin, begin + clone_size);

            insert(seen, CloneLocation(file.get_id(), file.line_offset(line)));
        }
}
