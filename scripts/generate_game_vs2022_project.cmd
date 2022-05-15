@echo off
set scriptDir=%~dp0

pushd %scriptDir:~0,-1%\..
	python scripts/build/code_generate.py

	mkdir build\game

	pushd build\game
		cmake build ../.. -G "Visual Studio 17 2022"
	popd
popd
