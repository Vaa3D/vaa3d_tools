
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main 

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += gaussianfilterplugin.h
SOURCES  = gaussianfilterplugin.cpp

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET        = $$qtLibraryTarget(gaussianfilter)
DESTDIR       = ../../v3d/plugins/image_filters/Gaussian_Filter

