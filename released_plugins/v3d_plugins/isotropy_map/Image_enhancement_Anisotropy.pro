
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat


HEADERS	+= Image_enhancement_Anisotropy_plugin.h
SOURCES	+= Image_enhancement_Anisotropy_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(image_enhancement_anisotropy)
DESTDIR	= ../../../../v3d_external/bin/plugins/image_enhancement_anisotropy/
