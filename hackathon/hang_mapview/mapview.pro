
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/xiaoh10/work/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= src

HEADERS	+= mapview_plugin.h
HEADERS	+= mapview_gui.h
HEADERS += src/mapview.h

SOURCES	+= mapview_plugin.cpp
SOURCES += src/mapview.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(mapview)
DESTDIR	= $$VAA3DPATH/bin/plugins/mapview/
