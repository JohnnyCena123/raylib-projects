cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)

project(Template)

message(STATUS "Looking for ccache/sccache")
find_program(CCACHE_PROGRAM ccache)
find_program(SCCACHE_PROGRAM sccache)
if(CCACHE_PROGRAM)
	message(NOTICE "Using ccache: ${CCACHE_PROGRAM}")
	set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
elseif(SCCACHE_PROGRAM)
	message(NOTICE "Using sccache: ${SCCACHE_PROGRAM}")
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/bin)

add_executable(${PROJECT_NAME}
	${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} raylib)

set(ICON_FILE ${CMAKE_SOURCE_DIR}/icon.rc)
if(EXISTS ${ICON_FILE} AND WIN32)
    set(APP_ICON_RESOURCE_WINDOWS ${ICON_FILE})
endif()

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
if(EXISTS ${RESOURCE_DIR})
	file(
		COPY ${RESOURCE_DIR}
		DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
	)
endif()
