#!/usr/bin/bash

./build.sh || exit 1
./build/main "$@"
