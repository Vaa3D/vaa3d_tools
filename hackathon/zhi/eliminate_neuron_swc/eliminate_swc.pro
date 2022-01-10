
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include

HEADERS	+= eliminate_swc_plugin.h
SOURCES	+= eliminate_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(eliminate_swc)
DESTDIR	= $$VAA3DPATH/../bin/plugins/eliminate_swc/
