
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/balala/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h

HEADERS	+= test00_plugin.h
SOURCES	+= test00_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(test00)
DESTDIR	= $$VAA3DPATH/bin/plugins/test00/
