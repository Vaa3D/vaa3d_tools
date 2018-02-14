# needed for Qt 5
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# apple backwards compatibility
option(MACOSX_BACKWARDS_COMPATIBILITY "MacOSX 10.11 backwords compatibility mode" OFF)
if(APPLE AND MACOSX_BACKWARDS_COMPATIBILITY)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.6" CACHE STRING "Minimum OS X deployment version" FORCE)
	set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk")
	message(STATUS ${CMAKE_OSX_DEPLOYMENT_TARGET})
	message(STATUS ${MACOSX_DEPLOYMENT_TARGET})
endif()

project( TeraStitcher )

# include cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(Utils)
include(Setup)
include(Version)
include(InstallLocation)

# check GUI option
option (WITH_GUI "Build Qt-based GUI" OFF) 
if(WITH_GUI)
	message(STATUS "Include gui ...")
	add_subdirectory( gui )	
	message(STATUS "Include gui ... OK!")
endif(WITH_GUI)

# build modules
add_subdirectory( 3rdparty/zlib )
add_subdirectory( 3rdparty/libtiff )
add_subdirectory( 3rdparty/tinyxml )
add_subdirectory( imagemanager )
add_subdirectory( iomanager )
add_subdirectory( crossmips )
add_subdirectory( volumemanager )
add_subdirectory( stitcher )
add_subdirectory( common )
add_subdirectory( mappingmanager )
add_subdirectory( multilayersmanager )
add_subdirectory( ui )

# build user-selectable modules
# --- find modules ---
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/utils)
# --- ask for selection of the modules to be included in the project (default is OFF) ---
FOREACH(subdir ${SUBDIRS})
	option (WITH_UTILITY_MODULE_${subdir} "Include this utility module (optional) in the project" OFF) 
ENDFOREACH()
# --- include selected modules only ---
FOREACH(subdir ${SUBDIRS})
	if(WITH_UTILITY_MODULE_${subdir})
		message(STATUS "Include utils/" "${subdir} ...")
		add_subdirectory(utils/${subdir})
		message(STATUS "Include utils/" "${subdir} ... OK!")
	endif(WITH_UTILITY_MODULE_${subdir})
ENDFOREACH()

# set CPack
include(TeraStitcherCPack)