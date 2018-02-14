# CmakeLists.txt in volumeconverter dir

option (WITH_RESUME_STATUS "Save status during image generation" OFF) 
if(WITH_RESUME_STATUS)
	add_definitions(-DSAVE_RESUMER_STATUS)
endif()

# make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/common)


# add 3rd party modules to include path

# add a library called volumeconverter (volumeconverter.a under Linux, volumeconverter.lib under Windows) from the all .h and .cpp files
file(GLOB volumeconverter_headers *.h)
file(GLOB volumeconverter_sources *.cpp)
add_library(volumeconverter STATIC ${volumeconverter_headers} ${volumeconverter_sources})
