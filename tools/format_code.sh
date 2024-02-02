#!/bin/bash

[[ ! $(clang-format --version) ]] && exit 1

script_path=${0%/*}
tools_root=${script_path%/*}
project_root=$tools_root/..

if [[ "$0" != "$project_root" ]] && [[ "$project_root" != "" ]]; then
  cd "$project_root" || exit 1
  echo "-- Changed pwd to $(pwd)"
fi

files=$(find lib tools example/android/app/src/main example/desktop example/flappy -name "*.?pp")

if [[ "$files" == "" ]]; then
  echo "-- No source files found"
  exit
fi

clang-format -i $files || exit 1
echo -e "-- Formatted Files:\n$files\n"

exit
