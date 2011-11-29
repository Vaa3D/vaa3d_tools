cmake_minimum_required(VERSION 2.4)
if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

project(v3d_released_plugins)


find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

include(CTest)

# include(CPack) # save cpack for parent directory

find_library(FFTW_LIBRARY
    NAMES libfftw3f.a v3dfftw3f libfftw3f.a fftw3f libfftw3f-3
    PATHS "$ENV{PROGRAMFILES}/fftw")
find_library(FFTW_THREADS_LIBRARY
    NAMES libfftw3f_threads.a v3dfftw3f_threads libfftw3f_threads.a fftw3f_threads )
if(NOT FFTW_LIBRARY)
    message("istitch plugin will not be built unless fftw library is found")
endif()

if(FFTW_LIBRARY)
    find_path(FFTW_INCLUDE_DIR
        NAME "fftw3.h"
        PATHS
            "${V3DMAINDIR}/common_lib/include"
            /usr/local/include
    )
    include_directories("${FFTW_INCLUDE_DIR}")
endif()

find_path(V3D_BINARY_DIR v3d)
set(INSTALLATION_DIRECTORY ${V3D_BINARY_DIR}/plugins)

find_path(V3D_BASIC_C_FUN_SOURCE_DIR v3d_interface.h)
if( V3D_BASIC_C_FUN_SOURCE_DIR )
  set( V3D_BASIC_C_FUN_BINARY_DIR  ${PROJECT_BINARY_DIR}/Source/basic_c_fun )
endif( V3D_BASIC_C_FUN_SOURCE_DIR )

if(NOT V3D_BINARY_DIR)
  message(FATAL "The V3D_BINARY_DIR is required. This must point to the location of the v3d executable")
endif()


set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE INTERNAL "Output directory for the shared libraries.")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE INTERNAL "Output directory for the executables.")


include_directories( ${V3D_BASIC_C_FUN_SOURCE_DIR} )

# CMB Nov-11-2010 Need to wait until all plugins are built before running macdeployqt
add_custom_target(FinishedPlugins
    COMMENT "All plugins have been built")

include(${PROJECT_SOURCE_DIR}/CMake/PluginConfiguration.cmake)

add_subdirectory( v3d_plugins )
add_subdirectory( special_v3d_plugins )

