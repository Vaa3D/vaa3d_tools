
set(PLUGIN_SOURCES recenterimageplugin.cpp )
set(PLUGIN_HEADERS recenterimageplugin.h )
set(PLUGIN_DIRECTORY_NAME image_geometry/ReCenter_Image)

include_directories(${TIFF_INCLUDE_DIR})

configure_v3d_plugin(ReCenter_Image)

