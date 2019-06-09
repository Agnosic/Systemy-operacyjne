#!/bin/bash

for size in 1000 2000 3000; do
    echo "------- size $size --------------------------------------------------------------- " 
    echo ""
    for count in 1 4 512 1024 4096 8196; do
        ./main generate file1 "$size" "$count"
        ./main copy file1 filelib "$size" "$count" lib
        ./main copy file1 filesys "$size" "$count" sys
        ./main sort filelib "$size" "$count" lib
        ./main sort filesys "$size" "$count" sys
        echo ""
    done
done