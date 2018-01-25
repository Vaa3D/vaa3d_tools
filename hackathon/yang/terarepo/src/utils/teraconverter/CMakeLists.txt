# CmakeLists.txt in teraconverter dir

# make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/utils/volumeconverter)
include_directories (${TeraStitcher_SOURCE_DIR}/common)

# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)


# add executable called teraconverter (teraconverter under Linux, teraconverter.exe under Windows) from the all .h and .cpp files
file(GLOB teraconverter_headers *.h)
file(GLOB teraconverter_sources *.cpp)
add_executable(teraconverter ${teraconverter_headers} ${teraconverter_sources})

# add executable to install target
install (TARGETS teraconverter RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# link the executable to the other modules. 
target_link_libraries(teraconverter volumeconverter )
target_link_libraries(teraconverter imagemanager )
target_link_libraries(teraconverter iomanager )
target_link_libraries(teraconverter volumemanager )
target_link_libraries(teraconverter tinyxml )
target_link_libraries(teraconverter stitcher )
target_link_libraries(teraconverter crossmips )
target_link_libraries(teraconverter common )

if(UNIX AND NOT APPLE)
    target_link_libraries(teraconverter rt)
endif()