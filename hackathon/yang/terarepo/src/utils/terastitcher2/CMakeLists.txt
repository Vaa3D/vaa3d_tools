# CmakeLists.txt in GUI dir

# Make sure the compiler can find include files of other modules. 
include_directories(${TeraStitcher_SOURCE_DIR}/CrossMIPs)
include_directories(${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories(${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories(${TeraStitcher_SOURCE_DIR}/multilayersmanager)
include_directories(${TeraStitcher_SOURCE_DIR}/stitcher)
include_directories(${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories(${TeraStitcher_SOURCE_DIR}/common)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
#Onofri
#include_directories(${TeraImagesTools_SOURCE_DIR}/../include/libtiff)
#link_directories(${TeraImagesTools_SOURCE_DIR}/../lib/libtiff)

# Adds executable called terastitcher2 (terastitcher2.a under Linux, terastitcher2.exe under Windows) from the all .h and .cpp files
file(GLOB GUI_headers *.h)
file(GLOB GUI_sources *.cpp)
add_executable(terastitcher2 ${GUI_headers} ${GUI_sources})

# Link the executable to the other modules. 
target_link_libraries(terastitcher2 multilayersmanager )
#target_link_libraries(terastitcher2 stitcher )
target_link_libraries(terastitcher2 volumemanager )
#target_link_libraries(terastitcher2 crossmips )
target_link_libraries(terastitcher2 iomanager )
target_link_libraries(terastitcher2 tinyxml )
target_link_libraries(terastitcher2 common )