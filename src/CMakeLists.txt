
file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

set(BUILD_METADATA_TARGET BuildMetadata)
add_subdirectory(metadata)

set(TARGET_ARGS ${SOURCES})

if(EMSCRIPTEN)
	list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/web.cpp")
elseif(UNIX AND NOT CYGWIN)
	if(APPLE)
		list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/mac.cpp")
	elseif(ANDROID)
		list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/android.cpp")
	else()
		list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux.cpp")
	endif()
elseif(WIN32 OR CYGWIN)
	set(ICON_FILE ${PROJECT_SOURCE_DIR}/icon.rc)
	set(RESOURCE_FILE ${ICON_FILE})
	list(APPEND TARGET_ARGS ${RESOURCE_FILE})
	list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows.cpp")
	if(CONFIGURATION STREQUAL "Release" AND NOT INCLUDE_TERMINAL_IN_RELEASE)
		list(INSERT TARGET_ARGS 0 "WIN32")
		list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows-no-terminal.cpp")
	endif()
endif()
if(PLATFORM_DESKTOP)
	list(APPEND TARGET_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/platform/desktop.cpp")
endif()
add_executable(${PROJECT_NAME} ${TARGET_ARGS})

target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/platform/platforms.hpp")
if(USE_PRECOMPILED_HEADERS)
	file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.hpp")
	if(PLATFORM_DESKTOP)
		list(APPEND HEADERS "${tinyfd_SOURCE_DIR}/tinyfiledialogs.h" "${lcb_SOURCE_DIR}/include/libclipboard.h")
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

if(CONFIGURATION STREQUAL Release AND NOT IMGUI_IN_RELEASE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DIMGUI_OFF)
	message(STATUS "ImGui: OFF")
else()
	if(BUILD_SHARED_LIBS)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DIMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/src/imgui-config.hpp")
	endif()
	message(STATUS "ImGui: ON")
endif()
if(WIN32 AND DISABLE_WARNINGS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIGURATION=${CONFIGURATION})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_include_directories(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/metadata"
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
	${LCB_TARGET}
	${IMGUI_TARGET}
	${BUILD_METADATA_TARGET}
)

if(PLATFORM_DESKTOP)
	set(PORTABLE_INDICATOR_FILE ".portable-application")
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPORTABLE_INDICATOR_FILE="${PORTABLE_INDICATOR_FILE}")
	if(BUILD_PORTABLE_APPLICATION)
		add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PORTABLE_INDICATOR_FILE}"
		)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DPORTABLE)
	endif()

	if(NOT MSVC)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
		if(BUILD_SHARED_LIBS)
			foreach(LIB ${RAYLIB_TARGET} ${TINYFD_TARGET} ${LCB_TARGET} ${IMGUI_TARGET})
				set_target_properties(${LIB} PROPERTIES LINK_FLAGS_RELEASE "-s")
			endforeach()
		endif()
	endif()
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/help-launcher")
endif()

if(EMSCRIPTEN)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
		-sUSE_GLFW=3 \
		-sASSERTIONS=1 \
		-sWASM=1 -sASYNCIFY \
		-sGL_ENABLE_GET_PROC_ADDRESS=1 \
		-sEXPORTED_RUNTIME_METHODS=ccall \
		-sALLOW_MEMORY_GROWTH \
		-sABORTING_MALLOC=0 \
		--preload-file '${PROJECT_SOURCE_DIR}/resources@resources' \
		--shell-file '${raylib_SOURCE_DIR}/src/minshell.html'
	")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
	set(RESOURCE_DIR "${PROJECT_SOURCE_DIR}/resources")
	set(OUTPUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
	set(RESOURCES_STAMP_FILE "${CMAKE_BINARY_DIR}/dummy3.stamp")
	file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${RESOURCE_DIR}/*")

	add_custom_command(
		TARGET ${PROJECT_NAME} PRE_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DSOURCE_DIR=${RESOURCE_DIR}
			-DDESTINATION_DIR=${OUTPUT_RESOURCE_DIR}
			-DSTAMP_FILE=${RESOURCES_STAMP_FILE}
			-P "${PROJECT_SOURCE_DIR}/cmake/CopyResources.cmake"
		DEPENDS ${RESOURCE_FILES}
		COMMENT "Copying resources to output directory..."
		VERBATIM
	)

	include(InstallConfig)
	include(PackageConfig)
endif()
