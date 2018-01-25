# set CPack options

# set package description file (ignored by certain installers)
set (CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../DESCRIPTION.txt)

# set package license (Apple DragNDrop needs a file with UNIX ending)
if(APPLE)
	set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE_APPLE.txt)
else()
	set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt)
endif()

# set package short description (ignored by certain installers)
set (CPACK_PACKAGE_DESCRIPTION "A tool for fast automatic 3D-stitching of teravoxel-sized microscopy images")

# set package version
set (CPACK_PACKAGE_VERSION_MAJOR ${TeraStitcher_VERSION_MAJOR} )
set (CPACK_PACKAGE_VERSION_MINOR ${TeraStitcher_VERSION_MINOR} )
set (CPACK_PACKAGE_VERSION_PATCH ${TeraStitcher_VERSION_PATCH} )

# set package contact
set (CPACK_PACKAGE_CONTACT "Alessandro Bria (a.bria@unicas.it)")

# set package name (depends on Qt)
if(WITH_QT5)
	set(CPACK_PACKAGE_NAME "TeraStitcher-Qt5-standalone")
elseif(WITH_QT4)
	set(CPACK_PACKAGE_NAME "TeraStitcher-Qt4-standalone")
else()
	set(CPACK_PACKAGE_NAME "TeraStitcher-standalone")
endif()

# set other O.S. specific packaging options
# APPLE
if(APPLE)
	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
	set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../src/gui/icons/terastitcher.icns")
	set(CPACK_PACKAGING_INSTALL_PREFIX "/${CPACK_PACKAGE_NAME}")
	set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../src/gui/icons/terastitcher.icns")
endif()
# Windows
if(WIN32)
	string (CONCAT DESKTOP_LINK_NAME 
		"TeraStitcher " 
		${TeraStitcher_VERSION_MAJOR} 
		"." 
		${TeraStitcher_VERSION_MINOR} 
		"." 
		${TeraStitcher_VERSION_PATCH})
	set (CPACK_PACKAGE_EXECUTABLES "terastitcher-gui" "${DESKTOP_LINK_NAME}")
	set (CPACK_CREATE_DESKTOP_LINKS "terastitcher-gui")
endif()

# set other installer specific options
configure_file("${CMAKE_CURRENT_LIST_DIR}/TeraStitcherCPackOptions.cmake.in"
  "${TeraStitcher_BINARY_DIR}/TeraStitcherCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE
  "${TeraStitcher_BINARY_DIR}/TeraStitcherCPackOptions.cmake")

# activate cpack
include (CPack )