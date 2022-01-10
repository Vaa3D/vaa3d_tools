
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = 	../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include


HEADERS	+= save_3D_tiles_plugin.h
SOURCES	+= save_3D_tiles_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(save_3D_tiles)
DESTDIR	= $$VAA3DPATH/bin/plugins/data_IO/save_3D_tiles/
