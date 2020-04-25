
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Center_Tile_Boundary_plugin.h
SOURCES	+= Center_Tile_Boundary_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Center_Tile_Boundary)
DESTDIR	= $$VAA3DPATH/bin/plugins/Center_Tile_Boundary/
