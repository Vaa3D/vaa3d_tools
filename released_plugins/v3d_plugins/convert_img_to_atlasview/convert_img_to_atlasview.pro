
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS       = convert_img_to_atlasview.h

SOURCES       = convert_img_to_atlasview.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(convert_img_to_atlasview)
DESTDIR       = ../../v3d/plugins/data_type/Convert_Image_to_AtlasViewMode

