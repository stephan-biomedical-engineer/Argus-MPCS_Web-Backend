#!/bin/bash

dirs=(
    ../api
    ../api/routes  
    ../drivers
    ../models
    ../services
    ../system
    ../utl
)

for dir in "${dirs[@]}"; do
    echo "Formatting files in $dir"
    find "$dir" -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -style=file -i {} +
done
