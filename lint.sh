#!/bin/bash

orig_path="$(pwd)"
script_path="$(dirname $(readlink -f $0))"
working_dir="build/lint"

mkdir -p "$working_dir"
cd "$working_dir"

cmake "$script_path" -DMYRIAD_ENABLE_LINT=ON
make

cd "$orig_path"
