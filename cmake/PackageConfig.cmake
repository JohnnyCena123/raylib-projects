
if(UNIX)
	if(APPLE)
		set(CPACK_GENERATOR "TGZ;ZIP;7Z;STGZ;DragNDrop;productbuild")
	else()
		set(CPACK_GENERATOR "TGZ;ZIP;7Z;STGZ;DEB;RPM")
		set(DESKTOP_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop")
		install(FILES "${DESKTOP_FILE}" DESTINATION share/applications)
		install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.png" DESTINATION share/icons/hicolor/64x64/apps/)
		if(NOT DEFINED CPACK_PACKAGING_INSTALL_PREFIX)
			set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
		endif()
		set(COMMENT "@MIN_DESCRIPTION@")
		configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/project.desktop.in"
			"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop.in")
		configure_file("${PROJECT_SOURCE_DIR}/resources/icon.png"
			"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.png" COPYONLY)
	endif()
elseif(WIN32 OR CYGWIN)
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
	"/*build*/"
	"/bin/"
)
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Johnny Cena")
set(CPACK_PACKAGE_CONTACT "iamahuman1395@gmail.com")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/icon.png")
set(CPACK_PACKAGE_DOCUMENTATION_FILES "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_CREATE_DESKTOP_LINKS "$<TARGET_FILE:${PROJECT_NAME}>")

configure_file("${PROJECT_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
if(APPLE)
	configure_file("${PROJECT_SOURCE_DIR}/README.md" "${CMAKE_BINARY_DIR}/README.md.txt")
	set(CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/README.md.txt")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
else()
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
endif()
set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/resources/icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/resources/icon.ico")
if(WIN32)
	string(REPLACE "/" "\\\\" CPACK_PACKAGE_ICON "${CPACK_PACKAGE_ICON}")
endif()

# set(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libopengl0, libglx0, libglu1-mesa, libstdc++6, libgcc-s1, libx11-6, libxcb1")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Johnny Cena <${CPACK_PACKAGE_CONTACT}>")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")

set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_GROUP "Games")

set(CPACK_MONOLITHIC_INSTALL ON)
include(CPack)

add_custom_command(
	OUTPUT "${CMAKE_BINARY_DIR}/dummy5.stamp"
	COMMAND ${CMAKE_COMMAND}
		-DEXE_PATH="$<TARGET_FILE:${PROJECT_NAME}>"
		-DSOURCE_CPACK_PROPERTIES_FILEPATH="${PROJECT_SOURCE_DIR}/cmake/templates/CPackProperties.cmake.in"
		-DCPACK_PROPERTIES_FILEPATH="${CMAKE_BINARY_DIR}/CPackProperties.cmake"
		-DDESKTOP_FILE="${DESKTOP_FILE}"
		-P "${PROJECT_SOURCE_DIR}/cmake/InjectPackageInfo.cmake"
	COMMENT "Generating package information..."
)
add_custom_target(GeneratePackageInfo ALL DEPENDS "${CMAKE_BINARY_DIR}/dummy5.stamp")
