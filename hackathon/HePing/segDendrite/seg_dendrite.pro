
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vs2013project/vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= seg_dendrite_plugin.h \
    seg_dendrite.h
SOURCES	+= seg_dendrite_plugin.cpp \
    seg_dendrite.cpp\
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(seg_dendrite)
DESTDIR	= $$VAA3DPATH/bin/plugins/seg_dendrite/
