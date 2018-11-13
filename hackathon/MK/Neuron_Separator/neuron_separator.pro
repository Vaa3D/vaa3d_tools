
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_CXXFLAGS += -std=c++11
#CONFIG	+= x86_64

VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += ../NeuronStructNavigator

unix:!macx {
	LIBS += -L$$VAA3DPATH/v3d_man/common_lib/lib -lNeuronStructNavigator
	#LIBS += -L/usr/lib64/ -lboost_system
}

HEADERS	+= neuron_separator_plugin.h
HEADERS += NeuronStructExplorer.h
HEADERS += NeuronStructUtilities.h
SOURCES	+= neuron_separator_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$INCLUDEPATH/basic_surf_objs.cpp
SOURCES += neuronSeparation_func.cpp
SOURCES += NeuronStructExplorer.cpp
SOURCES += NeuronStructUtilities.cpp

TARGET	= $$qtLibraryTarget(neuron_separator)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_separator/
