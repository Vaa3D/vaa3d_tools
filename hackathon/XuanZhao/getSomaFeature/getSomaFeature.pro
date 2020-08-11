
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= getSomaFeature_plugin.h \
    somefunction.h
SOURCES	+= getSomaFeature_plugin.cpp \
    somefunction.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(getSomaFeature)
DESTDIR	= $$VAA3DPATH/bin/plugins/getSomaFeature/
