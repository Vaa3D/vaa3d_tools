
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

HEADERS	+= inter_node_pruning_plugin.h
HEADERS      += ../sort_neuron_swc/openSWCDialog.h

SOURCES	+= inter_node_pruning_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= my_surf_objs.cpp
SOURCES      += ../sort_neuron_swc/openSWCDialog.cpp


TARGET	= $$qtLibraryTarget(inter_node_pruning)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/inter_node_pruning/
