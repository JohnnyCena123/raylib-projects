cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

if(NOT DEFINED GIT_BRANCH)
	if(EXISTS "${CMAKE_SOURCE_DIR}/.git-branch")
		file(READ "${CMAKE_SOURCE_DIR}/.git-branch" GIT_BRANCH)
		string(REGEX REPLACE "\\s" "" GIT_BRANCH "${GIT_BRANCH}")
	else()
		execute_process(
			COMMAND git rev-parse --abbrev-ref HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_BRANCH
			RESULT_VARIABLE GIT_RESULT
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(NOT GIT_RESULT STREQUAL "0")
			message(FATAL_ERROR "Unable to determine the git branch!\n"
				"Please specify it via the command-line using -DGIT_BRANCH=example,\n"
				"or by adding a .git-branch file containing the git branch to the root of this project ."
			)
		endif()
		configure_file("${CMAKE_SOURCE_DIR}/cmake/templates/.git-branch.in"
			"${CMAKE_SOURCE_DIR}/.git-branch")
	endif()
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

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(OUTPUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
set(RESOURCES_STAMP_FILE "${CMAKE_BINARY_DIR}/dummy3.stamp")
file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${RESOURCE_DIR}/*")

if(EMSCRIPTEN)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
		-sUSE_GLFW=3 \
		-sASSERTIONS=1 \
		-sWASM=1 -sASYNCIFY \
		-sGL_ENABLE_GET_PROC_ADDRESS=1 \
		-sEXPORTED_RUNTIME_METHODS=ccall \
		--preload-file ../resources \
		--shell-file '${raylib_SOURCE_DIR}/src/minshell.html'
	")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
	add_custom_command(
		OUTPUT "${RESOURCES_STAMP_FILE}"
		COMMAND ${CMAKE_COMMAND}
			-DSOURCE_DIR=${RESOURCE_DIR}
			-DDESTINATION_DIR=${OUTPUT_RESOURCE_DIR}
			-DSTAMP_FILE=${RESOURCES_STAMP_FILE}
			-P "${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake"
		DEPENDS ${RESOURCE_FILES}
		COMMENT "Copying resources to output directory..."
		VERBATIM
	)
	add_dependencies(${PROJECT_NAME} CopyResources)
	add_custom_target(CopyResources ALL DEPENDS "${RESOURCES_STAMP_FILE}")
endif()

if(PLATFORM_DESKTOP)
	set(PORTABLE_INDICATOR_FILE ".portable-application")
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPORTABLE_INDICATOR_FILE="${PORTABLE_INDICATOR_FILE}")
	if(BUILD_PORTABLE_APPLICATION)
		add_custom_command(
			OUTPUT "${CMAKE_BINARY_DIR}/unused"
			COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PORTABLE_INDICATOR_FILE}"
		)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DPORTABLE)
	endif()
endif()

include("${CMAKE_SOURCE_DIR}/cmake/InstallConfig.cmake")
