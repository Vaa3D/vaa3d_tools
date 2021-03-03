
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= mipImageSeries_plugin.h
SOURCES	+= mipImageSeries_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(mipImageSeries)
DESTDIR =  $$V3DMAINPATH/../bin/plugins/image_projection/maximum_intensity_projection_Image_Series/
