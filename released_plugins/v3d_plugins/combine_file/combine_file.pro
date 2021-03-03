
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= combine_file_plugin.h
HEADERS      += ../sort_neuron_swc/openSWCDialog.h

SOURCES	+= combine_file_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += ../sort_neuron_swc/openSWCDialog.cpp



TARGET	= $$qtLibraryTarget(combine_file)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/misc/combine_file/
