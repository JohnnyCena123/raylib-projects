cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)

execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
project(${GIT_BRANCH}) # i might decide to change this in specific branches but overall itll be the default

if(NOT DONT_USE_CACHING_COMPILER)
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
else()
	message(STATUS "Not looking for ccache/sccache.")
endif()

if(DEFINED CMAKE_BUILD_TYPE)
	set(CONFIGURATION ${CMAKE_BUILD_TYPE})
else()
	set(CONFIGURATION $<CONFIG>)
endif()

if(NOT CUSTOM_OUTPUT_OPTIONS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

if(CMAKE_LOCAL_BUILD)
	# separate different build types when building locally
	if(CONFIGURATION STREQUAL "Release")
		set(BIN_SUFFIX) # nothing
	else()
		set(BIN_SUFFIX "${CONFIGURATION}")
	endif()
endif()
if (BUILD_SHARED_LIBS)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()
if((CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR STREQUAL "Xcode") AND NOT SEPARATE_DIFFERENT_BUILD_TYPES_MULTI_CONFIG)
	foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
		string(TOUPPER "${CONFIG}" UPPER_CONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		if (BUILD_SHARED_LIBS)
			set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
		endif()
	endforeach()
endif()

include(cmake/CPM.cmake)
include(CheckCCompilerFlag)
CPMAddPackage("gh:raysan5/raylib#master")
check_c_compiler_flag("-Wno-tautological-compare" HAS_W_TAUTOLOGICAL_COMPARE)
if(HAS_W_TAUTOLOGICAL_COMPARE AND NOT DONT_DISABLE_WARNINGS)
	target_compile_options(raylib PRIVATE "-Wno-tautological-compare")
endif()
if(BUILD_SHARED_LIBS AND DEFINED BIN_SUFFIX)
	set_target_properties(raylib PROPERTIES OUTPUT_NAME "raylib-${BIN_SUFFIX}")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

if(WIN32)
	set(ICON_FILE ${CMAKE_SOURCE_DIR}/icon.rc)
	if(EXISTS ${ICON_FILE} AND NOT DONT_INCLUDE_ICON)
		set(RESOURCE_FILE ${ICON_FILE})
	else()
		set(RESOURCE_FILE)
	endif()
	if(CONFIGURATION STREQUAL "Release" AND NOT INCLUDE_TERMINAL_IN_RELEASE_WINDOWS)
		add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCE_FILE})
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DNO_TERMINAL_WINDOWS)
	else()
		add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILE})
	endif()
else()
	add_executable(${PROJECT_NAME} ${SOURCES})
endif()
if(NOT NO_PRECOMPILED_HEADERS)
	file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS src/*.hpp)
	target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADERS})
endif()
if(DEFINED BIN_SUFFIX)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}-${BIN_SUFFIX}")
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} raylib)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
if(EXISTS ${RESOURCE_DIR})
	set(OUTPUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
	set(STAMP_FILE "${CMAKE_BINARY_DIR}/resources_copied.stamp")
	file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${RESOURCE_DIR}/*")

	add_custom_command(
		OUTPUT "${STAMP_FILE}"
		COMMAND ${CMAKE_COMMAND}
			-DSOURCE_DIR=${RESOURCE_DIR}
			-DDESTINATION_DIR=${OUTPUT_RESOURCE_DIR}
			-DSTAMP_FILE=${STAMP_FILE}
			-P "${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake"
		DEPENDS ${RESOURCE_FILES}
		COMMENT "Copying resources to output directory..."
		VERBATIM
	)
	add_dependencies(${PROJECT_NAME} CopyResources)
	add_custom_target(CopyResources ALL
		DEPENDS "${STAMP_FILE}"
	)
endif()