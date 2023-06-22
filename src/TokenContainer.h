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
 * A container for the read token stream.
 */

#pragma once

#include <algorithm>
#include <istream>
#include <vector>
#include <string>
#include <iostream>

#include "CollectionViews.h"

class FileData;

typedef std::vector<FileData> FileDataCollection;
typedef FileDataCollection::size_type file_id_type;

// Data stored about each file
class FileData {
public:
    typedef short token_type;

private:
    // File name
    std::string name;

    // Identifier
    file_id_type id;

    // Tokens of all files
    std::vector<token_type> tokens;
public:
    typedef decltype(tokens)::size_type token_offset_type;

private:
    // Offsset in tokens of each line
    std::vector<token_offset_type> line_offsets;
public:
    typedef decltype(line_offsets)::size_type line_number_type;

    file_id_type get_id() const { return id; }

    // Construct given a file name
    FileData(std::string name, file_id_type id) : name(name), id(id) {}

    // Add a token at the end
    void add_token(token_type token) {
        tokens.push_back(token);
    }

    // Add a line at the end
    void add_line() {
        line_offsets.push_back(tokens.size());
    }

    // Shrink excessively allocated capacity
    void shrink_to_fit() {
        tokens.shrink_to_fit();
        line_offsets.shrink_to_fit();
    }

    const std::string &get_name() const { return name; }

    // Return an iterator over the container's lines (line numbers)
    IndexRange<decltype(line_offsets)> line_view() {
        return IndexRange<decltype(line_offsets)>(line_offsets.size());
    }

    /*
     * Return true if the specified line is empty
     * Internally line numbers are 0-based
     */
    bool line_is_empty(line_number_type line_number) {
        if (line_number == line_offsets.size() - 1)
            return line_offsets[line_number] == tokens.size();
        else
            return line_offsets[line_number] == line_offsets[line_number + 1];
    }

    /*
     * Return the number of tokens remaining in a file after
     * a given line position.
     * Internally line numbers are 0-based
     */
    token_offset_type remaining_tokens(line_number_type line_number) {
        return tokens.size() - line_offsets[line_number];
    }

    // Return an iterator to the tokens starting in the specified line
    decltype(tokens)::iterator line_begin(line_number_type line_number) {
        return tokens.begin() + line_offsets[line_number];
    }

    // Return an iterator to the tokens starting in the specified line
    token_offset_type line_offset(line_number_type line_number) {
        return line_offsets[line_number];
    }

    // Return the (0-based) line number to which a token belongs
    line_number_type get_token_line_number(token_offset_type o) const {
        // First line with an offset greater than o
        auto upper = std::upper_bound(line_offsets.begin(), line_offsets.end(), o);
        return std::distance(line_offsets.begin(), upper) - 1;
    }
};

class TokenContainer {
private:
    FileDataCollection file_data;

    // Add a new file, which becomes the top-most one
    void add_file(const std::string &name) {
        if (!file_data.empty())
            file_data.back().shrink_to_fit();
        file_data.push_back(FileData(name, file_data.size()));
    }

    // Add a token to the top-most file
    void add_token(FileData::token_type token) {
        file_data.back().add_token(token);
    }

    // Add a line to the top-most file
    void add_line() {
        file_data.back().add_line();
    }
public:
    typedef decltype(file_data)::size_type file_id_type;

    // Construct from an input stream
    TokenContainer(std::istream &in);

    // Return an iterator over the container's files
    ConstVectorView<decltype(file_data)::value_type> file_view() const {
        return file_data;
    }

    // Return a file's name
    const std::string& get_file_name(file_id_type id) const {
        return file_data[id].get_name();
    }

    // Return a token's line
    FileData::line_number_type get_token_line_number(file_id_type id, FileData::token_offset_type o) const {
        return file_data[id].get_token_line_number(o);
    }
};
