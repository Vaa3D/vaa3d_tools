
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS       = istitch.h 
HEADERS      +=	istitch_gui.h
HEADERS      +=	y_imglib.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h

SOURCES       = istitch.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp


win32{
    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibtiff
    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3f-3
#    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3l-3
#    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3-3
    }
unix{
     LIBS	  += -lv3dfftw3f -lv3dfftw3f_threads
     LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
     LIBS         += -lpthread
}
TARGET        = $$qtLibraryTarget(imageStitch)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/image_stitching/istitch

RESOURCE     += istitch.qrc

