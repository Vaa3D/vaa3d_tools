
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/penglab/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= soma_remove_plugin.h \
    soma_remove_main.h
SOURCES	+= soma_remove_plugin.cpp \
    soma_remove_main.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(soma_remove)
DESTDIR	= $$VAA3DPATH/bin/plugins/soma_remove/
