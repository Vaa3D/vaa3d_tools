
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3DPATH =  /Users/jazz/Desktop/v3d_new/v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
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
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(plugin_creator)
DESTDIR = /Users/jazz/Desktop/vaa3dapp/plugins/SuperPlugin_Jazz
