
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	= neuronassembler_plugin_creator_plugin.h
HEADERS	+= plugin_creator_func.h
HEADERS	+= plugin_creator_gui.h


SOURCES	= neuronassembler_plugin_creator_plugin.cpp
SOURCES	+= plugin_creator_func.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(plugin_creator)
DESTDIR = $$VAA3DPATH/../bin/plugins/_Vaa3D_neuronassembler_plugin_creator
