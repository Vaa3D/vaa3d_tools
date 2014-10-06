
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += main 

HEADERS += ada_threshold.h
SOURCES  = ada_threshold.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(ada_threshold)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_thresholding/Simple_Adaptive_Thresholding
