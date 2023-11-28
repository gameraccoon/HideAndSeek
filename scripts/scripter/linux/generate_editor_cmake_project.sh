#!/bin/bash

python3 scripts/build/code_generate.py

mkdir -p cmake-build-editor-linux-ninja

pushd cmake-build-editor-linux-ninja
	cmake ../editor --preset linux-ninja
popd
