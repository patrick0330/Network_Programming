#!/bin/bash
file_path=./input
echo $(ls $file_path)
for i in {0..999}
do
    num=$(printf "%06d" $i)
    $(base64 /dev/urandom | head -c 5000 > $file_path/$num)
done