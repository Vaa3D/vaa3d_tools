
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/brl/vaa3d_related/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/jba/newmat11




HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.h
HEADERS	+= BRL00_plugin.h


SOURCES	+= BRL00_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp

TARGET	= $$qtLibraryTarget(BRL00)
DESTDIR	= $$VAA3DPATH/bin/plugins/BRL00/
