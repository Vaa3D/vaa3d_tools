
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= BoutonDectection_plugin.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../v3d_main/basic_c_fun/color_xyz.h \
    boutonDetection_fun.h
SOURCES	+= BoutonDectection_plugin.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp \
    boutonDetection_fun.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BoutonDectection)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/BoutonDectection/
