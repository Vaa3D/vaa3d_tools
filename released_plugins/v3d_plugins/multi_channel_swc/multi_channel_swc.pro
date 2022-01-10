
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= multi_channel_swc_plugin.h \
    multi_channel_swc_dialog.h \
    multi_channel_swc_template.h \
    multi_channel_swc_func.h

SOURCES	+= multi_channel_swc_plugin.cpp \
    multi_channel_swc_dialog.cpp \
    multi_channel_swc_func.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(multi_channel_swc)
DESTDIR	= $$VAA3DPATH/../bin/plugins/multi_channel_swc/
