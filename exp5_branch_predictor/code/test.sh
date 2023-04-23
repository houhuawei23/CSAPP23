#!/bin/bash

trace_folder="./traces"
res_folder="./results"

executable="./$1"
outfile="$res_folder/$executable.out"
cd "$res_folder"
python outprocess.py "$executable.out"