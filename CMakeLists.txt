cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
if(NOT DEFINED GIT_BRANCH)
	set(GIT_BRANCH_FILE "${CMAKE_SOURCE_DIR}/.git-branch")
	message(STATUS "Checking ${GIT_BRANCH_FILE} for the git branch")
	execute_process(
		COMMAND git symbolic-ref -q --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_IS_BRANCH
		RESULT_VARIABLE GIT_RESULT1
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(EXISTS "${GIT_BRANCH_FILE}" AND NOT (
		LOCAL_CMAKE_BUILD AND NOT (GIT_RESULT1 STREQUAL "0"
		AND NOT GIT_IS_BRANCH STREQUAL ""
	)))
		file(READ "${GIT_BRANCH_FILE}" GIT_BRANCH)
		string(REGEX REPLACE "[ \t\n]" "" GIT_BRANCH "${GIT_BRANCH}")
		message(STATUS "Read branch from file: ${GIT_BRANCH}")
	else()
		execute_process(
			COMMAND git rev-parse --abbrev-ref HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_BRANCH
			RESULT_VARIABLE GIT_RESULT2
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(NOT GIT_RESULT2 STREQUAL "0")
			message(FATAL_ERROR "Unable to determine the git branch!\n"
				"Please specify it via the command-line using -DGIT_BRANCH=example,\n"
				"or by adding a .git-branch file containing the git branch to the root of this project ."
			)
		endif()
		message(STATUS "Detected branch from git: ${GIT_BRANCH}")
	endif()
	configure_file("${CMAKE_SOURCE_DIR}/cmake/templates/.git-branch.in"
		"${CMAKE_SOURCE_DIR}/.git-branch")
endif()
project(${GIT_BRANCH}
	LANGUAGES CXX C
	VERSION 1.0.0
	HOMEPAGE_URL "https://github.com/JohnnyCena123/raylib-projects/tree/${GIT_BRANCH}"
)
include("${CMAKE_SOURCE_DIR}/cmake/CMakeOptions.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/FindCacher.cmake")
if(DEFINED CMAKE_BUILD_TYPE)
	set(CONFIGURATION ${CMAKE_BUILD_TYPE})
else()
	set(CONFIGURATION $<CONFIG>)
endif()
if(ALL_BUILD_TYPES_TOGETHER)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
elseif(NOT CUSTOM_OUTPUT_OPTIONS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$CONFIGURATION")
endif()
if(LOCAL_CMAKE_BUILD AND NOT CUSTOM_OUTPUT_OPTIONS)
	# separate different projects
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}")
	# separate different build types
	if(CONFIGURATION STREQUAL "Release")
		set(BIN_SUFFIX) # nothing
	else()
		set(BIN_SUFFIX "${CONFIGURATION}")
	endif()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/..")
elseif(NOT CUSTOM_OUTPUT_OPTIONS)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()
if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR STREQUAL "Xcode")
	foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER "${CONFIG}" UPPER_CONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
	endforeach()
endif()
include("${CMAKE_SOURCE_DIR}/cmake/Deps.cmake")
set(IMGUI_TARGET ${IMGUI_TARGET})
add_subdirectory("${CMAKE_SOURCE_DIR}/src")
