@echo off

python tools/build/code_generate.py

mkdir build\game

pushd build\game
	cmake ../.. -G "Visual Studio 17 2022"
	if %errorlevel% neq 0 exit /b %errorlevel%
popd
