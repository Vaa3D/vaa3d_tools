
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS += montage_image_sections.h
SOURCES  = montage_image_sections.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(montage_image_sections)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_geometry/Montage_All_Z_Sections
