# ----------------------------------------------------------------------------
#  CMake file for libtiff. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------
project( tiff )
cmake_minimum_required(VERSION 2.7)

include(CheckFunctionExists)
include(CheckIncludeFile)

check_include_file(assert.h HAVE_ASSERT_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(io.h HAVE_IO_H)
check_function_exists(jbg_newlen HAVE_JBG_NEWLEN)
check_function_exists(mmap HAVE_MMAP)
check_include_file(search.h HAVE_SEARCH_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)

if(WIN32)
  set(USE_WIN32_FILEIO 1)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tif_config.h.cmakein"
               "${CMAKE_CURRENT_BINARY_DIR}/tif_config.h" @ONLY)
			   
# Make sure the compiler can find include files of other modules. 
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/zlib)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/zlib)
#ocv_include_directories("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" ${ZLIB_INCLUDE_DIR})

set(lib_srcs
    tif_aux.c
    tif_close.c
    tif_codec.c
    tif_color.c
    tif_compress.c
    tif_dir.c
    tif_dirinfo.c
    tif_dirread.c
    tif_dirwrite.c
    tif_dumpmode.c
    tif_error.c
    tif_extension.c
    tif_fax3.c
    tif_fax3sm.c
    tif_flush.c
    tif_getimage.c
    tif_jbig.c
    tif_jpeg_12.c
    tif_jpeg.c
    tif_luv.c
    tif_lzma.c
    tif_lzw.c
    tif_next.c
    tif_ojpeg.c
    tif_open.c
    tif_packbits.c
    tif_pixarlog.c
    tif_predict.c
    tif_print.c
    tif_read.c
    tif_strip.c
    tif_swab.c
    tif_thunder.c
    tif_tile.c
    tif_version.c
    tif_warning.c
    tif_write.c
    tif_zip.c
    tif_stream.cxx
    t4.h
    tif_dir.h
    tif_fax3.h
    tiff.h
    tiffio.h
    tiffiop.h
    tiffvers.h
    tif_predict.h
    uvcode.h
    tiffio.hxx
    "${CMAKE_CURRENT_BINARY_DIR}/tif_config.h"
    )

if(UNIX)
  list(APPEND lib_srcs tif_unix.c)
endif()

if(WIN32)
  list(APPEND lib_srcs tif_win32.c)
endif(WIN32)

#disabling annoying warnings
warnings_disable(CMAKE_CXX_FLAGS /wd4290) # vs2012: can't properly handle exceptions
warnings_disable(CMAKE_CXX_FLAGS /wd4996) # vs2012: complains about unsafe standard C++ functions

#ocv_warnings_disable(CMAKE_C_FLAGS -Wno-unused-but-set-variable -Wmissing-prototypes -Wmissing-declarations -Wundef -Wunused -Wsign-compare
#                                   -Wcast-align -Wshadow -Wno-maybe-uninitialized -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast)
#ocv_warnings_disable(CMAKE_C_FLAGS -Wunused-parameter) # clang
#ocv_warnings_disable(CMAKE_CXX_FLAGS -Wmissing-declarations -Wunused-parameter)
#ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4018 /wd4100 /wd4127 /wd4311 /wd4701 /wd4706) # vs2005
#ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4244) # vs2008
#ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4267 /wd4305 /wd4306) # vs2008 Win64
#ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4703) # vs2012

#ocv_warnings_disable(CMAKE_C_FLAGS /wd4267 /wd4244 /wd4018)

if(UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR CV_ICC))
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

add_library(tiff STATIC ${lib_srcs})
target_link_libraries(tiff zlib)

# set_target_properties(libtiff
    # PROPERTIES
    # OUTPUT_NAME "libtiff"
	# ARCHIVE_OUTPUT_DIRECTORY ${3P_LIBRARY_OUTPUT_PATH}
    # #DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
    # #ARCHIVE_OUTPUT_DIRECTORY ${3P_LIBRARY_OUTPUT_PATH}
    # )

# if(ENABLE_SOLUTION_FOLDERS)
  # set_target_properties(libtiff PROPERTIES FOLDER "3rdparty")
# endif()

# if(NOT BUILD_SHARED_LIBS)
  # install(TARGETS libtiff ARCHIVE DESTINATION ${3P_LIBRARY_OUTPUT_PATH} COMPONENT main)
# endif()
