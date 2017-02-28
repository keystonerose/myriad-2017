#!/bin/bash

orig_path="$(pwd)"
script_path="$(dirname $(readlink -f $0))"
working_dir="build/lint"

mkdir -p "$working_dir"
cd "$working_dir"


cmake "$script_path" \
-DCMAKE_C_COMPILER=/usr/bin/clang \
-DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
-DMYRIAD_ENABLE_LINT=ON

make

cd "$orig_path"
