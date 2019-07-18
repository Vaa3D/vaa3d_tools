
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_CXXFLAGS += -std=c++11
#CONFIG	+= x86_64

VAA3DPATH = ../../../../v3d_external
V3DTOOLPATH = ../../../released_plugins/v3d_plugins
NEURONLIBPATH = ../NeuronStructNavigator
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$NEURONLIBPATH
INCLUDEPATH += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH += ../v3d_imgManagerMK
INCLUDEPATH += ../v3d_imgManagerMK/imgProcessor/
INCLUDEPATH += ../v3d_imgManagerMK/imgAnalyzer/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include/
INCLUDEPATH += $$V3DTOOLPATH/swc2mask_cylinder

unix:!macx {
        LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -lNeuronStructNavigator
        LIBS += -L/usr/lib64/ -lboost_system
}

HEADERS	+= neuron_separator_plugin.h
HEADERS += $$NEURONLIBPATH/NeuronStructExplorer.h
HEADERS += $$NEURONLIBPATH/NeuronStructUtilities.h
HEADERS += ../v3d_imgManagerMK/ImgManager.h
HEADERS += ../v3d_imgManagerMK/imgAnalyzer/ImgAnalyzer.h
HEADERS += $$V3DTOOLPATH/swc2mask_cylinder/my_surf_objs.h

SOURCES	+= neuron_separator_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += neuronSeparation_func.cpp
SOURCES += $$NEURONLIBPATH/NeuronStructExplorer.cpp
SOURCES += $$NEURONLIBPATH/NeuronStructUtilities.cpp

TARGET	= $$qtLibraryTarget(neuron_separator)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_separator/
