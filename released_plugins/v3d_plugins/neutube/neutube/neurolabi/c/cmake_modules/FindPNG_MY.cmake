# - Try to find PNG
# Once done, this will define
#
#  PNG_FOUND - system has PNG
#  PNG_INCLUDE_DIRS - the PNG include directories
#  PNG_LIBRARIES - link these to use PNG
#
# By default, the dynamic libraries will be found. To find the static ones instead,
# you must set the PNG_STATIC_LIBRARY variable to TRUE before calling find_package.
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(PNG_PKGCONF libpng)

# attempt to find static library first if this is set
if(PNG_STATIC_LIBRARY)
  set(PNG_STATIC libpng.a)
endif(PNG_STATIC_LIBRARY)

# additional hints
if(MINGW)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} C:/Mingw)
endif(MINGW)

if(MSVC)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} D:/devlib/vc/libpng)
endif(MSVC)

# Include dir
find_path(PNG_INCLUDE_DIR
  NAMES png.h
  PATHS ${PNG_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(PNG_LIBRARY
  NAMES ${PNG_STATIC} png libpng15
  PATHS ${PNG_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(PNG_PROCESS_INCLUDES PNG_INCLUDE_DIR)
set(PNG_PROCESS_LIBS PNG_LIBRARY)
libfind_process(PNG)