
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
PLUGINPATH = ./src/
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH     += $$PLUGINPATH
INCLUDEPATH     += $$PLUGINPATH/..

HEADERS	+= GVFplugin_plugin.h
HEADERS += $$PLUGINPATH/FL_gvfCellSeg.h

SOURCES	+= GVFplugin_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$PLUGINPATH/FL_gvfCellSeg.cpp

FORMS += src/FL_watershedSegPara.ui

TARGET	= $$qtLibraryTarget(gvf_cellseg)
DESTDIR	= $$VAA3DPATH/../bin/plugins/image_segmentation/Cell_Segmentation_GVF/
