
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS += minMaxfilterplugin.h
SOURCES  = minMaxfilterplugin.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(minMaxfilter)
DESTDIR       = ../../v3d/plugins/image_filters/min_Max_Filter

