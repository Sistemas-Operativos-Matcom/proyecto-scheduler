#!/usr/bin/bash

./build.sh || exit 1
for test_case in $1/*.txt; do
  ./build/main "$test_case" "${@:2}" || exit 1
done
