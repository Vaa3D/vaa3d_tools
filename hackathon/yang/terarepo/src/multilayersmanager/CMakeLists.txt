# CmakeLists.txt in multilayersmanager dir

# Make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/crossmips)
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/stitcher)
include_directories (${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)
include_directories (${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/common)

# Adds a library called imageanager (multiLayersmanager.a under Linux, multiLayersmanager.lib under Windows) from the all .h and .cpp files
file(GLOB multiLayersmanager_headers *.h)
file(GLOB multiLayersmanager_sources *.cpp)
add_library(multilayersmanager STATIC ${multiLayersmanager_headers} ${multiLayersmanager_sources})
