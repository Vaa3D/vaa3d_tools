# - Try to find LIBJANSSON
# Once done, this will define
#
#  JANSSON_FOUND - system has JANSSON
#  JANSSON_INCLUDE_DIRS - the JANSSON include directories
#  JANSSON_LIBRARIES - link these to use JANSSON
#
# By default, the dynamic libraries will be found. To find the static ones instead,
# you must set the JANSSON_STATIC_LIBRARY variable to TRUE before calling find_package.

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(JANSSON_PKGCONF jansson)

# attempt to find static library first if this is set
if(JANSSON_STATIC_LIBRARY)
  set(JANSSON_STATIC libjansson.a)
endif(JANSSON_STATIC_LIBRARY)

# additional hints
if(MINGW)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} C:/Mingw)
endif(MINGW)

if(MSVC)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} D:/devlib/vc/jansson)
endif(MSVC)

# Include dir
find_path(JANSSON_INCLUDE_DIR
  NAMES jansson.h
  PATHS ${JANSSON_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(JANSSON_LIBRARY
  NAMES ${JANSSON_STATIC} jansson
  PATHS ${JANSSON_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(JANSSON_PROCESS_INCLUDES JANSSON_INCLUDE_DIR)
set(JANSSON_PROCESS_LIBS JANSSON_LIBRARY)
libfind_process(JANSSON)