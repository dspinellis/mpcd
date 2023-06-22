#!/bin/sh
#
# Find c++ clones in the current directory
#

find . -type f -name '*.cpp' -o -name '*.h' -print0 |
while IFS= read -r -d '' file; do
  echo "F$file"
  tokenizer -l C++ -o line -t N "$file"
done |
mpcd -n 25
