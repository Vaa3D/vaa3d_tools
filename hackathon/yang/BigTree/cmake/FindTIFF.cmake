# - Find TIFF
#
# Find the TIFF includes and libraries
# This module defines
#  TIFF_FOUND           - TRUE if found, FALSE otherwise
#  TIFF_INCLUDE_DIR     - Include directories for TIFF
#  TIFF_LIBRARIES       - The libraries to link against to use TIFF

FIND_PACKAGE(PackageHandleStandardArgs)

SET(TIFF_SEARCHPATH
    /usr/include/
    /usr/local/include/
    /opt/include/
    /opt/local/include/
    )

FIND_PATH(TIFF_INCLUDE_DIR
    NAMES tiff.h
    PATHS ${TIFF_SEARCHPATH}
    DOC "The TIFF include directory")

FIND_LIBRARY(TIFF_LIBRARY
    NAMES tiff
    PATHS
    /usr/lib
    /usr/local/lib
    /usr/lib64
    /usr/local/lib64
    /opt/lib
    /opt/local/lib
    /opt/lib64
    /opt/local/lib64
    DOC "The TIFF libraries")

FIND_LIBRARY(TIFFXX_LIBRARY
    NAMES tiffxx
    PATHS
    /usr/lib
    /usr/local/lib
    /usr/lib64
    /usr/local/lib64
    /opt/lib
    /opt/local/lib
    /opt/lib64
    /opt/local/lib64
    DOC "The TIFFXX libraries")


SET(TIFF_LIBRARIES ${TIFF_LIBRARY} ${TIFFXX_LIBRARY})

IF(TIFF_INCLUDE_DIR AND TIFF_LIBRARIES)
    SET(TIFF_FOUND TRUE)
ELSE(TIFF_INCLUDE_DIR AND TIFF_LIBRARIES)
    SET(TIFF_FOUND FALSE)
ENDIF(TIFF_INCLUDE_DIR AND TIFF_LIBRARIES)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TIFF REQUIRED_VARS TIFF_INCLUDE_DIR TIFF_LIBRARIES)

MARK_AS_ADVANCED( TIFF_INCLUDE_DIR TIFF_LIBRARIES )
