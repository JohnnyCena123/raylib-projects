cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)

project(TicTacToe)

message(STATUS "Looking for ccache/sccache")
find_program(CCACHE_PROGRAM ccache)
find_program(SCCACHE_PROGRAM sccache)
if(CCACHE_PROGRAM)
	message(NOTICE "Using ccache: ${CCACHE_PROGRAM}")
	set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

find_program(SCCACHE_PROGRAM sccache)
if (SCCACHE_PROGRAM AND NOT CCACHE_PROGRAM)
	message(STATUS "Found sccache: ${SCCACHE_PROGRAM}")
	set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
	set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
else()
	message(STATUS "Not using ccache/sccache.")
endif()

include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake
  GIT_TAG origin/master
)
FetchContent_MakeAvailable(CPM)

CPMAddPackage("gh:raysan5/raylib#5.5") 

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	src/*.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} raylib)