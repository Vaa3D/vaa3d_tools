# CmakeLists.txt in CrossMIPs dir

# Adds a library called crossmips (crossmips.a under Linux, crossmips.lib under Windows) from the all .h and .cpp files
file(GLOB crossmips_headers *.h)
file(GLOB crossmips_sources *.cpp)
add_library(crossmips STATIC ${crossmips_headers} ${crossmips_sources})
set_target_properties(crossmips PROPERTIES POSITION_INDEPENDENT_CODE ON)
