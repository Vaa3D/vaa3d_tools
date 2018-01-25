# CmakeLists.txt in subvolextractor dir

# make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/common)

# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)

# add executable called subvolextractor (subvolextractor under Linux, subvolextractor.exe under Windows) from the all .h and .cpp files
file(GLOB subvolextractor_headers *.h)
file(GLOB subvolextractor_sources *.cpp)
add_executable(subvolextractor ${subvolextractor_headers} ${subvolextractor_sources})

# add executable to install target
install (TARGETS subvolextractor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# link the executable to the other modules. 
target_link_libraries(subvolextractor imagemanager )
target_link_libraries(subvolextractor iomanager )
target_link_libraries(subvolextractor volumemanager )
target_link_libraries(subvolextractor tinyxml )
target_link_libraries(subvolextractor stitcher )
target_link_libraries(subvolextractor crossmips )
target_link_libraries(subvolextractor common )