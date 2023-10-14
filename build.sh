#!/bin/bash

mkdir -p build
gcc -o build/main $(find src | grep -p "\.c") -Wall || exit 1
