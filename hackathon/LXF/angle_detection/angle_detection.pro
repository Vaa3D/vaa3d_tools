
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= angle_detection_plugin.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= angle_detection_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(angle_detection)
DESTDIR	= $$VAA3DPATH/bin/plugins/angle_detection/
