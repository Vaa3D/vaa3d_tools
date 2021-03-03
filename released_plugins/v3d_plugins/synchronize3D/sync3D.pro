
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

win32{
}


INCLUDEPATH += main 


HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h

HEADERS	+= sync3D_plugin.h
SOURCES	+= sync3D_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET	= $$qtLibraryTarget(sync3D)
DESTDIR = $$V3DMAINPATH/../bin/plugins/Sync_Views/sync3D/

