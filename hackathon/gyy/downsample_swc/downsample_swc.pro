
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= downsample_swc_plugin.h \
    downsample_gui.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= downsample_swc_plugin.cpp \
    downsample_file.cpp \
    downsample_gui.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(downsample_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/downsample_swc/
