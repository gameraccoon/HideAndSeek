Simple 2D game

Made with **C++**

[![MIT License](https://img.shields.io/github/license/gameraccoon/hide-and-seek)](https://github.com/gameraccoon/hide-and-seek/blob/develop/License.txt)  
[![Windows - build](https://github.com/gameraccoon/hide-and-seek/actions/workflows/build-game-windows.yml/badge.svg)](https://github.com/gameraccoon/hide-and-seek/actions/workflows/build-game-windows.yml) [![Ubuntu - build and run unit-tests](https://github.com/gameraccoon/hide-and-seek/actions/workflows/build-game-ubuntu.yml/badge.svg)](https://github.com/gameraccoon/hide-and-seek/actions/workflows/build-game-ubuntu.yml) [![Ubuntu - build with clang](https://github.com/gameraccoon/hide-and-seek/actions/workflows/build-game-ubuntu-clang.yml/badge.svg)](https://github.com/gameraccoon/hide-and-seek/actions/workflows/build-game-ubuntu-clang.yml)

## Intent
This is an educational project that doesn't have any applications to real-world problems, but it can still be used as a source of inspiration (or as a copypaste source).

This project was made in my free time for fun and to test some approaches. So don't expect much.

## Features
### Fully ECS-based code
ECS pattern has been chosen here **not** for performance reasons, but to get better maintainability and increase testability of gameplay code. ECS makes these goals easier to be achieved.

There's a very [trivial implementation of ECS](https://github.com/gameraccoon/raccoon-ecs) used, but the profiler tells that it is fine.

### 2D dynamic lights
Lights with shadows that are being calculated at runtime (several can be calculated in parallel).

### Non-intrusive auto-tests and unit-tests
External code for testing game logic that doesn't require changes to the tested code in order to write new tests.

### External editor
Editor for most of the data with Undo/Redo support, and with a simple 2D editor for levels. It is compiled as a separate executable and requires minimal changes to the gameplay code to support new features (most likely only serialization-deserialization).

## Modules
The game is divided into several modules. Modules of lower levels can't depend on modules of higher levels.

There's no split between game and engine in the folder structure, rather the game is made with minimum dependencies between core features, so you can remove part of the code of the features (systems => utility code => components) that you don't need in the new project while keeping the game compilable and working. A good example is [tank-game](https://github.com/gameraccoon/tank-game), a multiplayer game based on this project.

### Base
Project-agnostic code that can be used in any other module:
- project-wide type aliases
- logging and debug assertions
- macros (for compiler specific features support)
- custom string types (hashed strings, localizable strings, resource paths)

### GameData
Contains gameplay related data that will be stored in components
- ECS Components (code-generated from jsons) and data for them
- some utilty classes that don't have gameplay-specific code: Vector2D, FSM, ...

### Utils
Gameplay specific code and utilities code
- alghorithm implementations: graph search, collision detection, save-load, ...
- game initialization and debug code support

### HAL
Hardware Abstraction Layer

Project-agnostic high-level interfaces to input, graphic libraries, sound libraries, ...

### GameLogic
Contains gameplay code and code that glue everything into a game
- the code of ECS Systems
- game initialization and the game loop

### UnitTests
Executable with unit-tests

### AutoTests
Game examples that can run for a limited period of frames with fixed dt and RNG seeds that be used for performance measurement. Or autotests that checks some mechanics sanity

### External code
External libraries can be found in `external` folder.

* SDL2 with glew and glm
* [gameraccoon/raccoon-ecs](https://github.com/gameraccoon/raccoon-ecs)
* [nlohmann/json](https://github.com/nlohmann/json)
* [Neargye/magic-enum](https://github.com/Neargye/magic_enum)
* [cameron314/concurrentqueue](https://github.com/cameron314/concurrentqueue)
* [google/googletest](https://github.com/google/googletest)
* [ocornut/imgui](https://github.com/ocornut/imgui)
* [ivanfratric/polypartition](https://github.com/ivanfratric/polypartition)
* [gameraccoon/soa-sort](https://github.com/gameraccoon/soa-sort)
* [Stephan Brumme FNV1a hash](https://create.stephan-brumme.com/fnv-hash/) + [ruby0x1/hash_fnv1a.h](https://gist.github.com/ruby0x1/81308642d0325fd386237cfa3b44785c)


## Getting and building (Windows/Linux)

### Prerequisites for building the game
- git with git-lfs
- CMake (see minimal supported version in [CMakeLists.txt](https://github.com/gameraccoon/hide-and-seek/blob/develop/CMakeLists.txt#L1=))
- python3
- gcc 13 (or higher), or clang-16 (or higher), or latest Visual Studio 2022 (or newer)
- for Linux you need to install sound libraries using your packet manager  
e.g. for apt: `sudo apt-get install libopusfile-dev libflac-dev libxmp-dev libfluidsynth-dev libwavpack-dev libmodplug-dev`

### Prerequisites for building the editor
- git with git-lfs
- CMake (see minimal supported version in [editor/CMakeLists.txt](https://github.com/gameraccoon/hide-and-seek/blob/develop/editor/CMakeLists.txt#L1=))
- python3
- gcc 13 (or higher)
- qt5 core libraries installed, e.g. for apt: `sudo apt install qtbase5-dev`

### Getting the code
With SSH  
`git clone --recursive git@github.com:gameraccoon/hide-and-seek.git`

With HTTPS  
`git clone --recursive https://github.com/gameraccoon/hide-and-seek.git`

### Building the game
#### Windows
For Windows with Visual Studio you can run `scripts\generate_game_vs2020_project.cmd` it will generate the solution for Visual Studio 2022. Path to the generated solution: `build\game\GameMain.sln`

#### Linux
For all the other cases and platforms just generate the project using CMake with `CMakeLists.txt` in the root folder.

E.g. using make
```bash
mkdir -p build/game
cd build/game
cmake ../..
make
```

After being built, the resulting executables can be found in `bin` folder

### Building the editor
Generate the project using CMake with `editor/CMakeLists.txt`. Windows build of the editor is not tested, only Linux build is confirmed to be working.

E.g. using make
```bash
mkdir -p build/editor
cd build/editor
cmake ../../editor
make
```

After being built, the resulting executables can be found in `bin/Editor` folder
