
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= extractZSlices_plugin.h
SOURCES	+= extractZSlices_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(extractZSlices)
DESTDIR	= $$VAA3DPATH/bin/plugins/extractZSlices/
