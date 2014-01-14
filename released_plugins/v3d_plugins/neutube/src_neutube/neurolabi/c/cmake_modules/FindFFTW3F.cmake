# - Try to find FFTW3F
# Once done, this will define
#
#  FFTW3F_FOUND - system has FFTW3F
#  FFTW3F_INCLUDE_DIRS - the FFTW3F include directories
#  FFTW3F_LIBRARIES - link these to use FFTW3F
#
# By default, the dynamic libraries will be found. To find the static ones instead,
# you must set the FFTW3F_STATIC_LIBRARY variable to TRUE before calling find_package.
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(FFTW3F_PKGCONF fftw3f)

# attempt to find static library first if this is set
if(FFTW3F_STATIC_LIBRARY)
  set(FFTW3F_STATIC libfftw3f.a)
endif(FFTW3F_STATIC_LIBRARY)

# additional hints
if(MINGW)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} C:/Mingw)
endif(MINGW)

if(MSVC)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} D:/devlib/vc/fftw)
endif(MSVC)

# Include dir
find_path(FFTW3F_INCLUDE_DIR
  NAMES fftw3.h
  PATHS ${FFTW3F_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(FFTW3F_LIBRARY
  NAMES ${FFTW3F_STATIC} fftw3f libfftw3f-3.lib
  PATHS ${FFTW3F_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(FFTW3F_PROCESS_INCLUDES FFTW3F_INCLUDE_DIR)
set(FFTW3F_PROCESS_LIBS FFTW3F_LIBRARY)
libfind_process(FFTW3F)