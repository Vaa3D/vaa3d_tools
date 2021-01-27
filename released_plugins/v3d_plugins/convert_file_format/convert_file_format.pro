
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= convert_file_format_plugin.h
SOURCES	+= convert_file_format_plugin.cpp

SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(convert_file_format)
DESTDIR	= $$V3DMAINPATH/bin/plugins/data_IO/convert_file_format/

