#!/bin/bash

for file in *
do
    if [ -f "$file" ]; then
        # Get the first letter of the filename and capitalize it
        first_letter=$(echo "${file:0:1}" | tr '[:lower:]' '[:upper:]')
        # Get the rest of the filename after the first letter
        rest_of_name=$(echo "${file:1}")
        # Concatenate the capitalized first letter and the rest of the filename
        new_name="${first_letter}${rest_of_name}"
        # Rename the file
        mv "$file" "$new_name"
    fi
done