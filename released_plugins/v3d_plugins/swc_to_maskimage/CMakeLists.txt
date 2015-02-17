
set(PLUGIN_HEADERS swc_to_maskimage.h )
set(PLUGIN_SOURCES 
    swc_to_maskimage.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/basic_surf_objs.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/v3d_message.cpp
)
set(PLUGIN_DIRECTORY_NAME neuron_utilities/swc_to_maskimage)

configure_v3d_plugin(swc_to_maskimage)

