
set(PLUGIN_HEADERS swc_to_maskimage.h )
set(PLUGIN_SOURCES 
    swc_to_maskimage.cpp
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
    ../../../v3d_main/basic_c_fun/v3d_message.cpp
)
set(PLUGIN_DIRECTORY_NAME neuron_utilities/swc_to_maskimage)

configure_v3d_plugin(swc_to_maskimage)

