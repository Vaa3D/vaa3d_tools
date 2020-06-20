
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/neuron_editing


HEADERS	+= neuron_dist_plugin.h \ \
    basic_fun/basic_surf_objs.h \
    basic_fun/v3d_message.h \
    neuron_edit/global_feature_compute.h \
    neuron_edit/neuron_format_converter.h \
    neuron_edit/neuron_sim_scores.h \
    neuron_edit/v_neuronswc.h \
    swc_to_maskimage/filter_dialog.h

HEADERS	+= neuron_dist_func.h
HEADERS	+= neuron_dist_gui.h



SOURCES	= neuron_dist_plugin.cpp \
    basic_fun/basic_surf_objs.cpp \
    basic_fun/v3d_message.cpp \
    neuron_edit/global_feature_compute.cpp \
    neuron_edit/neuron_format_converter.cpp \
    neuron_edit/neuron_sim_scores.cpp \
    neuron_edit/v_neuronswc.cpp \
    swc_to_maskimage/filter_dialog.cpp
SOURCES	+= neuron_dist_func.cpp
SOURCES	+= neuron_dist_gui.cpp


TARGET	= $$qtLibraryTarget(neuron_dist)
#DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/neuron_distance
