# CmakeLists.txt in volumemanager dir

# add used modules to include path 
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/stitcher)

# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)

# add STATIC library from the all .h and .cpp files
file(GLOB volumemanager_headers *.h)
file(GLOB volumemanager_sources *.cpp)
add_library(volumemanager STATIC ${volumemanager_headers} ${volumemanager_sources})
set_target_properties(volumemanager PROPERTIES POSITION_INDEPENDENT_CODE ON)
