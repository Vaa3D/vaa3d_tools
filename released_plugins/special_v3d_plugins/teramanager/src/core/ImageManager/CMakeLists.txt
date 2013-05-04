# CmakeLists.txt in ImageManager2 dir

# Adds a library called ImageManager2 (ImageManager2.a under Linux, ImageManager2.lib under Windows) from the all .h and .cpp files
file(GLOB ImageManager2_headers *.h)
file(GLOB ImageManager2_sources *.cpp)
add_library(ImageManager2 STATIC ${ImageManager2_headers} ${ImageManager2_sources})
