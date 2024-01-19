#!/bin/bash

fs_path="../../fs"
input="input.txt"
output="output.txt"

time ./client ../../test_mapper/build/simple_mapper ../../test_reducer/build/simple_reducer $fs_path/$input $fs_path/$output 1 3
# compare output and input files ignoring line order
if cmp -s <(sort $fs_path/$output) <(sort $fs_path/$input); then
    echo -e "Mapreduce finished"
else
    echo -e "Mapreduce finished, \e[1;31minvalid output\e[0m"
fi
echo -n "press Enter to remove split files"
read
rm $fs_path/${input%.*}-split-*
echo -n "press Enter to remove output file"
read
rm $fs_path/$output