
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH += $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include

HEADERS	+= saveZSlices_plugin.h
SOURCES	+= saveZSlices_plugin.cpp

SOURCES += $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(saveZSlices)
DESTDIR	= $$VAA3DPATH/../bin/plugins/data_IO/save_Z_Slices/

