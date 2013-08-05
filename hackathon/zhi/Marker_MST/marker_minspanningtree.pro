
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= marker_minspanningtree_plugin.h
SOURCES	+= marker_minspanningtree_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(marker_minspanningtree)
DESTDIR	= ../../../../v3d_external/bin//plugins/marker_minspanningtree/
