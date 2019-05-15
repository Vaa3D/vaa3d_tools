
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main 

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += steerablefilter2Dplugin.h
HEADERS += steerableDetector.h
HEADERS += convolver.h
SOURCES  = steerablefilter2Dplugin.cpp

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += steerableDetector.cpp
SOURCES += convolver.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET        = $$qtLibraryTarget(steerablefilter2D)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_filters/steerablefilter2D

