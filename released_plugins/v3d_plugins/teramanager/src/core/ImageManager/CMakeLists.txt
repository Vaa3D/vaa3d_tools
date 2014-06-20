# CmakeLists.txt in ImageManager dir

include_directories (${TeraImagesTools_SOURCE_DIR}/../include/libtiff)

# Adds a library called ImageManager (ImageManager.a under Linux, ImageManager.lib under Windows) from the all .h and .cpp files
file(GLOB ImageManager_headers *.h)
file(GLOB ImageManager_sources *.cpp)
add_library(ImageManager STATIC ${ImageManager_headers} ${ImageManager_sources})
