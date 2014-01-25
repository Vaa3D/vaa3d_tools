
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
#INCLUDEPATH	+= $$V3DMAINPATH/3drenderer

HEADERS	+= saveToanoPlugin_plugin.h
SOURCES	+= saveToanoPlugin_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(save3dviewer2linker)
DESTDIR	= ../../v3d/plugins/linker_file/Save_3Dviewer_Content_to_Linker_File/
