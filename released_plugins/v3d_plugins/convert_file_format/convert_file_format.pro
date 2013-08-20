
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= convert_file_format_plugin.h
SOURCES	+= convert_file_format_plugin.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(convert_file_format)
#DESTDIR	= $$VAA3DPATH/bin/plugins/data_IO/convert_file_format/
DESTDIR = ../../v3d/plugins/data_IO/convert_file_format/

