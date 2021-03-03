
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= hier_label_plugin.h
HEADERS	+= hier_label_func.h

SOURCES	= hier_label_plugin.cpp
SOURCES	+= hier_label_func.cpp
SOURCES	+= hierachical_labeling.cpp
SOURCES	+= eswc_core.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(hier_label)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/hierarchical_labeling_of_neuron
