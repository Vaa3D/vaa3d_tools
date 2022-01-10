
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= shape_extractor_plugin.h \
    shape_dialog.h \
    shape_extr_template.h \
    extract_fun.h
SOURCES	+= shape_extractor_plugin.cpp \
    shape_dialog.cpp \
    extract_fun.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(shape_extractor)
DESTDIR	= $$VAA3DPATH/../bin/plugins/shape_analysis/shape_extractor/
