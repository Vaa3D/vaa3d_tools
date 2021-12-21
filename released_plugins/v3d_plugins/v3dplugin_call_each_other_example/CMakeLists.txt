
set(PLUGIN_DIRECTORY_NAME V3D_PluginInterface_Demos/call_each_other )

set(PLUGIN_SOURCES ../v3dplugin_matrix_example/ex_matrix.cpp )
set(PLUGIN_HEADERS ../v3dplugin_matrix_example/ex_matrix.h )

configure_v3d_plugin(ex_matrix)

set(PLUGIN_SOURCES ex_call.cpp )
set(PLUGIN_HEADERS ex_call.h )

configure_v3d_plugin(ex_call)

