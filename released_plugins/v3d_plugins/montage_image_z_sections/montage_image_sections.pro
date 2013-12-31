
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS += montage_image_sections.h
SOURCES  = montage_image_sections.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(montage_image_sections)
DESTDIR       = ../../v3d/plugins/image_geometry/Montage_All_Z_Sections
