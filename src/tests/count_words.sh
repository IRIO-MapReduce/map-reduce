#!/bin/bash

fs_path="../../fs"
input="word_count_input.txt"
output="word_count_output.txt"

time ./client ../../test_mapper/build/word_count_mapper ../../test_reducer/build/word_count_reducer $fs_path/$input $fs_path/$output 1 3
echo -n "press Enter to remove split files"
read
rm $fs_path/${input%.*}-split-*
echo -n "press Enter to remove output file"
read
rm $fs_path/$output