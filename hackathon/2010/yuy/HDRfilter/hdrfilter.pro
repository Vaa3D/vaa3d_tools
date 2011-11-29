
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun

HEADERS      += hdrfilter.h 
HEADERS      += hdrfilter_gui.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h

SOURCES      += hdrfilter.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp

LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff

TARGET        = $$qtLibraryTarget(hdrfilter)
DESTDIR       = ../../v3d/plugins/image_filters/HDR_Filter

