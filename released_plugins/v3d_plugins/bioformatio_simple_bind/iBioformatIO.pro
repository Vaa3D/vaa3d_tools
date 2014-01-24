
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS       = iBioformatIO.h

SOURCES       = iBioformatIO.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(imageIO_Bioformat)
DESTDIR       = ../../v3d/plugins/data_IO/load_image_using_Bioformats

win32 {
        QMAKE_POST_LINK = copy loci_tools.jar ..\\..\\v3d\\plugins\\data_IO\\load_image_using_Bioformats\\.
}
else {
        QMAKE_POST_LINK = cp loci_tools.jar ../../v3d/plugins/data_IO/load_image_using_Bioformats/.
}


