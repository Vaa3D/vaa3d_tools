
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
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(save3dviewer2linker)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/linker_file/Save_3Dviewer_Content_to_Linker_File/
