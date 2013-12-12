# - Try to find FFTW3
# Once done, this will define
#
#  FFTW3_FOUND - system has FFTW3
#  FFTW3_INCLUDE_DIRS - the FFTW3 include directories
#  FFTW3_LIBRARIES - link these to use FFTW3
#
# By default, the dynamic libraries will be found. To find the static ones instead,
# you must set the FFTW3_STATIC_LIBRARY variable to TRUE before calling find_package.
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(FFTW3_PKGCONF fftw3)

# attempt to find static library first if this is set
if(FFTW3_STATIC_LIBRARY)
  set(FFTW3_STATIC libfftw3.a)
endif(FFTW3_STATIC_LIBRARY)

# additional hints
if(MINGW)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} C:/Mingw)
endif(MINGW)

if(MSVC)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} D:/devlib/vc/fftw)
endif(MSVC)

# Include dir
find_path(FFTW3_INCLUDE_DIR
  NAMES fftw3.h
  PATHS ${FFTW3_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(FFTW3_LIBRARY
  NAMES ${FFTW3_STATIC} fftw3 libfftw3-3.lib
  PATHS ${FFTW3_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(FFTW3_PROCESS_INCLUDES FFTW3_INCLUDE_DIR)
set(FFTW3_PROCESS_LIBS FFTW3_LIBRARY)
libfind_process(FFTW3)