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

#include <istream>
#include <vector>
#include <string>
#include <iostream>

// A constant view of a vector
template <typename T>
class ConstVectorView {
    const std::vector<T>& vec;
public:
    ConstVectorView(const std::vector<T>& vec) : vec(vec) {}

    typename std::vector<T>::const_iterator begin() const {
        return vec.begin();
    }
    typename std::vector<T>::const_iterator end() const {
        return vec.end();
    }
};

// An iterator over a vector's indices
template<typename T>
class IndexRange {
public:
    using size_type = typename T::size_type;

    class iterator {
        size_type index;
    public:
        iterator(size_type index) : index(index) {}
        size_type operator*() const { return index; }
        const iterator& operator++() { ++index; return *this; }
        bool operator!=(const iterator& other) const {
		return index != other.index;
	}
    };

    iterator begin() const { return iterator(0); }
    iterator end() const { return iterator(size); }

    IndexRange(size_type size) : size(size) {}

private:
    const size_type size;
};

// Data stored about each file
class FileData {
public:
    typedef short token_type;

private:
    // File name
    std::string name;

    // Tokens of all files
    std::vector<token_type> tokens;
    typedef decltype(tokens)::size_type line_offset_type;

    // Offsset in tokens of each line
    std::vector<line_offset_type> line_offsets;
public:
    typedef decltype(line_offsets)::size_type line_number_type;

    // Construct given a file name
    FileData(std::string name) : name(name) {}

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

    std::string get_name() const { return name; }

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
    line_offset_type remaining_tokens(line_number_type line_number) {
        return tokens.size() - line_offsets[line_number];
    }
};

class TokenContainer {
private:
    std::vector<FileData> file_data;

    // Add a new file, which becomes the top-most one
    void add_file(const std::string &name) {
        if (!file_data.empty())
            file_data.back().shrink_to_fit();
        file_data.push_back(FileData(name));
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
    // Construct from an input stream
    TokenContainer(std::istream &in);

    // Return an iterator over the container's files
    ConstVectorView<decltype(file_data)::value_type> file_view() {
        return file_data;
    }
};
