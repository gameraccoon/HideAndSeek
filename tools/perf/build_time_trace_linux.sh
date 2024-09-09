#!/bin/bash

mkdir build_profile
pushd build_profile
	cmake -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -ftime-trace=../build_time_trace.json" .. --preset linux-ninja-clang
popd

cmake --build ./build_profile --target all

rm -r build_profile

