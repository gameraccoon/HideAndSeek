@echo off
set scriptDir=%~dp0

pushd %scriptDir:~0,-1%\..
	pushd build\game
		start GameMain.sln
	popd
popd
