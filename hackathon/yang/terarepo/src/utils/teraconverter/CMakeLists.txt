# CmakeLists.txt in teraconverter dir

# make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/utils/volumeconverter)
include_directories (${TeraStitcher_SOURCE_DIR}/common)

# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)


# add executable called biconverter (biconverter under Linux, biconverter.exe under Windows) from the all .h and .cpp files
file(GLOB biconverter_headers *.h)
file(GLOB biconverter_sources *.cpp)
add_executable(biconverter ${biconverter_headers} ${biconverter_sources})

# add executable to install target
install (TARGETS biconverter RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# link the executable to the other modules. 
target_link_libraries(biconverter volumeconverter )
target_link_libraries(biconverter imagemanager )
target_link_libraries(biconverter iomanager )
target_link_libraries(biconverter volumemanager )
target_link_libraries(biconverter tinyxml )
target_link_libraries(biconverter stitcher )
target_link_libraries(biconverter crossmips )
target_link_libraries(biconverter common )

if(UNIX AND NOT APPLE)
    target_link_libraries(biconverter rt)
endif()
