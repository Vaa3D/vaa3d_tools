
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= saveZSlices_plugin.h
SOURCES	+= saveZSlices_plugin.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(saveZSlices)
#DESTDIR	= $$VAA3DPATH/bin/plugins/data_IO/save_Z_Slices/
DESTDIR = ../../v3d/plugins/data_IO/save_Z_Slices/

