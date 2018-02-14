# CMakeLists.txt in UI dir

# add used modules to include path
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/crossmips)
include_directories (${TeraStitcher_SOURCE_DIR}/stitcher)
include_directories (${TeraStitcher_SOURCE_DIR}/common)
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)

# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)

# add executable from the all .h and .cpp files
file(GLOB UI_headers *.h)
file(GLOB UI_sources *.cpp)
add_executable(terastitcher ${UI_headers} ${UI_sources})

# add executable to install target
install(TARGETS terastitcher
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

# link the executable to the other modules 
target_link_libraries(terastitcher stitcher )
target_link_libraries(terastitcher volumemanager )
target_link_libraries(terastitcher crossmips )
target_link_libraries(terastitcher iomanager )
target_link_libraries(terastitcher tinyxml )
target_link_libraries(terastitcher common )