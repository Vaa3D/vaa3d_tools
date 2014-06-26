
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Users/Xinnamin/Documents/vaa3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= test_plugin.h
SOURCES	+= test_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(test)
DESTDIR	= C:/Users/Xinnamin/Documents/vaa3d_win7_32bit_v2.707/plugins/test/
