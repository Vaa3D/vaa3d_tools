
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include


HEADERS	+= IVSCC_import_data_plugin.h
SOURCES	+= IVSCC_import_data_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(IVSCC_import_data)
DESTDIR	= $$VAA3DPATH/../bin/plugins/IVSCC/IVSCC_import_data/
