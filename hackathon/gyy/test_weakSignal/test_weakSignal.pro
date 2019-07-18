
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/balala/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= test_weakSignal_plugin.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES	+= test_weakSignal_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(test_weakSignal)
DESTDIR	= $$VAA3DPATH/bin/plugins/test_weakSignal/
