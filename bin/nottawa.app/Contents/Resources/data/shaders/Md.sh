#!/bin/bash

for file in *
do
    if [ -f "$file" ]; then
        touch "${file%.*}.md"
    fi
done