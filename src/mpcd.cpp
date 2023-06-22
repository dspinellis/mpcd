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

// Identify clones among the tokenized input stream
int
main(int argc, char * const argv[])
{
    int opt;
    int clone_tokens = 15; // Minimum number of same tokens to identify a clone

    while ((opt = getopt(argc, argv, "n:")) != -1)
        switch (opt) {
        case 'n':
            clone_tokens = std::atoi(optarg);
            if (clone_tokens == 0) {
                std::cerr << "Invalid token number specified" << std::endl;
                exit(EXIT_FAILURE);
            }
            break;
        default: /* ? */
            std::cerr << "Usage: " << argv[0] <<
                " [-n tokens]" << std::endl;
            exit(EXIT_FAILURE);
        }

    TokenContainer tc(std::cin);
    CloneDetector cd(tc, clone_tokens);
    cd.prune_non_clones();
    cd.report();

    exit(EXIT_SUCCESS);
}
