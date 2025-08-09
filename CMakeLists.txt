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
		configure_file("${CMAKE_SOURCE_DIR}/cmake/.git-branch.in"
			"${CMAKE_SOURCE_DIR}/.git-branch")
	endif()
endif()
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

include(CheckCCompilerFlag)
CPMAddPackage("gh:raysan5/raylib#master")
set(RAYLIB_TARGET raylib)
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
	set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
	CPMAddPackage(NAME lcb
		GITHUB_REPOSITORY "jtanx/libclipboard"
		GIT_TAG master
	)
	if(EXISTS "${lcb_SOURCE_DIR}/third_party/googletest")
		execute_process(
			WORKING_DIRECTORY ${lcb_SOURCE_DIR}
			COMMAND git submodule deinit "third_party/googletest"
			COMMAND_ERROR_IS_FATAL ANY
		)
	endif()

	set(LIBCLIPBOARD_TARGET clipboard)
	target_include_directories(${LIBCLIPBOARD_TARGET} PRIVATE "${lcb_BINARY_DIR}/include")
	set_target_properties(${LIBCLIPBOARD_TARGET} PROPERTIES
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
	target_link_libraries(${IMGUI_TARGET} PRIVATE ${RAYLIB_TARGET})

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
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
if(NOT PLATFORM_DESKTOP)
	list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/cli.cpp")
endif()

set(BUILD_METADATA_FILE "${CMAKE_SOURCE_DIR}/src/build-metadata.cpp")
list(REMOVE_ITEM SOURCES "${BUILD_METADATA_FILE}")

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

set(EXE_NAME ${PROJECT_NAME})
if(DEFINED BIN_SUFFIX)
	string(APPEND EXE_NAME "-${BIN_SUFFIX}")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${EXE_NAME}")

set(APP_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/..:$ORIGIN/../lib:$ORIGIN/../lib/${PROJECT_NAME}")
if(UNIX)
	string(APPEND APP_RPATH ":/usr/lib:/usr/lib/${PROJECT_NAME}")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "${APP_RPATH}")

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
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_include_directories(${PROJECT_NAME} PRIVATE
	"${raylib_SOURCE_DIR}/src"
	# would be too much effort to remove these 3 for no reason.
	# ill just let it look for headers in /include and "" (empty path), which most likely doesnt exist, but it doesnt matter
	"${tinyfd_SOURCE_DIR}"
	"${lcb_SOURCE_DIR}/include"
	"${lcb_BINARY_DIR}/include"
	"${imgui_SOURCE_DIR}"
	"${rlImGui_SOURCE_DIR}"
)
target_link_libraries(${PROJECT_NAME} 
	${RAYLIB_TARGET}
	${TINYFD_TARGET}
	${LIBCLIPBOARD_TARGET}
	${IMGUI_TARGET}
)

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
	add_custom_target(CopyResources ALL
		DEPENDS "${RESOURCES_STAMP_FILE}"
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

set(BUILD_METADATA_LIB BuildMetadata)
add_library(${BUILD_METADATA_LIB} STATIC "${CMAKE_SOURCE_DIR}/src/build-metadata.cpp")
add_custom_command(
	TARGET ${BUILD_METADATA_LIB} PRE_BUILD
	COMMAND "${CMAKE_COMMAND}" -E touch "${BUILD_METADATA_FILE}"
	VERBATIM
)
add_custom_target(GenerateBuildMetadata DEPENDS "${CMAKE_SOURCE_DIR}/${BUILD_METADATA_FILE}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CMAKE_SYSTEM="${CMAKE_SYSTEM}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CMAKE_SYSTEM_NAME="${CMAKE_SYSTEM_NAME}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CMAKE_VERSION="${CMAKE_VERSION}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE C_COMPILER="${CMAKE_C_COMPILER_ID}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE C_COMPILER_VERSION="${CMAKE_C_COMPILER_VERSION}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CXX_COMPILER="${CMAKE_CXX_COMPILER_ID}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE C_LINKER="${CMAKE_C_COMPILER_LINKER_ID}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE C_LINKER_VERSION="${CMAKE_C_COMPILER_LINKER_VERSION}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CXX_LINKER="${CMAKE_CXX_COMPILER_LINKER_ID}")
target_compile_definitions(${BUILD_METADATA_LIB} PRIVATE CXX_LINKER_VERSION="${CMAKE_CXX_COMPILER_LINKER_VERSION}")
target_link_libraries(${PROJECT_NAME} ${BUILD_METADATA_LIB})


set(INSTALL_TARGETS ${PROJECT_NAME})
if(BUILD_SHARED_LIBS)
	list(APPEND INSTALL_TARGETS ${RAYLIB_TARGET} ${LIBCLIPBOARD_TARGET} ${IMGUI_TARGET})
endif()
install(
	TARGETS ${INSTALL_TARGETS}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib/${PROJECT_NAME}
)
install(FILES
	"${CMAKE_SOURCE_DIR}/LICENSE" 
	"${CMAKE_SOURCE_DIR}/README.md" 
	DESTINATION "share/${PROJECT_NAME}"
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources"
	DESTINATION "share/${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${APP_RPATH}")

if(UNIX)
	if(APPLE)
		set(CPACK_GENERATOR "TGZ;ZIP;7Z;STGZ;DragNDrop;Bundle;ProductBuild")
	else()
		set(CPACK_GENERATOR "TGZ;ZIP;7Z;STGZ;DEB;RPM")
		set(DESKTOP_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop")
		install(FILES "${DESKTOP_FILE}" DESTINATION share/applications
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
				GROUP_READ GROUP_EXECUTE 
				WORLD_READ WORLD_EXECUTE
		)
		install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.png" DESTINATION share/icons/hicolor/64x64/apps/)
		if(NOT DEFINED CPACK_PACKAGING_INSTALL_PREFIX)
			set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
		endif()
		set(COMMENT "@MIN_DESCRIPTION@. Source code: @REPO@.")
		configure_file("${CMAKE_SOURCE_DIR}/cmake/project.desktop.in" 
			"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop.in")
		configure_file("${CMAKE_SOURCE_DIR}/resources/icon.png"
			"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.png" COPYONLY)
	endif()
elseif(WIN32)
	set(CPACK_GENERATOR "TGZ;ZIP;7Z;NSIS;WIX")
endif()
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP;7Z")

set(CPACK_SOURCE_IGNORE_FILES
	"/\\\\.gitignore$"
	"/\\\\.clangd$"
	"/\\\\.DS_Store$"
	"/\\\\.git/"
	"/\\\\.github/"
	"/git-ignore/"
	"/\\\\.vscode/"
	"/\\\\.cache/"
	"/build/"
	"/bin/"
)
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Johnny Cena")
set(CPACK_PACKAGE_CONTACT "iamahuman1395@gmail.com")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/icon.png")
set(CPACK_PACKAGE_DOCUMENTATION_FILES "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_CREATE_DESKTOP_LINKS "$<TARGET_FILE:${PROJECT_NAME}>")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
else()
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
endif()
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libopengl0, libglx0, libglu1-mesa, libstdc++6, libgcc-s1, libx11-6, libxcb1")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Johnny Cena <${CPACK_PACKAGE_CONTACT}>")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")

set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_GROUP "Games")

set(CPACK_MONOLITHIC_INSTALL ON)
include(CPack)

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND}
		-DEXE_PATH="$<TARGET_FILE:${PROJECT_NAME}>"
		-DSOURCE_CPACK_PROPERTIES_FILEPATH="${CMAKE_SOURCE_DIR}/cmake/CPackProperties.cmake.in"
		-DCPACK_PROPERTIES_FILEPATH="${CMAKE_BINARY_DIR}/CPackProperties.cmake"
		-DDESKTOP_FILE="${DESKTOP_FILE}"
		-P "${CMAKE_SOURCE_DIR}/cmake/InjectPackageInfo.cmake"
)
