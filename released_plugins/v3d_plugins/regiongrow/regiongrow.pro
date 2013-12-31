
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS += regiongrow.h
SOURCES  = regiongrow.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(regiongrow)
DESTDIR       = ../../v3d/plugins/image_segmentation/Label_Objects
