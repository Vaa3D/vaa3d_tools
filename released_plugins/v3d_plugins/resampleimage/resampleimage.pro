
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d


HEADERS	+= resampleimage_plugin.h
HEADERS	+= ../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_upSample3D.h
HEADERS	+= ../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_downSample3D.h
HEADERS	+= ../../../released_plugins/v3d_plugins/istitch/y_imglib.h


SOURCES	+= resampleimage_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(resampleimage)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_geometry/image_resample/
