
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= branches_detection_v2_plugin.h \
    branches_detection_v2.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h \
    rayshooting_func.h
SOURCES	+= branches_detection_v2_plugin.cpp \
    branches_detection_v2.cpp \
    rayshooting_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(branches_detection_v2)
DESTDIR	= $$VAA3DPATH/bin/plugins/branches_detection_v2/
