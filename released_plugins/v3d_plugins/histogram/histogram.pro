
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= histogram_plugin.h
HEADERS	+= histogram_func.h
HEADERS	+= histogram_gui.h

SOURCES	= histogram_plugin.cpp
SOURCES	+= histogram_func.cpp
SOURCES	+= histogram_gui.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(histogram)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_analysis/histogram/
