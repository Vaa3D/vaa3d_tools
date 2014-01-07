
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
PLUGINPATH = ../../../released_plugins/v3d_plugins/fl_cellseg/src
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH     += $$PLUGINPATH
INCLUDEPATH     += $$PLUGINPATH/..

HEADERS	+= GVFplugin_plugin.h
HEADERS += $$PLUGINPATH/FL_gvfCellSeg.h

SOURCES	+= GVFplugin_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$PLUGINPATH/FL_gvfCellSeg.cpp

TARGET	= $$qtLibraryTarget(GVFplugin)
DESTDIR	= $$VAA3DPATH/../../v3d_external/bin/plugins/GVFplugin/
