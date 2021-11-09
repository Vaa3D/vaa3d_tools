
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =  ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= convert_file_format_plugin.h
SOURCES	+= convert_file_format_plugin.cpp

SOURCES += $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(convert_file_format)
DESTDIR	= $$VAA3DPATH/bin/plugins/data_IO/convert_file_format/

