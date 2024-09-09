#!/bin/bash

python3 tools/build/code_generate.py

mkdir -p cmake-build-linux-ninja

pushd cmake-build-linux-ninja
	cmake .. --preset linux-ninja
popd
