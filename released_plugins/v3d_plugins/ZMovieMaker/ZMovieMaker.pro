
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main

HEADERS	+= ZMovieMaker_plugin.h
SOURCES	+= ZMovieMaker_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(ZMovieMaker)
DESTDIR	= $$VAA3DPATH/../bin/plugins/movies/ZMovieMaker/
