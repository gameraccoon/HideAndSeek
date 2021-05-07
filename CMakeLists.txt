cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_NAME GameMain)
project(${APP_NAME})

set(EXECUTABLE_NAME Game)

# parameters
option(DEBUG_CHECKS "Enable debug checks and assertions" ON)
option(PROFILE_SYSTEMS "Enable systems time gathering for systems time reports" ON)
option(MEMLEAK_DETECTION "Enable memory leaks detection code (slow)" OFF)
option(MEMLEAK_REDEFINE_NEW "Redefine default new (provide more detailed info but can conflict with some libs)" OFF)
option(CONCURRENT_ACCESS_DETECTION "Enable concurrent access detection" ON)
option(IMGUI_ENABLED "Enable imgui in the build" ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# constants
set(GAME_BASE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GAME_BASE_DIR}/cmake")
set(GAME_BIN_DIR ${GAME_BASE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GAME_BIN_DIR})
set(GAME_SRC_DIR ${GAME_BASE_DIR}/src)
set(EXTERNALS_DIR ${GAME_BASE_DIR}/external)
set(BIN_PLATFORM "x64")

file (STRINGS "${GAME_BASE_DIR}/VersionInfo" EXECUTABLE_VERSION)
file(STRINGS "${GAME_BASE_DIR}/BuildNum" BUILD_NUMBER)
project(${APP_NAME} VERSION ${EXECUTABLE_VERSION})

file(MAKE_DIRECTORY ${GAME_BIN_DIR})

# definitions
if(DEBUG_CHECKS)
	add_definitions(-DDEBUG_CHECKS -DECS_DEBUG_CHECKS_ENABLED)
endif(DEBUG_CHECKS)

if(PROFILE_SYSTEMS)
	add_definitions(-DPROFILE_SYSTEMS)
endif(PROFILE_SYSTEMS)

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

add_definitions(-DGLEW_NO_GLU)
add_definitions(-DGLM_FORCE_RADIANS)

# compiler parameters
if (MSVC)
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
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

# code generation
execute_process(
	COMMAND ${PYTHON_EXECUTABLE} "${GAME_BASE_DIR}/scripts/build/code_generate.py" WORKING_DIRECTORY "${GAME_BASE_DIR}"
)

# requirements
find_package(PythonInterp 3.5 REQUIRED)
if (NOT WIN32)
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	include_directories(${SDL2_INCLUDE_DIR})
endif()

include_directories(include)

include_directories(
	${EXTERNALS_DIR}/include
	${EXTERNALS_DIR}/include/sdl
	${EXTERNALS_DIR}/sources
	${EXTERNALS_DIR}/sources/soa-sort/include
	${GAME_SRC_DIR}
)

link_directories(
	${EXTERNALS_DIR}/lib/${BIN_PLATFORM}
	${GAME_BASE_DIR}/vs_project
)

function(add_folder_as_library folder_path name)
	file(GLOB_RECURSE TEMP_SRC RELATIVE "" FOLLOW_SYMLINKS "${folder_path}/*")
	add_library(${name} ${TEMP_SRC})
	target_precompile_headers(${name} PRIVATE "${GAME_SRC_DIR}/Base/precomp.h")
	set_target_properties(${name} PROPERTIES UNITY_BUILD ON)
	target_compile_options(${name} PRIVATE ${PROJECT_CXX_FLAGS})
endfunction()

add_folder_as_library(${GAME_SRC_DIR}/Base Base)
add_folder_as_library(${GAME_SRC_DIR}/ECS Ecs)
add_folder_as_library(${GAME_SRC_DIR}/GameData GameData)
add_folder_as_library(${GAME_SRC_DIR}/GameLogic GameLogic)
add_folder_as_library(${GAME_SRC_DIR}/Utils Utils)
add_folder_as_library(${GAME_SRC_DIR}/HAL Hal)

file(GLOB_RECURSE APP_SRC RELATIVE "" FOLLOW_SYMLINKS "${GAME_SRC_DIR}/GameMain/*")
add_executable(${APP_NAME} "${APP_SRC}")
target_compile_options(${APP_NAME} PRIVATE ${PROJECT_CXX_FLAGS})

# configure VS specific project parameters
if (MSVC)
	set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${GAME_BIN_DIR}")
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
endif()

# Polypartition
add_library(Polypartition EXCLUDE_FROM_ALL ${EXTERNALS_DIR}/sources/polypartition/src/polypartition.cpp)
add_dependencies(${APP_NAME} Polypartition)
include_directories(${EXTERNALS_DIR}/sources/polypartition/include)
target_compile_options(Polypartition PRIVATE ${EXTERNALS_CXX_FLAGS})

# ImGui
add_library(Imgui EXCLUDE_FROM_ALL
	${EXTERNALS_DIR}/sources/imgui/imgui.cpp
	${EXTERNALS_DIR}/sources/imgui/imgui_demo.cpp
	${EXTERNALS_DIR}/sources/imgui/imgui_draw.cpp
	${EXTERNALS_DIR}/sources/imgui/imgui_widgets.cpp
	${EXTERNALS_DIR}/sources/imgui/examples/imgui_impl_opengl2.cpp
	${EXTERNALS_DIR}/sources/imgui/examples/imgui_impl_sdl.cpp
)
add_dependencies(${APP_NAME} Imgui)
include_directories(${EXTERNALS_DIR}/sources/imgui)
target_compile_options(Imgui PRIVATE ${EXTERNALS_CXX_FLAGS})

set(GENERAL_PROJECT_LIBS
	GameLogic
	Hal
	Utils
	GameData
	Ecs
	Base

	Polypartition
	Imgui

	SDL2
	SDL2_image
	SDL2_mixer
	SDL2_ttf
	SDL2main
)

if (WIN32)
	set(GENERAL_PROJECT_LIBS
		${GENERAL_PROJECT_LIBS}
		glew32
		glew32s
		SDL2test
		opengl32
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

target_link_libraries(${APP_NAME}
	${GENERAL_PROJECT_LIBS}
)

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
target_compile_options(${UNITTESTS_NAME} PRIVATE ${PROJECT_CXX_FLAGS})
#set_target_properties(${UNITTESTS_NAME} PROPERTIES UNITY_BUILD ON)
target_link_libraries(${UNITTESTS_NAME}
	${GENERAL_PROJECT_LIBS}
)

# Autotests
set(AUTOTESTS_NAME AutoTests)
file(GLOB_RECURSE AUTOTESTS_SRC RELATIVE "" FOLLOW_SYMLINKS "${GAME_SRC_DIR}/AutoTests/*")
add_executable(${AUTOTESTS_NAME} ${AUTOTESTS_SRC})
target_compile_options(${AUTOTESTS_NAME} PRIVATE ${PROJECT_CXX_FLAGS})
set_target_properties(${AUTOTESTS_NAME} PROPERTIES UNITY_BUILD ON)
target_link_libraries(${AUTOTESTS_NAME}
	${GENERAL_PROJECT_LIBS}
)

# symlink resources dir
if(NOT EXISTS "${GAME_BIN_DIR}/resources")
	execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/resources" "${GAME_BIN_DIR}/resources")
endif()

if (WIN32)
	# copy dlls
	file(GLOB WIN32_DLLS
		"${EXTERNALS_DIR}/dll/${BIN_PLATFORM}/*"
	)
	file(COPY ${WIN32_DLLS} DESTINATION ${GAME_BIN_DIR})
endif()

# pre-compile additional code generation
add_custom_target(
	AdditionalCodeGeneration
	COMMAND ${PYTHON_EXECUTABLE} ${GAME_BASE_DIR}/scripts/build/precompile_code_generate.py ${GAME_BASE_DIR}
	COMMENT "C++ pre-compile additional code generation"
)
add_dependencies(Base AdditionalCodeGeneration)
