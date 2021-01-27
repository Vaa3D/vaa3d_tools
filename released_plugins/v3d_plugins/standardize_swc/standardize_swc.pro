
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

HEADERS	+= standardize_swc_plugin.h
HEADERS	+= $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/resample_swc/resampling.h
HEADERS	+= $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h



SOURCES	+= standardize_swc_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(standardize__swc)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/standardize_swc/
