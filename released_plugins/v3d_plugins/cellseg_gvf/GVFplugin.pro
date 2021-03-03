
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
PLUGINPATH = ./src/
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d
INCLUDEPATH     += $$PLUGINPATH
INCLUDEPATH     += $$PLUGINPATH/..

HEADERS	+= GVFplugin_plugin.h
HEADERS += $$PLUGINPATH/FL_gvfCellSeg.h

SOURCES	+= GVFplugin_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$PLUGINPATH/FL_gvfCellSeg.cpp

FORMS += src/FL_watershedSegPara.ui

TARGET	= $$qtLibraryTarget(gvf_cellseg)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_segmentation/Cell_Segmentation_GVF/
