
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/terafly/SRCMOC

HEADERS	+= test_00_plugin.h \
    some_function.h
SOURCES	+= test_00_plugin.cpp \
    some_function.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(test_00)
DESTDIR	= $$VAA3DPATH/bin/plugins/test_00/
