
TEMPLATE      = lib
CONFIG       += plugin warn_off
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = updatepxlvalplugin.h
SOURCES       = updatepxlvalplugin.cpp
TARGET        = $$qtLibraryTarget(updatepxlval)
DESTDIR       = ../../v3d/plugins/pixel_intensity/Change_single_pixel_value

