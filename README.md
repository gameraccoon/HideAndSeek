Simple 2D game

Made with **C++**

[![MIT License](https://img.shields.io/apm/l/atomic-design-ui.svg?)](https://github.com/gameraccoon/hide-and-seek/blob/develop/License.txt)

### Intent
This is an educational project that doesn't have any applications to real-world problems, but it can still be used as a source of inspiration (or as a copypaste source).

This project was made in my free time for fun and to test some approaches. So don't expect much.

#### Features
##### Fully ECS-based code
ECS pattern has been chosen here **not** for performance reasons, but to get better maintainability and increase testability of gameplay code. ECS makes these goals easier to be achieved.

There's a very trivial implementation of ECS used, but the profiler tells that it is fine.

##### 2D dynamic lights
Nothing really interesting, just lights that are being calculated at runtime (several can be calculated in parallel).

##### Non-intrusive auto-tests and unit-tests
External applications for testing game logic that don't require changes to initial code to write new tests.

##### External editor
Editor for most of the data with Ctrl+Z/Ctrl+Y support, and with a simple 2D editor for levels. It also compiled as a separate executable and requires minimal changes in gameplay code to support new features (most likely only serialization-deserialization).

### Modules
The game divided into several modules. Modules of lower levels can't depend on modules of higher levels.

#### Base
Code that can be used in any other module:
- project-wide type aliases
- logging and debug assertions
- macros (for compiler specific features support)
- custom string types (stringIDs, localizable strings, resource paths)

#### GameData
Contains gameplay related data that will be stored in components.
- ECS Components (code-generated from jsons) and data for them
- some utilitar classes that don't have gameplay-specific code: Vector2D, FSM, ...

#### Utils
Gameplay specific code and utilities code.
- alghorithm implementations: graph search, collision detection, save-load, ...
- game initialization and debug code support

#### HAL
Hardware Abstraction Layer

High-level interfaces to input, graphic libraries, sound libraries, ...

#### GameLogic
Contains gameplay code and code that glue everything into a game
- ECS Systems code
- game initialization and the game loop

#### UnitTests
Pretty self-explanatory. Executable with unit-tests.

#### AutoTests
Game examples that can run for a limited period of frames with fixed dt and RNG seeds that be used for benchmarking some optimizations. Or autotests that checks some mechanics sanity.

### Installation (Windows/Linux)

#### Prerequisites for building the game
- git-lfs
- CMake
- python3
- a compiler that supports C++20
- for Linux you need to install SLD2, SDL2_Image and SDL2_mixer (devel and static) using your packet manager  
e.g. for apt: `sudo apt-get install libsdl2-dev libsdl2-2.0-0 libsdl2-image-dev libsdl2-image-2.0-0 libsdl2-mixer-dev libsdl2-mixer-2.0-0`

#### Prerequisites for building the editor
- git-lfs
- CMake
- python3
- a compiler that supports C++20
- Qt Creator (Qt 5.5 or higher)

#### Building the game
##### Windows
For Windows with Visual Studio you can run `scripts\generate_game_vs2019_project.cmd` it will generate the solution for Visual Studio 2019. Path to the generated solution: `build\game\GameMain.sln`

##### Linux
For all the other cases and platforms just generate the project using CMake with `CMakeLists.txt` in the root folder.

E.g. using make
```bash
mkdir -p build/game
cd build/game
cmake ../..
make
```

After being built, the resulting executables can be found in `bin` folder

#### Building the editor
Generate the project using CMake with `editor/CMakeLists.txt`

E.g. using make
```bash
mkdir -p build/editor
cd build/editor
cmake ../../editor
make
```

Resulting executable: `bin/Editor`
