#!/bin/bash

python scripts/build/code_generate.py

mkdir -p cmake-build-debug

pushd cmake-build-debug
	cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja -G Ninja
popd
