
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Branches_Detection_plugin.h \
    branches_detection.h \
        rayshooting_func.h \
            $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= Branches_Detection_plugin.cpp \
    branches_detection.cpp \
        rayshooting_func.cpp \
            $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Branches_Detection)
DESTDIR	= $$VAA3DPATH/bin/plugins/Branches_Detection/
