
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3DPATH = ../../../v3d_external
VAA3DMAINPATH = $$VAA3DPATH/v3d_main
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun

HEADERS	= plugin_creator_plugin.h
HEADERS	+= plugin_creator_func.h
HEADERS	+= plugin_creator_gui.h
HEADERS	+= create_plugin.h

SOURCES	= plugin_creator_plugin.cpp
SOURCES	+= plugin_creator_func.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(plugin_creator)
DESTDIR = $$VAA3DPATH/v3d/plugins/plugin_creater
#DESTDIR	= ~/Applications/v3d/plugins/plugin_creator/
