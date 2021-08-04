
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include

HEADERS	+= image_quality_plugin.h
HEADERS	+= image_quality_func.h
HEADERS	+= image_quality_gui.h

SOURCES	= image_quality_plugin.cpp
SOURCES	+= image_quality_func.cpp
SOURCES	+= image_quality_gui.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(image_quality)
DESTDIR	= $$VAA3DPATH/../bin/plugins/image_analysis/image_quality/
