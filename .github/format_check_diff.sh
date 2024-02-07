#!/bin/bash

[[ ! $(git --version) ]] && exit 1

output=$(git diff)

if [[ "$output" != "" ]]; then
  echo "One or more source files are not formatted!"
  exit 1
fi

echo "All source files are formatted"
exit
