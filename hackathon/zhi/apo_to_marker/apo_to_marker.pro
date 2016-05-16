
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= apo_to_marker_plugin.h
SOURCES	+= apo_to_marker_plugin.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(apo_to_marker)
DESTDIR	= $$VAA3DPATH/bin/plugins/apo_to_marker/
