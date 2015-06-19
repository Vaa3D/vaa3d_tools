
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= multi_channel_swc_plugin.h \
    multi_channel_swc_dialog.h \
    multi_channel_swc_template.h
SOURCES	+= multi_channel_swc_plugin.cpp \
    multi_channel_swc_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(multi_channel_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/multi_channel_swc/
