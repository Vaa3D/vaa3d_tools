
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#QMAKE_CXXFLAGS += -std=c++11

#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
VAA3DTOOLSPATH = ../../../../vaa3d_tools
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/
INCLUDEPATH += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc/
INCLUDEPATH += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/
#INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/NeuronStructNavigator/
#INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/v3d_imgManagerMK/
#INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/v3d_imgManagerMK/imgProcessor/
#INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/v3d_imgManagerMK/imgAnalyzer/
#INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/MK/v3d_imgManagerMK/imgManager/
INCLUDEPATH += $$VAA3DTOOLSPATH/hackathon/PengXie/neuron_reliability_score/src/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include/

#unix:!macx {
#        LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -lNeuronStructNavigator
#        LIBS += -L/usr/lib64/ -lboost_system
#}

HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/v3d_interface.h \
    $$VAA3DTOOLSPATH/hackathon/PengXie/neuron_reliability_score/src/my_surf_objs.h \
    get_terminal.h \
    utilities.h \
    get_terminal_signal.h
HEADERS +=  $$VAA3DPATH/v3d_main/basic_c_fun/v3d_interface.h

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    get_terminal.cpp \
    utilities.cpp \
    get_terminal_signal.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

HEADERS	+= tip_signal_plugin.h
SOURCES	+= tip_signal_plugin.cpp
TARGET	= $$qtLibraryTarget(tip_signal)
DESTDIR	= $$VAA3DPATH/bin/plugins/tip_signal/
