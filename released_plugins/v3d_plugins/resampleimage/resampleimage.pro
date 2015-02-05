
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH += $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include


HEADERS	+= resampleimage_plugin.h
HEADERS	+= ../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_upSample3D.h
HEADERS	+= ../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_downSample3D.h
HEADERS	+= ../../../released_plugins/v3d_plugins/istitch/y_imglib.h


SOURCES	+= resampleimage_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(resampleimage)
DESTDIR	= $$VAA3DPATH/../bin/plugins/image_geometry/image_resample/
