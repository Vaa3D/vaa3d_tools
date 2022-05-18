
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH =  ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
QT += widgets
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
HEADERS	+= standardize_image_plugin.h
SOURCES	+= standardize_image_plugin.cpp \
    standardize_func.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(standardize_image)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/standardize_image/
