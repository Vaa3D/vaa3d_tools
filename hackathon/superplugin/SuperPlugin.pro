
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/zhouj/work/vaa3d
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= SuperPlugin_plugin.h
HEADERS	+= SuperPlugin_func.h

SOURCES	= SuperPlugin_plugin.cpp
SOURCES	+= SuperPlugin_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(superplugin)
DESTDIR	= $$VAA3DPATH/bin/plugins/SuperPlugin/
