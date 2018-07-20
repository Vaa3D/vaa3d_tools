
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= make_segments_plugin.h \
    my_surf_objs.h \
    swc_utils.h \
    basic_surf_objs.h
SOURCES	+= make_segments_plugin.cpp \
    my_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(make_segments)
DESTDIR	= $$VAA3DPATH/bin/plugins/make_segments/
