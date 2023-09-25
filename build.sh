#!/bin/bash

mkdir -p build
gcc -o build/main $(find src | grep -P "\.c") -Wall || exit 1
