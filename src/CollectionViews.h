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
 * Read-only views of collections
 */

#pragma once

#include <vector>

// A constant view of a collection
template <typename C>
class ConstCollectionView {
    const C& col;
public:
    ConstCollectionView(const C& col) : col(col) {}

    typename C::const_iterator begin() const {
        return col.begin();
    }
    typename C::const_iterator end() const {
        return col.end();
    }
};

// An iterator over a vector's indices
template<typename T>
class IndexRange {
public:
    using size_type = typename T::size_type;

    class ConstIterator {
        size_type index;
    public:
        ConstIterator(size_type index) : index(index) {}
        size_type operator*() const { return index; }
        const ConstIterator& operator++() { ++index; return *this; }
        bool operator!=(const ConstIterator& other) const {
            return index != other.index;
        }
    };

    ConstIterator begin() const { return ConstIterator(0); }
    ConstIterator end() const { return ConstIterator(size); }

    IndexRange(size_type size) : size(size) {}

private:
    const size_type size;
};
