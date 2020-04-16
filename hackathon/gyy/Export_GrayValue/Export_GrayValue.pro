
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG += qaxcontainer
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Export_GrayValue_plugin.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= Export_GrayValue_plugin.cpp \
    calculateGrayValue.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Export_GrayValue)
DESTDIR	= $$VAA3DPATH/bin/plugins/Export_GrayValue/
