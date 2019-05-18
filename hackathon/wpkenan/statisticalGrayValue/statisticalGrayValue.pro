
TEMPLATE = lib
CONFIG += qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
V3DMAINPATH = $$VAA3DPATH/v3d_main
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS += statisticalGrayValue_plugin.h
SOURCES += statisticalGrayValue_plugin.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET = $$qtLibraryTarget(statisticalGrayValue)
DESTDIR = $$V3DMAINPATH/../bin/plugins/wpkenanPlugin/statisticalGrayValue

