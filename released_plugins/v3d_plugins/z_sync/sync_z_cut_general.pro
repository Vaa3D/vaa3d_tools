
TEMPLATE = lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= sync_z_cut_plugin.h
SOURCES	+= sync_z_cut_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(sync_z_cut)
DESTDIR	= $$VAA3DPATH/../bin/plugins/sync_z_cut
