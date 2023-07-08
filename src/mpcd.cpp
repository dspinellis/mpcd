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
 * Modular performant clone detector
 *
 */

#include <cstring>
#include <string>
#include <iostream>
#include <ostream>

#include <errno.h>
#include <unistd.h>

#include "TokenContainer.h"
#include "CloneDetector.h"

const char version[] = "1.1.2";

// Identify clones among the tokenized input stream
int
main(int argc, char * const argv[])
{
    int opt;
    int clone_tokens = 15; // Minimum number of same tokens to identify a clone
    bool verbose = false;
    bool json = false;

    while ((opt = getopt(argc, argv, "jn:Vv")) != -1)
        switch (opt) {
        case 'j':
            json = true;
            break;
        case 'n':
            clone_tokens = std::atoi(optarg);
            if (clone_tokens == 0) {
                std::cerr << "Invalid token number specified" << std::endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 'V':
            std::cout << "mpcd " << version << std::endl;
            exit(EXIT_SUCCESS);
        case 'v':
            verbose = true;
            break;
        default: /* ? */
            std::cerr << "Usage: " << argv[0] <<
                " [-jvV] [-n tokens]" << std::endl;
            exit(EXIT_FAILURE);
        }

    if (verbose)
        std::cerr << "Reading input tokens." << std::endl;
    TokenContainer tc(std::cin);
    if (verbose)
        std::cerr << "Read "
            << tc.file_size() << " files, "
            << tc.line_size() << " lines, "
            << tc.token_size() << " tokens."
            << std::endl;

    CloneDetector cd(tc, clone_tokens);
    if (verbose)
        std::cerr << "Identified "
            << cd.get_number_of_seen_clones() << " potential clones in "
            << cd.get_number_of_seen_sites() << " total sites."
            << std::endl;

    cd.prune_non_clones();
    if (verbose)
        std::cerr << "Pruned non-clone sites leaving "
            << cd.get_number_of_seen_sites() << " sites."
            << std::endl;

    cd.create_line_region_clones();
    if (verbose) {
        std::cerr << "Identified " << cd.get_number_of_clones()
            << " clones in " << cd.get_number_of_clone_groups() << " groups."
            << std::endl;
        if (cd.get_number_of_clone_groups() > 0)
            std::cerr << "Each clone element is on average "
                << cd.get_number_of_clone_tokens() / cd.get_number_of_clone_groups()
                << " tokens long."
                << std::endl;
    }

    cd.extend_clones();
    if (verbose) {
        std::cerr << "Extended clones to their maximal size." << std::endl;
        if (cd.get_number_of_clone_groups() > 0)
            std::cerr << "Each clone element is on average "
                << cd.get_number_of_clone_tokens() / cd.get_number_of_clone_groups()
                << " tokens long."
                << std::endl;
    }

    cd.remove_shadowed_groups();
    if (verbose)
        std::cerr << "Removed shadowed clone groups, with the result being "
            << cd.get_number_of_clones() << " clones in "
            << cd.get_number_of_clone_groups() << " groups."
            << std::endl;

    if (json)
        cd.report_json();
    else
        cd.report_text();

    exit(EXIT_SUCCESS);
}
