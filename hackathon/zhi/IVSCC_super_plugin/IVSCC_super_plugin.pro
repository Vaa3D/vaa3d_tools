
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= IVSCC_super_plugin_plugin.h
HEADERS	+= $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/resample_swc/resampling.h
HEADERS	+= sort_swc_fiji.h

SOURCES	+= IVSCC_super_plugin_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/global_neuron_feature/compute.cpp




TARGET	= $$qtLibraryTarget(IVSCC_super_plugin)
DESTDIR	= $$VAA3DPATH/../bin/plugins/IVSCC_super_plugin/
