cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
project(${GIT_BRANCH}) # i might decide to change this in specific branches but overall itll be the default

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
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(raylib PRIVATE "-Wno-tautological-compare")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	src/*.cpp
)

if(DEFINED CMAKE_BUILD_TYPE)
	set(CONFIGURATION ${CMAKE_BUILD_TYPE})
else()
	set(CONFIGURATION $<CONFIG>)
endif()

if(DEFINED CMAKE_LOCAL_BUILD_DO_NOT_DEFINE_PLEASE AND CMAKE_LOCAL_BUILD_DO_NOT_DEFINE_PLEASE)
	# to separate different local branches
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME})
	# and separate different build types when building locally
	if(CONFIGURATION STREQUAL "Release")
		set(EXECUTABLE_NAME "${PROJECT_NAME}")
	else()
		set(EXECUTABLE_NAME "${PROJECT_NAME}-${CONFIGURATION}")
	endif()
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
	# separating different platforms when building in github actions
	if(WIN32)
		set(EXECUTABLE_NAME "${PROJECT_NAME}-win")
	elseif(APPLE)
		set(EXECUTABLE_NAME "${PROJECT_NAME}-mac")
	else()
		set(EXECUTABLE_NAME "${PROJECT_NAME}-linux")
	endif()
endif()
if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR STREQUAL "Xcode")
	foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
		string(TOUPPER "${CONFIG}" UPPER_CONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	endforeach()
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
	if(CONFIGURATION STREQUAL "Release")
		add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCE_FILE})
	else()
		add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILE})
	endif()
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})
if(CMAKE_BUILD_TYPE STREQUAL Release)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DCMAKE_RELEASE_BUILD)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} raylib)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(OUTPUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
if(EXISTS ${RESOURCE_DIR})
	file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/resources/*.*)

	add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/resources_copied.stamp
		BYPRODUCTS 
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_RESOURCE_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_DIR} ${OUTPUT_RESOURCE_DIR}
		COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/resources_copied.stamp"
		DEPENDS ${RESOURCE_FILES}
		COMMENT "Copying resources to output directory..."
		VERBATIM
	)
	add_custom_target(CopyResources ALL
		DEPENDS "${CMAKE_BINARY_DIR}/resources_copied.stamp"
	)
endif()