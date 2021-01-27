
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= neuron_toolbox_plugin.h
HEADERS	+= neuron_toolbox_func.h
HEADERS	+= toolbox_gui.h

SOURCES	= neuron_toolbox_plugin.cpp
SOURCES	+= neuron_toolbox_func.cpp
SOURCES	+= toolbox_gui.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_toolbox)
DESTDIR        = $$V3DMAINPATH/../bin/plugins/neuron_toolbox/

