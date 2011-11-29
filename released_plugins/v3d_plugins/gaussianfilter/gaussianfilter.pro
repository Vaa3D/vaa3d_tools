
TEMPLATE      = lib
CONFIG       += plugin warn_off
#CONFIG	     += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = gaussianfilterplugin.h
SOURCES       = gaussianfilterplugin.cpp
TARGET        = $$qtLibraryTarget(gaussianfilter)
DESTDIR       = ../../v3d/plugins/image_filters/Gaussian_Filter

