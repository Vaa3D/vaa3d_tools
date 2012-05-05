
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= hier_label_plugin.h
HEADERS	+= hier_label_func.h

SOURCES	= hier_label_plugin.cpp
SOURCES	+= hier_label_func.cpp
SOURCES	+= hierachical_labeling.cpp
SOURCES	+= eswc_core.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(hier_label)
DESTDIR	= ../../v3d/plugins/neuron_utilities/hierarchical_labeling_of_neuron
