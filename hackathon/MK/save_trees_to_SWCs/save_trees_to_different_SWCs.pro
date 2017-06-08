
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Vaa3D_2010_Qt473/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= save_trees_to_different_SWCs_plugin.h
SOURCES	+= save_trees_to_different_SWCs_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(save_trees_to_different_SWCs)
DESTDIR	= $$VAA3DPATH/bin/plugins/save_trees_to_different_SWCs/
