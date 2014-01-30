
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun


HEADERS	+= image_blend_plugin.h
SOURCES	+= image_blend_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(blend_two_images)
DESTDIR	=  ../../../../v3d_external/bin/plugins/image_blending/blend_two_images
