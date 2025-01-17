cmake_minimum_required(VERSION 3.16)
include(CMakeDependentOption)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# parameters
option(DEBUG_CHECKS "Enable debug checks and assertions" ON)
option(ENABLE_SCOPED_PROFILER "Enable profiler that records time of execution of scopes across threads" OFF)
option(MEMLEAK_DETECTION "Enable memory leaks detection code (slow)" OFF)
option(MEMLEAK_REDEFINE_NEW "Redefine default new (provide more detailed info but can conflict with some libs)" OFF)
option(CONCURRENT_ACCESS_DETECTION "Enable concurrent access detection" ON)
option(IMGUI_ENABLED "Enable imgui in the build" ON)
option(ENABLE_UNITY_BUILD "Should the sources be built with unity build" ON)
option(BUILD_AUTO_TESTS "Build automatic tests" ON)

set(APP_NAME GameMain)
set(EXECUTABLE_NAME Game)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# constants
set(GAME_BASE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GAME_BASE_DIR}/cmake")
set(GAME_BIN_DIR ${GAME_BASE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GAME_BIN_DIR})
set(GAME_SRC_DIR ${GAME_BASE_DIR}/src)
set(ENGINE_SRC_DIR ${GAME_SRC_DIR}/Engine)
set(EXTERNALS_DIR ${GAME_BASE_DIR}/external)
set(BIN_PLATFORM "x64")

set(BUILD_SHARED_LIBS FALSE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)

file(STRINGS "${GAME_BASE_DIR}/VersionInfo" EXECUTABLE_VERSION)
file(STRINGS "${GAME_BASE_DIR}/BuildNum" BUILD_NUMBER)
project(${APP_NAME} VERSION ${EXECUTABLE_VERSION})

file(MAKE_DIRECTORY ${GAME_BIN_DIR})

# definitions
if(DEBUG_CHECKS)
	add_definitions(-DDEBUG_CHECKS -DRACCOON_ECS_DEBUG_CHECKS_ENABLED)
endif(DEBUG_CHECKS)

if(ENABLE_SCOPED_PROFILER)
	add_definitions(-DENABLE_SCOPED_PROFILER)
endif(ENABLE_SCOPED_PROFILER)

if(MEMLEAK_DETECTION)
	add_definitions(-DDETECT_MEMORY_LEAKS)
	if(MEMLEAK_REDEFINE_NEW)
		add_definitions(-DREDEFINE_NEW)
	endif()
endif()

if(IMGUI_ENABLED)
	add_definitions(-DIMGUI_ENABLED)
endif()

if(CONCURRENT_ACCESS_DETECTION)
	add_definitions(-DCONCURRENT_ACCESS_DETECTION)
endif(CONCURRENT_ACCESS_DETECTION)

if(BUILD_AUTO_TESTS)
	add_definitions(-DBUILD_AUTO_TESTS)
endif()

add_definitions(-DGLEW_NO_GLU)
add_definitions(-DGLM_FORCE_RADIANS)
# Real network requires GameNetworkingSockets library, so we disable it until we need it
add_definitions(-DFAKE_NETWORK)

# compiler parameters
if(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()
	set(PROJECT_CXX_FLAGS /W4 /std:c++latest /wd4996)
	set(EXTERNALS_CXX_FLAGS /W0 /std:c++latest /wd4996 /wd4458)
	set(EXTERNALS_C_FLAGS /W0)

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
else()
	set(PROJECT_CXX_FLAGS -std=c++2a -Wall -Wextra -pedantic -Werror)
	set(EXTERNALS_CXX_FLAGS -std=c++2a)
	set(EXTERNALS_C_FLAGS "")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# requirements
find_package(Python3 COMPONENTS Interpreter)

# code generation
execute_process(
	COMMAND ${Python3_EXECUTABLE} "${GAME_BASE_DIR}/tools/build/run_code_generation.py" --config ${GAME_BASE_DIR}/tools/build/generators.json --group pre-project-generation
	WORKING_DIRECTORY ${GAME_BASE_DIR}
	COMMAND_ERROR_IS_FATAL ANY
)

include_directories(
	${EXTERNALS_DIR}/include
	${EXTERNALS_DIR}/sources
	${EXTERNALS_DIR}/sources/soa-sort/include
	${GAME_SRC_DIR}
	${ENGINE_SRC_DIR}
)

link_directories(
	${EXTERNALS_DIR}/lib/${BIN_PLATFORM}
	${GAME_BASE_DIR}/vs_project
)

function(add_folder_as_library folder_path name)
	file(GLOB_RECURSE TEMP_SRC RELATIVE "" FOLLOW_SYMLINKS "${folder_path}/*")
	add_library(${name} ${TEMP_SRC})
	target_precompile_headers(${name} PRIVATE "${ENGINE_SRC_DIR}/EngineCommon/precomp.h")
	set_target_properties(${name} PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILD})
	target_compile_options(${name} PRIVATE ${PROJECT_CXX_FLAGS})
endfunction()

# All the things that are going to be included anywhere and not related
# to any specific game
add_folder_as_library(${ENGINE_SRC_DIR}/EngineCommon EngineCommon)
# Data types that can be stored in engine components
add_folder_as_library(${ENGINE_SRC_DIR}/EngineData EngineData)
# Data types that can be stored in game-specific ECS components
add_folder_as_library(${GAME_SRC_DIR}/GameData GameData)
# Algorithms and types that don't have to be stored in ECS components (shared between games)
add_folder_as_library(${ENGINE_SRC_DIR}/EngineUtils EngineUtils)
# Algorithms and types that don't have to be stored in ECS components (specific to one game)
add_folder_as_library(${GAME_SRC_DIR}/GameUtils GameUtils)
# Hardware Access Layer (shared between games, can depend only on Engine common/data/utils)
add_folder_as_library(${ENGINE_SRC_DIR}/HAL Hal)
# ECS systems and glue code (engine specific)
add_folder_as_library(${ENGINE_SRC_DIR}/EngineLogic EngineLogic)
# ECS systems and glue code (specific to one game)
add_folder_as_library(${GAME_SRC_DIR}/GameLogic GameLogic)
if(BUILD_AUTO_TESTS)
	# Automatic tests that run predefined scenarios and validate against checklists
	add_folder_as_library(${GAME_SRC_DIR}/AutoTests AutoTests)
endif()

file(GLOB_RECURSE APP_SRC RELATIVE "" FOLLOW_SYMLINKS "${GAME_SRC_DIR}/GameMain/*")
add_executable(${APP_NAME} "${APP_SRC}")
target_compile_options(${APP_NAME} PRIVATE ${PROJECT_CXX_FLAGS})

# configure VS specific project parameters
if(MSVC)
	set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${GAME_BIN_DIR}")
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
endif()

# SDL2
add_subdirectory(${EXTERNALS_DIR}/sources/SDL EXCLUDE_FROM_ALL)
include_directories(${EXTERNALS_DIR}/sources/SDL/include)

#SDL_image
add_subdirectory(${EXTERNALS_DIR}/sources/SDL_image EXCLUDE_FROM_ALL)
include_directories(${EXTERNALS_DIR}/sources/SDL_image/include)

#SDL_mixer
add_subdirectory(${EXTERNALS_DIR}/sources/SDL_mixer EXCLUDE_FROM_ALL)
include_directories(${EXTERNALS_DIR}/sources/SDL_mixer/include)

# Polypartition
add_library(Polypartition EXCLUDE_FROM_ALL ${EXTERNALS_DIR}/sources/polypartition/src/polypartition.cpp)
add_dependencies(${APP_NAME} Polypartition)
include_directories(${EXTERNALS_DIR}/sources/polypartition/include)
target_compile_options(Polypartition PRIVATE ${EXTERNALS_CXX_FLAGS})

# ImGui
if(IMGUI_ENABLED)
	add_library(Imgui EXCLUDE_FROM_ALL
		${EXTERNALS_DIR}/sources/imgui/imgui.cpp
		${EXTERNALS_DIR}/sources/imgui/imgui_demo.cpp
		${EXTERNALS_DIR}/sources/imgui/imgui_draw.cpp
		${EXTERNALS_DIR}/sources/imgui/imgui_tables.cpp
		${EXTERNALS_DIR}/sources/imgui/imgui_widgets.cpp
		${EXTERNALS_DIR}/sources/imgui/backends/imgui_impl_opengl2.cpp
		${EXTERNALS_DIR}/sources/imgui/backends/imgui_impl_sdl2.cpp
	)
	add_dependencies(${APP_NAME} Imgui)
	include_directories(${EXTERNALS_DIR}/sources/imgui)
	target_compile_options(Imgui PRIVATE ${EXTERNALS_CXX_FLAGS})
endif()

set(GENERAL_PROJECT_LIBS
	GameLogic
	EngineLogic
	Hal
	GameUtils
	EngineUtils
	GameData
	EngineData
	EngineCommon

	Polypartition

	SDL2::SDL2-static
	SDL2_image
	SDL2_mixer
	SDL2main
)

if (BUILD_AUTO_TESTS)
	set(GENERAL_PROJECT_LIBS
			${GENERAL_PROJECT_LIBS}
			AutoTests
	)
endif()

if(IMGUI_ENABLED)
	set(GENERAL_PROJECT_LIBS
		${GENERAL_PROJECT_LIBS}
		Imgui
	)
endif()

if(WIN32)
	set(GENERAL_PROJECT_LIBS
		${GENERAL_PROJECT_LIBS}
		opengl32
	)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(GENERAL_PROJECT_LIBS
		${GENERAL_PROJECT_LIBS}
		GL
		stdc++
	)
else()
	set(GENERAL_PROJECT_LIBS
		${GENERAL_PROJECT_LIBS}
		${SDL2_LIBRARY}
		${SDL2_IMAGE_LIBRARY}
		GL
		stdc++fs
	)
endif()

target_link_libraries(${APP_NAME} ${GENERAL_PROJECT_LIBS})

set_target_properties(${APP_NAME} PROPERTIES
	VERSION "${EXECUTABLE_VERSION}.${BUILD_NUMBER}"
	SOVERSION ${BUILD_NUMBER}
	EXPORT_NAME ${EXECUTABLE_NAME}
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${GAME_BIN_DIR}"
)

# Unit tests
set(UNITTESTS_NAME UnitTests)
file(GLOB_RECURSE UNITTESTS_SRC RELATIVE "" FOLLOW_SYMLINKS "${GAME_SRC_DIR}/UnitTests/*")
set(UNITTESTS_SRC
	${UNITTESTS_SRC}
	${EXTERNALS_DIR}/sources/googletest/src/gtest-all.cc
)
include_directories(${EXTERNALS_DIR}/sources/googletest)
add_executable(${UNITTESTS_NAME} ${UNITTESTS_SRC})
target_compile_definitions(${UNITTESTS_NAME} PRIVATE -DRACCOON_ECS_TOOLMODE)
target_compile_options(${UNITTESTS_NAME} PRIVATE ${PROJECT_CXX_FLAGS})
target_link_libraries(${UNITTESTS_NAME} ${GENERAL_PROJECT_LIBS})

# symlink resources dir
if(NOT EXISTS "${GAME_BIN_DIR}/resources")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/resources" "${GAME_BIN_DIR}/resources"
		COMMAND_ERROR_IS_FATAL ANY
	)
endif()

# pre-compile additional code generation
add_custom_target(
	AdditionalCodeGeneration
	COMMAND ${Python3_EXECUTABLE} ${GAME_BASE_DIR}/tools/build/run_code_generation.py --config ${GAME_BASE_DIR}/tools/build/generators.json --working-dir ${GAME_BASE_DIR} --group pre-project-generation --quiet
	COMMENT "C++ pre-compile additional code generation"
)
add_dependencies(EngineCommon AdditionalCodeGeneration)
