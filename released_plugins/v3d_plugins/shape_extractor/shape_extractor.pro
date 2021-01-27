
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= shape_extractor_plugin.h \
    shape_dialog.h \
    shape_extr_template.h \
    extract_fun.h
SOURCES	+= shape_extractor_plugin.cpp \
    shape_dialog.cpp \
    extract_fun.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(shape_extractor)
DESTDIR	= $$V3DMAINPATH/bin/plugins/shape_analysis/shape_extractor/
