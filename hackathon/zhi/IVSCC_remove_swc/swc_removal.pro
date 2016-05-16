
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= swc_removal_plugin.h
SOURCES	+= swc_removal_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(swc_removal)
DESTDIR	= $$VAA3DPATH/bin/plugins/IVSCC/IVSCC_swc_removal/
