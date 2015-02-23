# CmakeLists.txt in Stitcher dir

# Make sure the compiler can find include files of other modules. 
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/zlib)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/zlib)
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/libtiff)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/libtiff) 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/tinyxml)
include_directories (${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/crossmips)

#disabling annoying warnings
warnings_disable(CMAKE_CXX_FLAGS /wd4290) # vs2012: can't properly handle exceptions
warnings_disable(CMAKE_CXX_FLAGS /wd4996) # vs2012: complains about unsafe standard C++ functions


# Adds a library called stitcher (stitcher.a under Linux, stitcher.lib under Windows) from the all .h and .cpp files
file(GLOB stitcher_headers *.h)
file(GLOB stitcher_sources *.cpp)
add_library(stitcher STATIC ${stitcher_headers} ${stitcher_sources})