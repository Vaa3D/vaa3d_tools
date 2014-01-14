# - Try to find LIBXML2
# Once done, this will define
#
#  LIBXML2_FOUND - system has LIBXML2
#  LIBXML2_INCLUDE_DIRS - the LIBXML2 include directories
#  LIBXML2_LIBRARIES - link these to use LIBXML2
#
# By default, the dynamic libraries will be found. To find the static ones instead,
# you must set the LIBXML2_STATIC_LIBRARY variable to TRUE before calling find_package.
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LIBXML2_PKGCONF libxml-2.0)

# attempt to find static library first if this is set
if(LIBXML2_STATIC_LIBRARY)
  set(LIBXML2_STATIC libxml2.a)
endif(LIBXML2_STATIC_LIBRARY)

# additional hints
if(MINGW)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} C:/Mingw)
endif(MINGW)

if(MSVC)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} D:/devlib/vc/libxml2)
endif(MSVC)

# Include dir
find_path(LIBXML2_INCLUDE_DIR
  NAMES libxml/xpath.h
  PATHS ${LIBXML2_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES libxml2
)

# Finally the library itself
find_library(LIBXML2_LIBRARY
  NAMES ${LIBXML2_STATIC} xml2 libxml2
  PATHS ${LIBXML2_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LIBXML2_PROCESS_INCLUDES LIBXML2_INCLUDE_DIR)
set(LIBXML2_PROCESS_LIBS LIBXML2_LIBRARY)
libfind_process(LIBXML2)