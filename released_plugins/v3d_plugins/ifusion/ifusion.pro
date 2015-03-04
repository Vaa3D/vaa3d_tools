
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

INCLUDEPATH += main

HEADERS      += ifusion.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h

SOURCES      += ifusion.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

win32{
    LIBS	     += -L$$V3DMAINDIR/common_lib/winlib64 -llibfftw3f-3
    }

unix{
    LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dfftw3f
    LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dfftw3f_threads
    LIBS         += -lm -lpthread
}



TARGET        = $$qtLibraryTarget(ifusion)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/image_stitching/ifusion/
