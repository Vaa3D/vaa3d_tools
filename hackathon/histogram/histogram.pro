
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/zhouj/work/vaa3d
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= histogram_plugin.h
HEADERS	+= histogram_func.h
HEADERS	+= histogram_gui.h

SOURCES	= histogram_plugin.cpp
SOURCES	+= histogram_func.cpp
SOURCES	+= histogram_gui.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(histogram)
DESTDIR	= $$VAA3DPATH/bin/plugins/histogram/
