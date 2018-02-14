# CmakeLists.txt in mergedisplacements dir

# make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/common)

# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)

# add executable called mergedisplacements (mergedisplacements under Linux, mergedisplacements.exe under Windows) from the all .h and .cpp files
file(GLOB mergedisplacements_headers *.h)
file(GLOB mergedisplacements_sources *.cpp)
add_executable(mergedisplacements ${mergedisplacements_headers} ${mergedisplacements_sources})

# add executable to install target
install (TARGETS mergedisplacements RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# link the executable to the other modules. 
target_link_libraries(mergedisplacements iomanager )
target_link_libraries(mergedisplacements tinyxml )
target_link_libraries(mergedisplacements volumemanager )
target_link_libraries(mergedisplacements tinyxml )
target_link_libraries(mergedisplacements stitcher )
target_link_libraries(mergedisplacements crossmips )
target_link_libraries(mergedisplacements common )