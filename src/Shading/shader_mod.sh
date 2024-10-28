#!/bin/bash

find . -type f -name "*.hpp" -print0 | while IFS= read -r -d '' file; do
    sed -i '' -E 's|(shader\.load\("shaders/)([^"]+)("\);)|#ifdef TESTING\
\1\2\3\
#endif\
#ifdef RELEASE\
\1\2\3\
#endif|g' "$file"
done