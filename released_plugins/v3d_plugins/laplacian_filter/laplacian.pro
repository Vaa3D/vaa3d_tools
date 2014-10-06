
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

HEADERS       += laplacianfilterplugin.h

SOURCES       = laplacianfilterplugin.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(laplacianfilter)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_filters/Laplacian_Filter

