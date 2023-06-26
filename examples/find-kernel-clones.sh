#!/bin/sh
#
# Find Type 2 (near) clones in a specified version of the Linux kernel
# Demonstrates the application on Git repositories
#

if [ -z "$2" ] ; then
  echo "Usage: $0 git-dir tag [directory]" 1>&2
  echo "Example: $o /usr/local/src/linux/.git v6.3 kernel"
  exit 1
fi

git --git-dir "$1" ls-tree -r --name-only "$2" -- $3 |
grep '\.[ch]$' |
while read file; do
  echo "F$file"
  git --git-dir "$1" show "$2:$file" |
  tokenizer -l C -o line -c
done |
mpcd -n 40
