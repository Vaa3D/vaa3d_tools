
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Vaa3D_2013_Qt486/v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

FORMS += fragmentedTraceUI.ui

HEADERS	+= Fragmented_Auto-trace_plugin.h
SOURCES	+= Fragmented_Auto-trace_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Fragmented_Auto-trace)
DESTDIR	= ../../../../v3d_external/bin/plugins/Fragmented_Auto-trace/
