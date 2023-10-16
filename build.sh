#!/bin/bash

mkdir -p build
gcc -g -o build/main $(find src | grep -P "\.c") -Wall || exit 1
