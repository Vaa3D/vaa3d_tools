
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Users/Jade/Documents/V3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= shape_extractor_plugin.h \
    shape_dialog.h \
    shape_extr_template.h \
    extract_fun.h
SOURCES	+= shape_extractor_plugin.cpp \
    shape_dialog.cpp \
    extract_fun.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(shape_extractor)
DESTDIR	= $$VAA3DPATH/bin/plugins/shape_extractor/
