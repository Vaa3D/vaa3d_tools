
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	= plugin_creator_plugin.h
HEADERS	+= plugin_creator_func.h
HEADERS	+= plugin_creator_gui.h
HEADERS	+= create_plugin.h
HEADERS += common_dialog.h
HEADERS += produce_simplest_plugin.h

SOURCES	= plugin_creator_plugin.cpp
SOURCES	+= plugin_creator_func.cpp
SOURCES += produce_demo1.cpp
SOURCES += produce_demo2.cpp
SOURCES += produce_simplest_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(plugin_creator)
DESTDIR = $$V3DMAINPATH/../bin/plugins/_Vaa3D_plugin_creator
