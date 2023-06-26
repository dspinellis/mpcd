#!/bin/sh
#
# Find Type 1 (exact) clones among C++ files in the current directory
#

tokenizer -f -l C++ -o line -g *.cpp *.h | mpcd -n 25
