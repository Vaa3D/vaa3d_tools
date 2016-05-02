    set(PLUGIN_HEADERS ifusion.h) # Q_OBJECT macro containing files only
    set(PLUGIN_DIRECTORY_NAME image_stitching/ifusion)
    set(PLUGIN_SOURCES ifusion.cpp ifusion.h)
    include_directories("${TIFF_INCLUDE_DIR}")
    include_directories("${Boost_INCLUDE_DIR}")
    configure_v3d_plugin(ifusion)
