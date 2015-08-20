
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= cropImageTrace_plugin.h \
    crop_dialog.h
SOURCES	+= cropImageTrace_plugin.cpp \
    crop_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp \
           $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(cropImageTrace)
DESTDIR	= $$VAA3DPATH/bin/plugins/cropImageTrace/
