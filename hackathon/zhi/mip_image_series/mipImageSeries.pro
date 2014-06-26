
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include


HEADERS	+= mipImageSeries_plugin.h
SOURCES	+= mipImageSeries_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(mipImageSeries)
DESTDIR =  $$VAA3DPATH/../bin/plugins/image_projection/maximum_intensity_projection_Image_Series/
