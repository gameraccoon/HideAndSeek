cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(APP_NAME Editor)

option(MEMLEAK_DETECTION "Enable memory leaks detection code (slow)" OFF)
option(MEMLEAK_REDEFINE_NEW "Redefine default new (provide more detailed info but can conflict with some libs)" OFF)

set(GAME_BASE_DIR ${CMAKE_SOURCE_DIR}/..)
set(EXTERNALS_DIR ${GAME_BASE_DIR}/external)
set(GAME_SRC_DIR ${GAME_BASE_DIR}/src)
set(GAME_BIN_DIR ${GAME_BASE_DIR}/bin)
set(INTERMEDIATES_DIR ${GAME_BASE_DIR}/build/editor)

file(STRINGS "${GAME_BASE_DIR}/VersionInfo" EXECUTABLE_VERSION)
file(STRINGS "${GAME_BASE_DIR}/BuildNum" BUILD_NUMBER)

project(${APP_NAME} VERSION ${EXECUTABLE_VERSION})
set(EXECUTABLE_NAME ${APP_NAME})
set(REQUIRED_QT_VERSION 5.5.0)
find_package(Qt5Core ${REQUIRED_QT_VERSION} REQUIRED)
find_package(Qt5Gui ${REQUIRED_QT_VERSION} REQUIRED)
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED)
set(APP_LIBS ${APP_LIBS} ${Qt5Core_LIBRARIES})
set(APP_INCLUDE ${APP_INCLUDE} ${Qt5Core_INCLUDE_DIRS})
set(APP_COMPILE_DEFINE ${APP_COMPILE_DEFINE} ${Qt5Core_COMPILE_DEFINITIONS} )
set(APP_LIBS ${APP_LIBS} ${Qt5Gui_LIBRARIES})
set(APP_INCLUDE ${APP_INCLUDE} ${Qt5Gui_INCLUDE_DIRS})
set(APP_COMPILE_DEFINE ${APP_COMPILE_DEFINE} ${Qt5Gui_COMPILE_DEFINITIONS})
set(APP_LIBS ${APP_LIBS} ${Qt5Widgets_LIBRARIES})
set(APP_INCLUDE ${APP_INCLUDE} ${Qt5Widgets_INCLUDE_DIRS})
set(APP_COMPILE_DEFINE ${APP_COMPILE_DEFINE} ${Qt5Widgets_COMPILE_DEFINITIONS})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DDEBUG_CHECKS -DECS_DEBUG_CHECKS_ENABLED)
if(MEMLEAK_DETECTION)
	add_definitions(-DDETECT_MEMORY_LEAKS)
	if(MEMLEAK_REDEFINE_NEW)
		add_definitions(-DREDEFINE_NEW)
	endif()
endif()

# compiler parameters
if (MSVC)
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()
	set(PROJECT_CXX_FLAGS /W4 /std:c++latest)
	set(EXTERNALS_CXX_FLAGS /std:c++latest)
	set(EXTERNALS_C_FLAGS "")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
else()
	set(PROJECT_CXX_FLAGS -std=c++2a -Wall -Wextra -pedantic -Werror -pthread)
	set(EXTERNALS_CXX_FLAGS -std=c++2a)
	set(EXTERNALS_C_FLAGS "")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# code generation
execute_process(
	COMMAND ${PYTHON_EXECUTABLE} ${GAME_BASE_DIR}/scripts/build/code_generate.py ${GAME_BASE_DIR}
)

set(APP_SRCS
	src/main.cpp
	src/mainwindow.cpp

	src/mainwindow.ui

	main.qrc
)

aux_source_directory(src/editorcommands APP_SRCS)
aux_source_directory(src/editorcommands/generated APP_SRCS)
aux_source_directory(src/componenteditcontent APP_SRCS)
aux_source_directory(src/componenteditcontent/customtypeeditconstructors APP_SRCS)
aux_source_directory(src/componenteditcontent/generated APP_SRCS)
aux_source_directory(src/toolboxes APP_SRCS)
aux_source_directory(src/editorsystems APP_SRCS)
aux_source_directory(src/editorutils APP_SRCS)

add_subdirectory(Qt-Advanced-Docking-System EXCLUDE_FROM_ALL)

add_executable(${APP_NAME} WIN32 ${APP_SRCS})
set_target_properties(${APP_NAME} PROPERTIES UNITY_BUILD ON)
target_precompile_headers(${APP_NAME} PRIVATE "${GAME_SRC_DIR}/Base/precomp.h")
target_compile_options(${APP_NAME} PRIVATE ${PROJECT_CXX_FLAGS})
if(BUILD_STATIC)
	set(APP_DEFINE ${APP_DEFINE} ADS_STATIC)
endif()

function(add_folder_as_library folder_path name)
	file(GLOB_RECURSE TEMP_SRC RELATIVE "" FOLLOW_SYMLINKS "${folder_path}/*")
	add_library(${name} ${TEMP_SRC})
	target_precompile_headers(${name} PRIVATE "${GAME_SRC_DIR}/Base/precomp.h")
	set_target_properties(${name} PROPERTIES UNITY_BUILD ON)
	target_compile_options(${name} PRIVATE ${PROJECT_CXX_FLAGS})
endfunction()

add_folder_as_library(${GAME_SRC_DIR}/Base Base)
add_folder_as_library(${GAME_SRC_DIR}/GameData GameData)
add_folder_as_library(${GAME_SRC_DIR}/Utils Utils)

include_directories(
	${EXTERNALS_DIR}/include
	${EXTERNALS_DIR}/sources
	${EXTERNALS_DIR}/sources/soa-sort/include
	${GAME_SRC_DIR}
)

link_directories(
	/usr/local/lib
	${GAME_BASE_DIR}/bin/static-libs
	${EXTERNALS_DIR}/lib
	${CMAKE_SOURCE_DIR}/vs_project
)

# Polypartition
add_library(Polypartition ${EXTERNALS_DIR}/sources/polypartition/src/polypartition.cpp)
add_dependencies(${APP_NAME} Polypartition)
include_directories(${EXTERNALS_DIR}/sources/polypartition/include)
target_compile_options(Polypartition PRIVATE ${EXTERNALS_CXX_FLAGS})

set(APP_LIBS
	${APP_LIBS}
	Utils
	GameData
	Base

	Polypartition
)

if (NOT WIN32)
	set(APP_LIBS
		${APP_LIBS}
		stdc++fs
		pthread
	)
endif()

add_dependencies(${APP_NAME} qtadvanceddocking)
target_include_directories(${APP_NAME} PUBLIC
	$<BUILD_INTERFACE:${APP_INCLUDE}>
	$<INSTALL_INTERFACE:include>
)
target_include_directories(${APP_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Qt-Advanced-Docking-System/src" ${APP_INCLUDE})
target_link_libraries(${APP_NAME} PRIVATE qtadvanceddocking ${APP_LIBS})
target_compile_definitions(${APP_NAME} PRIVATE ${APP_DEFINE})
set_target_properties(${APP_NAME} PROPERTIES
	VERSION "${EXECUTABLE_VERSION}.${BUILD_NUMBER}"
	SOVERSION ${BUILD_NUMBER}
	EXPORT_NAME ${EXECUTABLE_NAME}
	ARCHIVE_OUTPUT_DIRECTORY "${INTERMEDIATES_DIR}/${ads_PlatformDir}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${INTERMEDIATES_DIR}/${ads_PlatformDir}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${GAME_BIN_DIR}"
)

# pre-compile additional code generation
add_custom_target(
	AdditionalCodeGeneration
	COMMAND ${PYTHON_EXECUTABLE} ${GAME_BASE_DIR}/scripts/build/precompile_code_generate.py ${GAME_BASE_DIR}
	COMMENT "C++ pre-compile additional code generation"
)
add_dependencies(Base AdditionalCodeGeneration)
