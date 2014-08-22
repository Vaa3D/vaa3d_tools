
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
#VAA3DPATH = D:\VAA3D_STUFF\v3d_external
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= sync_z_cut_plugin.h
SOURCES	+= sync_z_cut_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(sync_z_cut)
DESTDIR	= D:\VAA3D_STUFF\vaa3d_win7_32bit_v2.707\plugins\sync_z_cut
