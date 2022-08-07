
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= xjy_plugin.h
SOURCES	+= xjy_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(xjy)
DESTDIR	= $$VAA3DPATH/bin/plugins/xjy/
QT += core widgets

DISTFILES += \
    split_swc.py

INCLUDEPATH += $$VAA3DPATH\bin\Python36\include   ############# python enviroment
LIBS += -L$$VAA3DPATH\bin\Python36\libs\
-l_tkinter\
-lpython3\
-lpython36
