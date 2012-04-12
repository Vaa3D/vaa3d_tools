
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/wany/Work/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= neuron_toolbox_plugin.h
HEADERS	+= neuron_toolbox_func.h
HEADERS	+= toolbox_gui.h

SOURCES	= neuron_toolbox_plugin.cpp
SOURCES	+= neuron_toolbox_func.cpp
SOURCES	+= toolbox_gui.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_toolbox)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_toolbox/
