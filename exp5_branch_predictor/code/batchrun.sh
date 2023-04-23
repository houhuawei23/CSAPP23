#!/bin/bash

make $1
trace_folder="./traces"
res_folder="./results"

executable="./$1"
outfile="$res_folder/$executable.out"

if [ -e "$outfile" ]; then
    echo "Removing old output file"
    rm "$outfile"
else
    echo "Creating output file"
    touch "$outfile"
fi

for file in "${trace_folder}"/*.trace.gz
do
    if [[ -f $file ]]; then
        echo "Processing $file"
        "$executable" "$file" >> "$outfile"
    fi
done
cd "$res_folder"
python outprocess.py "$executable.out"