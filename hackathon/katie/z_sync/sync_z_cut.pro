
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= sync_z_cut_plugin.h
SOURCES	+= sync_z_cut_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(sync_z_cut)
DESTDIR	= /Users/zhiz/work/v3d/v3d_external/bin/plugins/sync_z_cut
