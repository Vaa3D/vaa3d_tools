# CmakeLists.txt in TinyXML dir

# Adds a library called tinyxml (tinyxml.a under Linux, tinyxml.lib under Windows) from the all .h and .cpp files
file(GLOB tinyxml_headers *.h)
file(GLOB tinyxml_sources *.cpp)
add_library(tinyxml STATIC ${tinyxml_headers} ${tinyxml_sources})