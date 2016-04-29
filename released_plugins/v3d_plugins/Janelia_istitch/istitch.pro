
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS       = istitch.h 
HEADERS      +=	istitch_gui.h
HEADERS      +=	y_imglib.h

SOURCES       = istitch.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

win32{
    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3f-3
#    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3l-3
#    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3-3
    }
unix{
     LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dfftw3f
     LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads
     LIBS         += -lpthread
}

TARGET        = $$qtLibraryTarget(Janelia_imageStitch)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/image_stitching/Janelia_istitch

RESOURCE     += istitch.qrc

