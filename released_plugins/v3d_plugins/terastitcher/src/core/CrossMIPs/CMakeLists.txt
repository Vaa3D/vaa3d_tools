# CmakeLists.txt in CrossMIPs dir

# Make sure the compiler can find include files of other modules. 
include_directories (${TeraStitcher_SOURCE_DIR}/IOManager)

# Adds a library called crossmips (crossmips.a under Linux, crossmips.lib under Windows) from the all .h and .cpp files
file(GLOB crossmips_headers *.h)
file(GLOB crossmips_sources *.cpp)
add_library(crossmips STATIC ${crossmips_headers} ${crossmips_sources})