
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS       = iBioformatIO.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp

SOURCES       = iBioformatIO.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

win32 {
	LIBS         += -L$$V3DMAINDIR/common_lib/winlib64 -llibtiff
}
else {
	LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
}
#LIBS         += -lpthread

TARGET        = $$qtLibraryTarget(imageIO_Bioformat)
DESTDIR       = ../../../v3d/plugins/data_IO/load_image_using_Bioformats

win32 {
	QMAKE_POST_LINK = copy loci_tools.jar ../../v3d/plugins/data_IO/load_image_using_Bioformats
}
else {
	QMAKE_POST_LINK = cp loci_tools.jar ../../v3d/plugins/data_IO/load_image_using_Bioformats
}


