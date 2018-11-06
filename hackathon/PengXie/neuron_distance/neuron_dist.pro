
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_CXXFLAGS += -std=c++11

#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
VAA3DTOOLSPATH = ../../../../vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/neuron_editing
INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/PengXie/preprocess/
INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/NeuronStructNavigator/
INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/v3d_imgManagerMK/imgProcessor/
INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/v3d_imgManagerMK/imgAnalyzer/
INCLUDEPATH	+= $$VAA3DPATH/common_lib/include/

unix:!macx {
	LIBS += -L$$VAA3DPATH/common_lib/lib -lNeuronStructNavigator
	LIBS += -L/usr/lib64/ -lboost_system
}

HEADERS	+= neuron_dist_plugin.h
HEADERS	+= neuron_dist_func.h
HEADERS	+= neuron_dist_gui.h
HEADERS += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
HEADERS += $$VAA3DTOOLSPATH/hackathon/PengXie/preprocess/utilities.h

SOURCES	= neuron_dist_plugin.cpp
SOURCES	+= neuron_dist_func.cpp
SOURCES	+= neuron_dist_gui.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(neuron_overlap)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_overlap