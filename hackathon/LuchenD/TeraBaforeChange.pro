
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = E:/Vaa3D_Qt6/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= TeraBaforeChange_plugin.h
SOURCES	+= TeraBaforeChange_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(TeraBaforeChange)
DESTDIR	= $$VAA3DPATH/bin/plugins/TeraBaforeChange/
