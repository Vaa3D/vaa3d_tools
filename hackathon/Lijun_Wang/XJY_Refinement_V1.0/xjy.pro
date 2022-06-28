
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

INCLUDEPATH += C:\Users\82700\anaconda3\envs\tensorflow\include   ############# python enviroment
LIBS += -LC:\Users\82700\anaconda3\envs\tensorflow\libs\
-l_tkinter\
-lpython3\
-lpython36
