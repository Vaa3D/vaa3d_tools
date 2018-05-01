
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= sync_swc_terafly_plugin.h

SOURCES	+= sync_swc_terafly_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(sync_swc_terafly)
DESTDIR	= $$VAA3DPATH/../bin/plugins/Sync_Views/sync_swc_terafly/
