set(PLUGIN_SOURCES refextract.cpp )
set(PLUGIN_HEADERS refextract.h )
set(PLUGIN_DIRECTORY_NAME refExtract)
include_directories(${TIFF_INCLUDE_DIR})
configure_v3d_plugin(refExtract)

