
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/neuron_editing

HEADERS	+= neuron_dist_plugin.h
HEADERS	+= neuron_dist_func.h
HEADERS	+= neuron_dist_gui.h

SOURCES	= neuron_dist_plugin.cpp
SOURCES	+= neuron_dist_func.cpp
SOURCES	+= neuron_dist_gui.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/v_neuronswc.cpp

TARGET	= $$qtLibraryTarget(neuron_dist)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/neuron_distance
