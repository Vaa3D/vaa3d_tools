
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d
INCLUDEPATH	+= $$V3DMAINPATH/neuron_editing


HEADERS	+= neuron_dist_plugin.h \
    ../../../v3d_main/neuron_editing/neuron_format_converter.h
HEADERS	+= neuron_dist_func.h
HEADERS	+= neuron_dist_gui.h
HEADERS += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h


SOURCES	= neuron_dist_plugin.cpp \
    ../../../v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES	+= neuron_dist_func.cpp
SOURCES	+= neuron_dist_gui.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp
SOURCES += $$V3DMAINPATH/neuron_editing/global_feature_compute.cpp

TARGET	= $$qtLibraryTarget(neuron_dist)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/neuron_distance
