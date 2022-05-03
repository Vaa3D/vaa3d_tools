
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
QT += widgets
HEADERS	+= HistogramEqualization_plugin.h
#HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h
#HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_interface.h

SOURCES	+= HistogramEqualization_plugin.cpp \
    HistogramEqualization_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(HistogramEqualization)
DESTDIR	= $$VAA3DPATH/bin/plugins/HistogramEqualization/
