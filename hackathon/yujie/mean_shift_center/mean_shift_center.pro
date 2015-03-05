
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= \
    mean_shift_dialog.h \
    mean_shift_center_plugin.h \
    mean_shift_extr_template.h \
    mean_shift_fun.h
SOURCES	+= \
    mean_shift_dialog.cpp \
    mean_shift_center_plugin.cpp \
    mean_shift_fun.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(mean_shift_center_finder)
DESTDIR	= $$VAA3DPATH/bin/plugins/mean_shift_center_finder/
