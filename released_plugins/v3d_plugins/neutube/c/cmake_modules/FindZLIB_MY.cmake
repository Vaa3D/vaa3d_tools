# - Try to find ZLIB
# Once done, this will define
#
#  ZLIB_FOUND - system has ZLIB
#  ZLIB_INCLUDE_DIRS - the ZLIB include directories
#  ZLIB_LIBRARIES - link these to use ZLIB
#
# By default, the dynamic libraries will be found. To find the static ones instead,
# you must set the ZLIB_STATIC_LIBRARY variable to TRUE before calling find_package.
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(ZLIB_PKGCONF ZLIB)

# attempt to find static library first if this is set
if(ZLIB_STATIC_LIBRARY)
  set(ZLIB_STATIC libz.a)
endif(ZLIB_STATIC_LIBRARY)

# additional hints
if(MINGW)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} C:/Mingw)
endif(MINGW)

if(MSVC)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} D:/devlib/vc/zlib)
endif(MSVC)

# Include dir
find_path(ZLIB_INCLUDE_DIR
  NAMES zlib.h
  PATHS ${ZLIB_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(ZLIB_LIBRARY
  NAMES ${ZLIB_STATIC} z zlib
  PATHS ${ZLIB_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(ZLIB_PROCESS_INCLUDES ZLIB_INCLUDE_DIR)
set(ZLIB_PROCESS_LIBS ZLIB_LIBRARY)
libfind_process(ZLIB)