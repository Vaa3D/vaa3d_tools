
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/zhihaozh/vaa3d_source/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= VaaPortTest_plugin.h
SOURCES	+= VaaPortTest_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(VaaPortTest)
DESTDIR	= $$VAA3DPATH/bin/plugins/VaaPortTest/
