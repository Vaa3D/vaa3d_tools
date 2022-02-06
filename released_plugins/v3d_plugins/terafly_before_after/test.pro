
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = E:/Vaa3D_Qt6/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
QT += widgets
QT += core
HEADERS	+= test_plugin.h \
    myswitch.h
SOURCES	+= test_plugin.cpp \
    myswitch.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(TeraBeforeChange)
DESTDIR	= $$VAA3DPATH/bin/plugins/TeraBaforeChange/


