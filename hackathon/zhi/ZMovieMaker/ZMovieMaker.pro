
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main

HEADERS	+= Smooth_Movie_plugin.h
SOURCES	+= Smooth_Movie_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Smooth_Movie)
DESTDIR	= $$VAA3DPATH/../bin/plugins/movies/Smooth_Movie/
