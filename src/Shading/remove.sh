#!/bin/bash

# Iterate over each file in the current directory
for file in *; do
    # Check if the file is a regular file (not a directory)
    if [[ -f "$file" ]]; then
        # Append #endif to the end of the file
        echo "#endif" >> "$file"
        
        # Optionally, you can add a message to confirm the file was processed
        echo "Appended #endif to $file"
    fi
done
