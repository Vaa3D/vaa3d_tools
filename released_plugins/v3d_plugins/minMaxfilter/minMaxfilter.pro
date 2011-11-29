
TEMPLATE      = lib
CONFIG       += plugin warn_off
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
HEADERS       = minMaxfilterplugin.h
SOURCES       = minMaxfilterplugin.cpp
TARGET        = $$qtLibraryTarget(minMaxfilter)
DESTDIR       = ../../v3d/plugins/image_filters/min_Max_Filter

