
include(CPM)
include(CheckCCompilerFlag)
CPMAddPackage("gh:JohnnyCena123/raylib#cmake-web-fix")
set(RAYLIB_TARGET raylib CACHE STRING "Name of raylib's target.")
if(DISABLE_WARNINGS)
	check_c_compiler_flag("-Wno-tautological-compare" HAS_W_NO_TAUTOLOGICAL_COMPARE)
	check_c_compiler_flag("-Wno-unused-result" HAS_W_NO_UNUSED_RESULT)
	check_c_compiler_flag("-Wno-stringop-overflow" HAS_W_NO_STRINGOP_OVERFLOW)
	if (HAS_W_NO_TAUTOLOGICAL_COMPARE)
		target_compile_options(${RAYLIB_TARGET} PRIVATE "-Wno-tautological-compare")
	endif()
	if (HAS_W_NO_UNUSED_RESULT)
		target_compile_options(${RAYLIB_TARGET} PRIVATE "-Wno-unused-result")
	endif()
	if (HAS_W_NO_STRINGOP_OVERFLOW)
		target_compile_options(${RAYLIB_TARGET} PRIVATE "-Wno-stringop-overflow")
	endif()
endif()
if(BUILD_SHARED_LIBS AND DEFINED BIN_SUFFIX)
	set_target_properties(${RAYLIB_TARGET} PROPERTIES OUTPUT_NAME "${RAYLIB_TARGET}-${BIN_SUFFIX}")
endif()

# helper
if(PLATFORM STREQUAL "Desktop")
	set(PLATFORM_DESKTOP ON CACHE BOOL "Whether the target platform is desktop.")
else()
	set(PLATFORM_DESKTOP OFF CACHE BOOL "Whether the target platform is desktop.")
endif()

if(PLATFORM_DESKTOP)
	CPMAddPackage(NAME "tinyfd"
		GIT_REPOSITORY "https://git.code.sf.net/p/tinyfiledialogs/code"
		GIT_TAG "master"
	)
	set(TINYFD_TARGET "tinyfd")
	add_library(${TINYFD_TARGET} STATIC "${tinyfd_SOURCE_DIR}/tinyfiledialogs.c")
	check_c_compiler_flag("-Wno-unused-result" HAS_W_NO_UNUSED_RESULT)
	if(HAS_W_NO_UNUSED_RESULT)
		target_compile_options(tinyfd PRIVATE "-Wno-unused-result")
	endif()

	if(DISABLE_WARNINGS)
		set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
	endif()
	set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
	CPMAddPackage(NAME lcb
		GITHUB_REPOSITORY "JohnnyCena123/libclipboard"
		GIT_TAG master
	)

	set(LCB_TARGET clipboard CACHE STRING "Name of libclipboard's target.")
	target_include_directories(${LCB_TARGET} PRIVATE "${lcb_BINARY_DIR}/include")
	set_target_properties(${LCB_TARGET} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
	)

	if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR STREQUAL "Xcode")
		foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
			string(TOUPPER "${CONFIG}" UPPER_CONFIG)
			set_target_properties(clipboard PROPERTIES
				LIBRARY_OUTPUT_DIRECTORY_${UPPER_CONFIG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
			)
		endforeach()
	endif()
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL Release OR IMGUI_IN_RELEASE)
	CPMAddPackage("gh:ocornut/imgui#eaac68c")
	CPMAddPackage("gh:raylib-extras/rlImGui#9512b36")
	set(IMGUI_TARGET imgui CACHE STRING "Name of ImGui's target." FORCE)
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
	target_link_libraries(${IMGUI_TARGET} PRIVATE ${RAYLIB_TARGET})

	if(BUILD_SHARED_LIBS)
		if(DEFINED BIN_SUFFIX)
			set_target_properties(${IMGUI_TARGET} PROPERTIES OUTPUT_NAME "${IMGUI_TARGET}-${BIN_SUFFIX}")
		endif()
		target_compile_definitions(${IMGUI_TARGET} PRIVATE -DBUILD_LIBTYPE_SHARED)
		target_compile_definitions(${IMGUI_TARGET} PRIVATE -DIMGUI_EXPORTING)
		target_compile_definitions(${IMGUI_TARGET} PRIVATE -DIMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/src/imgui-config.hpp")
	endif()

	check_c_compiler_flag("-Wno-nontrivial-memcall" HAS_W_NONTRIVIAL_MEMCALL)
	check_c_compiler_flag("-Wno-nontrivial-memaccess" HAS_W_NONTRIVIAL_MEMACCESS)
	if(HAS_W_NONTRIVIAL_MEMCALL)
		target_compile_options(${IMGUI_TARGET} PRIVATE "-Wno-nontrivial-memcall")
	elseif(HAS_W_NONTRIVIAL_MEMACCESS)
		target_compile_options(${IMGUI_TARGET} PRIVATE "-Wno-nontrivial-memaccess")
	endif()
else()
	set(IMGUI_TARGET "" CACHE STRING "Name of ImGui's target." FORCE)
endif()
