# CmakeLists.txt in imagemanager dir

# add used modules to include path 
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/tinyxml)

# add 3rd party modules to include path
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/zlib)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/zlib)
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/libtiff)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/libtiff)

# set up OpenCV library
#find_package(OpenCV REQUIRED)
#include_directories(${OpenCV_INCLUDE_DIRS})
#link_directories(${OpenCV_LIB_DIR})

# add STATIC library from the all .h and .cpp files
file(GLOB imagemanager_headers *.h)
file(GLOB imagemanager_sources *.cpp)
add_library(imagemanager STATIC ${imagemanager_headers} ${imagemanager_sources})
