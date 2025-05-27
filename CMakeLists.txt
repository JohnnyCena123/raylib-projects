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

include(cmake/CPM.cmake)
CPMAddPackage("gh:raysan5/raylib#master") 

target_compile_options(raylib PRIVATE "-Wno-tautological-compare")

if(DEFINED CMAKE_LOCAL_BUILD_DO_NOT_DEFINE_PLEASE AND CMAKE_LOCAL_BUILD_DO_NOT_DEFINE_PLEASE)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}) # to separate different local branches and build types
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	src/*.cpp
)

if(WIN32)
	set(ICON_FILE ${CMAKE_SOURCE_DIR}/icon.rc)
	if(EXISTS ${ICON_FILE})
		set(RESOURCE_FILE ${ICON_FILE})
	else()
		set(RESOURCE_FILE)
	endif()
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCE_FILE})
	else()
		add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILE})
	endif()
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DCMAKE_RELEASE_BUILD)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} raylib)

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
if(EXISTS ${RESOURCE_DIR})
	file(
		COPY ${RESOURCE_DIR}
		DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
	)
endif()
