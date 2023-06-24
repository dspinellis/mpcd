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
    typedef std::vector<token_type> tokens_type;

private:
    // File name
    std::string name;

    // Identifier
    file_id_type id;

    // Tokens of all files
    tokens_type tokens;

public:
    typedef tokens_type::size_type token_offset_type;

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

    // Add a line's offset at the end of the existing vector's elements
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
    bool line_is_empty(line_number_type line_number) const {
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
    token_offset_type remaining_tokens(line_number_type line_number) const {
        return tokens.size() - line_offsets[line_number];
    }

    // Return an iterator to the tokens starting in the specified line
    tokens_type::const_iterator line_begin(line_number_type line_number) const {
        return tokens.begin() + line_offsets[line_number];
    }

    // Return an iterator on the file's end
    tokens_type::const_iterator file_end() const {
        return tokens.end();
    }

    // Return an iterator to the tokens starting at the specified offset
    tokens_type::const_iterator offset_begin(token_offset_type o) const {
        return tokens.begin() + o;
    }

    // Return the offset of the tokens starting in the specified line
    token_offset_type line_offset(line_number_type line_number) const {
        return line_offsets[line_number];
    }

    // Return the (0-based) line number to which a token belongs
    line_number_type get_token_line_number(token_offset_type offset) const {
        // First line with an offset greater than offset
        auto upper = std::upper_bound(line_offsets.begin(), line_offsets.end(), offset);
        return std::distance(line_offsets.begin(), upper) - 1;
    }

    // Return an iterator to the end of the line to which a token belongs
    tokens_type::const_iterator line_from_offset_end(token_offset_type o) const {
        auto next_line_number = get_token_line_number(o) + 1;
        if (next_line_number == line_offsets.size())
            return file_end();
        else
            return tokens.begin() + line_offsets[next_line_number];
    }

    // Return the token at the specified location; 0 if at EOF
    FileData::token_type get_token(FileData::token_offset_type offset) const {
        if (tokens.begin() + offset == tokens.end())
            return 0;
        return tokens[offset];
    }

    // Return the end-offset of the line lying immediately before the offset
    FileData::token_offset_type get_preceding_eol_offset(
        FileData::token_offset_type offset) const {
        if (offset == tokens.size())
            return tokens.size();
        auto line_number = get_token_line_number(offset);
        return line_offsets[line_number];
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
    typedef FileDataCollection::size_type file_id_type;

    // Construct from an input stream
    TokenContainer(std::istream &in);

    // Return an iterator over the container's files
    ConstCollectionView<FileDataCollection> file_view() const {
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

    // Return an iterator to the tokens starting in the specified offset
    FileData::tokens_type::const_iterator offset_begin(file_id_type file_id,
             FileData::token_offset_type offset) const {
        return file_data[file_id].offset_begin(offset);
    }

    // Return an iterator to line end of tokens starting in the specified offset
    FileData::tokens_type::const_iterator line_from_offset_end(file_id_type file_id,
            FileData::token_offset_type offset) const {
        return file_data[file_id].line_from_offset_end(offset);
    }

    // Return the token at the specified location; 0 if at EOF
    FileData::token_type get_token(file_id_type file_id,
            FileData::token_offset_type offset) const {
        return file_data[file_id].get_token(offset);
    }

    // Return the end-offset of the line lying immediately before the offset
    FileData::token_offset_type get_preceding_eol_offset(file_id_type file_id,
            FileData::token_offset_type offset) const {
        return file_data[file_id].get_preceding_eol_offset(offset);
    }
};
