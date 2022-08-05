
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= UnsortedPlugin_plugin.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../v3d_main/basic_c_fun/color_xyz.h \
    ../../../v3d_main/basic_c_fun/basic_4dimage.h \
    my_fun.h \
    histeq.h
SOURCES	+= UnsortedPlugin_plugin.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp \
    ../../../v3d_main/basic_c_fun/basic_4dimage_create.cpp \
    my_fun.cpp \
    histeq.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(UnsortedPlugin)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/UnsortedPlugin/
