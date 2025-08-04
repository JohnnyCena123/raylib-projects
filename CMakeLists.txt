cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
project(${GIT_BRANCH}
	LANGUAGES CXX C
	VERSION 1.0.0
	HOMEPAGE_URL "https://github.com/JohnnyCena123/raylib-projects/tree/${GIT_BRANCH}"
)

option(USE_CACHING_COMPILER "Enables looking for a caching compiler" ON)
option(ALL_BUILD_TYPES_TOGETHER "Puts all outputs of all build types in one folder, and separates them with a suffix." ON)
option(LOCAL_CMAKE_BUILD "Enables separating outputs of different build types")
option(CUSTOM_OUTPUT_OPTIONS "Allows specifying a custom binary suffix and output directory" OFF)
option(DISABLE_WARNINGS "Disables warnings for dependencies" ON)
if(WIN32)
	option(INCLUDE_TERMINAL_IN_RELEASE "Keeps the terminal popup in Release builds when on Windows" OFF)
	option(INCLUDE_ICON "Includes the taskbar/explorer application icon when on Windows" ON)
endif()
option(USE_PRECOMPILED_HEADERS "Specifies whether or not the project headers will be compiled before the source, to optimize build times" ON)
option(BUILD_PORTABLE_APPLICATION "Makes the application portable by making sure everything will be in one folder" OFF)

if(NOT DONT_USE_CACHING_COMPILER)
	message(STATUS "Looking for ccache/sccache")
	find_program(CCACHE_PROGRAM ccache)
	find_program(SCCACHE_PROGRAM sccache)
	if(CCACHE_PROGRAM)
		message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
		set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
		set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	elseif(SCCACHE_PROGRAM)
		message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
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

include(cmake/CPM.cmake)

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
endif()
if(BUILD_SHARED_LIBS)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/..")
endif()
if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR STREQUAL "Xcode")
	foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
		string(TOUPPER "${CONFIG}" UPPER_CONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		if(BUILD_SHARED_LIBS)
			set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
			set_target_properties(clipboard PROPERTIES
				LIBRARY_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
			)
		endif()
	endforeach()
endif()

include(CheckCCompilerFlag)
CPMAddPackage("gh:raysan5/raylib#master")
check_c_compiler_flag("-Wno-tautological-compare" HAS_W_TAUTOLOGICAL_COMPARE)
if(HAS_W_TAUTOLOGICAL_COMPARE AND DISABLE_WARNINGS)
	target_compile_options(raylib PRIVATE "-Wno-tautological-compare")
endif()
if(BUILD_SHARED_LIBS AND DEFINED BIN_SUFFIX)
	set_target_properties(raylib PROPERTIES OUTPUT_NAME "raylib-${BIN_SUFFIX}")
endif()

# helper
if(PLATFORM STREQUAL "Desktop")
	set(PLATFORM_DESKTOP ON)
else()
	set(PLATFORM_DESKTOP OFF)
endif()

if(PLATFORM_DESKTOP)
	CPMAddPackage(NAME "tinyfd"
		GIT_REPOSITORY "https://git.code.sf.net/p/tinyfiledialogs/code"
		GIT_TAG "master"
	)
	set(TINYFD_TARGET "tinyfd")
	add_library(${TINYFD_TARGET} STATIC "${tinyfd_SOURCE_DIR}/tinyfiledialogs.c")

	if(DISABLE_WARNINGS)
		set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
	endif()
	CPMAddPackage("gh:jtanx/libclipboard#master")
	set(LIBCLIPBOARD_TARGET clipboard)
	target_include_directories(${LIBCLIPBOARD_TARGET} PRIVATE "${libclipboard_BINARY_DIR}/include")
	if(BUILD_SHARED_LIBS)
		set_target_properties(${LIBCLIPBOARD_TARGET} PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
		)
	endif()
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL Release OR IMGUI_IN_RELEASE)
	CPMAddPackage("gh:ocornut/imgui#eaac68c") 
	CPMAddPackage("gh:raylib-extras/rlImGui#9512b36")
	set(IMGUI_TARGET "imgui")
	add_library(${IMGUI_TARGET}
		${imgui_SOURCE_DIR}/imgui.cpp
		${imgui_SOURCE_DIR}/imgui_widgets.cpp
		${imgui_SOURCE_DIR}/imgui_tables.cpp
		${imgui_SOURCE_DIR}/imgui_draw.cpp
		${imgui_SOURCE_DIR}/imgui_demo.cpp
		${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
		
		${rlImGui_SOURCE_DIR}/rlImGui.cpp
	)
	target_include_directories(${IMGUI_TARGET} PRIVATE
		${raylib_SOURCE_DIR}/src
		${imgui_SOURCE_DIR}
		${rlImGui_SOURCE_DIR}
	)
	target_link_libraries(${IMGUI_TARGET} PRIVATE raylib)

	if(BUILD_SHARED_LIBS)
		if(DEFINED BIN_SUFFIX)
			set_target_properties(${IMGUI_TARGET} PROPERTIES OUTPUT_NAME "${IMGUI_TARGET}-${BIN_SUFFIX}")
		endif()
		target_compile_definitions(${IMGUI_TARGET} PRIVATE -DBUILD_LIBTYPE_SHARED)
		target_compile_definitions(${IMGUI_TARGET} PRIVATE -DIMGUI_EXPORTING)
		target_compile_definitions(${IMGUI_TARGET} PRIVATE -DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/imgui-config.hpp")
	endif()

	check_c_compiler_flag("-Wno-nontrivial-memcall" HAS_W_NONTRIVIAL_MEMCALL)
	check_c_compiler_flag("-Wno-nontrivial-memaccess" HAS_W_NONTRIVIAL_MEMACCESS)
	if(HAS_W_NONTRIVIAL_MEMCALL)
		target_compile_options(${IMGUI_TARGET} PRIVATE "-Wno-nontrivial-memcall")
	elseif(HAS_W_NONTRIVIAL_MEMACCESS)
		target_compile_options(${IMGUI_TARGET} PRIVATE "-Wno-nontrivial-memaccess")
	endif()
else()
	set(IMGUI_TARGET)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
message(STATUS "compiling ${SOURCES}")
if(NOT PLATFORM_DESKTOP)
	list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/cli.cpp")
endif()

if(WIN32)
	set(ICON_FILE ${CMAKE_SOURCE_DIR}/icon.rc)
	if(EXISTS ${ICON_FILE} AND INCLUDE_ICON)
		set(RESOURCE_FILE ${ICON_FILE})
	else()
		set(RESOURCE_FILE)
	endif()
	if(CONFIGURATION STREQUAL "Release" AND NOT INCLUDE_TERMINAL_IN_RELEASE)
		add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCE_FILE})
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DNO_TERMINAL_WINDOWS)
	else()
		add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILE})
	endif()
else()
	add_executable(${PROJECT_NAME} ${SOURCES})
endif()
if(USE_PRECOMPILED_HEADERS)
	file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS src/*.hpp)
	if(PLATFORM_DESKTOP)
		list(APPEND HEADERS "${tinyfd_SOURCE_DIR}/tinyfiledialogs.h")
	endif()
	target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADERS})
endif()
if(DEFINED BIN_SUFFIX)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}-${BIN_SUFFIX}")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
	BUILD_RPATH "$ORIGIN:$ORIGIN/..:$ORIGIN/../lib/${PROJECT_NAME}:/usr/lib/${PROJECT_NAME}"
)
if(CMAKE_BUILD_TYPE STREQUAL Release AND NOT IMGUI_IN_RELEASE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DIMGUI_OFF)
	message(STATUS "ImGui: OFF")
else()
	if(BUILD_SHARED_LIBS)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/imgui-config.hpp")
	endif()
	message(STATUS "ImGui: ON")
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="v${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_include_directories(${PROJECT_NAME} PRIVATE
	"${raylib_SOURCE_DIR}/src"
	# would be too much effort to remove these 3 for no reason.
	# ill just let it look for headers in /include and "" (empty path), which most likely doesnt exist, but it doesnt matter
	"${tinyfd_SOURCE_DIR}"
	"${libclipboard_SOURCE_DIR}/include"
	"${libclipboard_BINARY_DIR}/include"
	"${imgui_SOURCE_DIR}"
	"${rlImGui_SOURCE_DIR}"
)
target_link_libraries(${PROJECT_NAME} 
	raylib
	${TINYFD_TARGET}
	${LIBCLIPBOARD_TARGET}
	${IMGUI_TARGET}
)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(OUTPUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
set(STAMP_FILE "${CMAKE_BINARY_DIR}/resources_copied.stamp")
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

message(STATUS "output dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
