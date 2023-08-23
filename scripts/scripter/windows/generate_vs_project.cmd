@echo off

python scripts/build/code_generate.py

mkdir build\game

pushd build\game
	cmake ../.. -G "Visual Studio 17 2022"
popd
