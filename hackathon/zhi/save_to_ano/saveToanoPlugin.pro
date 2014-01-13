
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	+= saveToanoPlugin_plugin.h
SOURCES	+= saveToanoPlugin_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(saveToanoPlugin)
DESTDIR	= ../../../bin/plugins/linker_file/saveToano/
