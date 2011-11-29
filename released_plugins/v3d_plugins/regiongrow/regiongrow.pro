
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = regiongrow.h
SOURCES       = regiongrow.cpp
TARGET        = $$qtLibraryTarget(regiongrow)
DESTDIR       = ../../v3d/plugins/image_segmentation/Label_Objects


