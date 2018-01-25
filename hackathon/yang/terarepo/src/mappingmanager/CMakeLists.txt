# CmakeLists.txt in mappingmanager dir

# make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
#include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
#include_directories (${TeraStitcher_SOURCE_DIR}/common)


# add 3rd party modules to include path

# add a library called mappingmanager (mappingmanager.a under Linux, mappingmanager.lib under Windows) from the all .h and .cpp files
file(GLOB mappingmanager_headers *.h)
file(GLOB mappingmanager_sources *.cpp)
add_library(mappingmanager STATIC ${mappingmanager_headers} ${mappingmanager_sources})
