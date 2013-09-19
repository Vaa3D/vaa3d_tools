
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= extractZSlices_plugin.h
SOURCES	+= extractZSlices_plugin.cpp

TARGET	= $$qtLibraryTarget(extractZSlices)
#DESTDIR	= $$VAA3DPATH/bin/plugins/image_geometry/save_Z_Slices/
DESTDIR = ../../v3d/plugins/image_IO/save_Z_Slices/

