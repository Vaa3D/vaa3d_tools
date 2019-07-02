
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Vaa3D_2013_Qt486/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= BrainAtlas_plugin.h
SOURCES	+= BrainAtlas_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BrainAtlas)
DESTDIR	= $$VAA3DPATH/bin/plugins/BrainAtlas/
