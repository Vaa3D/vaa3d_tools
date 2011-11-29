TEMPLATE      = lib
CONFIG       += qt plugin warn_off
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = montage_image_sections.h
SOURCES       = montage_image_sections.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(montage_image_sections)
DESTDIR       = ../../v3d/plugins/image_geometry/Montage_All_Z_Sections
