# CmakeLists.txt in virtualvolume dir

# make sure the compiler can find include files of other modules.
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/common)

if(WIN32)

#set( EXPORT_FLAGS  "/EXPORT:mexFunction" )
#set( CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} ${EXPORT_FLAGS}" )

endif()

# add a library called virtualvolume (matlablib.so under Linux, matlablib.dll under Windows) from the all .h and .cpp files
#file(GLOB common_headers ${TeraStitcher_SOURCE_DIR}/common/*.h)
#file(GLOB common_sources ${TeraStitcher_SOURCE_DIR}/common/*.cpp)
#file(GLOB crossmips_headers ${TeraStitcher_SOURCE_DIR}/crossmips/*.h)
#file(GLOB crossmips_sources ${TeraStitcher_SOURCE_DIR}/crossmips/*.cpp)
add_library(virtualvolume SHARED $<TARGET_OBJECTS:imagemanager_obj>)

target_link_libraries(virtualvolume iomanager )
target_link_libraries(virtualvolume volumemanager )
target_link_libraries(virtualvolume tinyxml )
target_link_libraries(virtualvolume stitcher )
target_link_libraries(virtualvolume crossmips )
target_link_libraries(virtualvolume common )