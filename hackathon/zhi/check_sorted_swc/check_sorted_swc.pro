
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /local1/work/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= check_sorted_swc_plugin.h
HEADERS += openSWCDialog.h

SOURCES	+= check_sorted_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += openSWCDialog.cpp

TARGET	= $$qtLibraryTarget(check_sorted_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/check_sorted_swc/
