
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:\v3d\v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= findCrossBranch_plugin.h
SOURCES	+= findCrossBranch_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(findCrossBranch)
DESTDIR	= $$VAA3DPATH/bin/plugins/findCrossBranch/
