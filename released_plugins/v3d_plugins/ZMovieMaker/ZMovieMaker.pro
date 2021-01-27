
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

HEADERS	+= ZMovieMaker_plugin.h
SOURCES	+= ZMovieMaker_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(ZMovieMaker)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/movies_and_snapshots/ZMovieMaker/
