
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main 

HEADERS += updatepxlvalplugin.h
SOURCES  = updatepxlvalplugin.cpp

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(updatepxlval)
DESTDIR       = ../../v3d/plugins/pixel_intensity/Change_single_pixel_value

