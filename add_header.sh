#!/bin/bash

# Define the header
header="/*
 * This file is part of the watertight surface reconstruction code https://github.com/lcaraffa/spark-ddt
 * Copyright (c) 2024 Caraffa Laurent, Mathieu Brédif.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */"

# Find all matching files
files=$(find ./src/core -name '*.[ch]pp' -o -name '*.[ch]pp' -o -name '*.[ch]pp' -o \
    -path './services/wasure/include/*.[ch]pp' -o -path './services/wasure/src/lib/*.[ch]pp' -o -path './services/wasure/src/exe/*.[ch]pp' -o \
    -path './services/ddt/include/*.[ch]pp' -o -path './services/ddt/src/lib/*.[ch]pp' -o -path './services/ddt/src/exe/*.[ch]pp')

# Iterate over each file and prepend the header
for file in $files; do
    # Create a temporary file
    tmpfile=$(mktemp)

    # Prepend the header and then the file content to the temporary file
    echo "$header" > "$tmpfile"
    cat "$file" >> "$tmpfile"

    # Replace the original file with the temporary file
    mv "$tmpfile" "$file"
done

echo "Header added to all files."
