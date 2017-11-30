
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Vaa3D_2013_Qt486/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Deep_Learning_Handler_plugin.h
SOURCES	+= Deep_Learning_Handler_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += funcs.cpp

TARGET	= $$qtLibraryTarget(Deep_Learning_Handler)
DESTDIR	= $$VAA3DPATH/bin/plugins/Deep_Learning_Handler/
