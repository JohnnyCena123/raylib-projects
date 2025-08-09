
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
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
	file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.hpp")
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
	${LCB_TARGET}
	${IMGUI_TARGET}
)

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
