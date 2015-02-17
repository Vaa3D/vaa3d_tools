cmake_minimum_required(VERSION 2.4)
if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

project(v3d_extra_plugins) # those plugins are distributed in seperate repo (vaa3d_tools)  than the truck code


find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

include(CTest)

# include(CPack) # save cpack for parent directory

find_path(V3D_BUILD_DIR   REQUIRED)

find_path(V3D_PLUGIN_LIBRARIES_PATH
  NAMES libV3DInterface.a
  PATHS  "${V3D_BUILD_DIR}/released_plugins/bin" REQUIRED)
find_library(V3DInterface_LIBRARY NAMES V3DInterface  HINTS ${V3D_PLUGIN_LIBRARIES_PATH}  REQUIRED)



find_path(V3D_BINARY_DIR  REQUIRED)
if(NOT V3D_BINARY_DIR)
  message(FATAL "The V3D_BINARY_DIR is required. This must point to the location of the v3d executable")
endif()
set(INSTALLATION_DIRECTORY ${V3D_BINARY_DIR}/plugins)

find_path(V3D_BASIC_C_FUN_SOURCE_DIR v3d_interface.h)
if( V3D_BASIC_C_FUN_SOURCE_DIR )
  set( V3D_BASIC_C_FUN_BINARY_DIR  ${PROJECT_BINARY_DIR}/Source/basic_c_fun )
endif( V3D_BASIC_C_FUN_SOURCE_DIR )

set(V3DMAINDIR  "${V3D_BASIC_C_FUN_SOURCE_DIR}/../")


if(WIN32)
    message("Looking for fftw library")
    # message("$ENV{PROGRAMW6432}/fftw64_3.3.1")
    find_library(FFTW_LIBRARY
        NAMES libfftw3f.a v3dfftw3f libfftw3f.a fftw3f libfftw3f-3
        PATHS
            "$ENV{PROGRAMFILES}/fftw"
            "$ENV{PROGRAMW6432}/fftw64_3.3.1"
            "$ENV{PROGRAMW6432}/fftw-3.3.3"
    )
    find_library(FFTW_THREADS_LIBRARY
        NAMES libfftw3f_threads.a v3dfftw3f_threads libfftw3f_threads.a fftw3f_threads )
    #
    if(FFTW_LIBRARY)
        get_filename_component(FFTW_LIB_DIR "${FFTW_LIBRARY}" PATH)
        find_path(FFTW_INCLUDE_DIR
            NAME "fftw3.h"
            PATHS
                "${V3DMAINDIR}/common_lib/include"
                /usr/local/include
                ${FFTW_LIB_DIR}
        )
        include_directories("${FFTW_INCLUDE_DIR}")
    endif()
else()
    # Build FFTW as an external project
    set(fftw_dir "${CMAKE_CURRENT_BINARY_DIR}/fftw")
    include(ExternalProject)
    # AMD 64 requires -fPIC to link static libraries on linux
    set(fftw_cflags "")
    if(UNIX AND NOT APPLE)
      if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
        set(fftw_cflags "-fPIC ${fftw_cflags}")
      endif()
    endif()
    ExternalProject_Add(
        LibFFTW
        URL "${V3DMAINDIR}/common_lib/src_packages/fftw-3.1.2.tar.gz"
        SOURCE_DIR ${fftw_dir}/src
        BINARY_DIR ${fftw_dir}/build
        INSTALL_DIR ${fftw_dir}/install
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=<INSTALL_DIR>
            --enable-float
            --enable-threads
            --enable-static
            CFLAGS=${fftw_cflags}
    )
    set(FFTW_INCLUDE_DIR "${fftw_dir}/install/include" CACHE PATH "" FORCE)
    include_directories("${FFTW_INCLUDE_DIR}")
    set(lib_suffix "a")
    set(lib_prefix "lib")
    if(WIN32)
      set(lib_suffix "lib")
      set(lib_prefix "")
    endif()
    set(FFTW_LIBRARY "${fftw_dir}/install/lib/${lib_prefix}fftw3f.${lib_suffix}" CACHE PATH "" FORCE)

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

