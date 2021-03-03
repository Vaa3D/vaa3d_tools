
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= resample_swc_plugin.h
HEADERS	+= resample_swc_func.h
HEADERS      += ../sort_neuron_swc/openSWCDialog.h


SOURCES	= resample_swc_plugin.cpp
SOURCES	+= resample_swc_func.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += ../sort_neuron_swc/openSWCDialog.cpp


TARGET	= $$qtLibraryTarget(resample_swc)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/resample_swc/
