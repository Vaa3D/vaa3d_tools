
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/jazz/Desktop/v3d_new/v3d_external/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
QT += widgets
QT += core
HEADERS	+= test_plugin.h \
    myswitch.h
SOURCES	+= test_plugin.cpp \
    myswitch.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(TeraBeforeChange)
DESTDIR	= /Users/jazz/Desktop/vaa3dapp/plugins/TeraBeforeChange


