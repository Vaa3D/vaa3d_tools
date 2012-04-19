
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	+= neuron_toolbox_plugin.h
HEADERS	+= neuron_toolbox_func.h
HEADERS	+= toolbox_gui.h

SOURCES	= neuron_toolbox_plugin.cpp
SOURCES	+= neuron_toolbox_func.cpp
SOURCES	+= toolbox_gui.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_toolbox)
DESTDIR        = ../../v3d/plugins/neuron_toolbox/

