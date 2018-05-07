
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS      += ireg.h
HEADERS      += y_NiftiImageIO.h
HEADERS      += y_imgreg.h
HEADERS      += y_img.h
HEADERS      += y_img.hxx

HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
unix:HEADERS += $$V3DMAINDIR/basic_c_fun/imageio_mylib.h

HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h

HEADERS      += niftilib/nifti1.h
HEADERS      += niftilib/nifti1_io.h
HEADERS      += niftilib/znzlib.h

SOURCES      += ireg.cpp
SOURCES      += y_NiftiImageIO.cpp

SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
unix:SOURCES += $$V3DMAINDIR/basic_c_fun/imageio_mylib.cpp

SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

SOURCES      += niftilib/nifti1_io.c
SOURCES      += niftilib/znzlib.c

LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DMAINDIR/common_lib/src_packages/mylib_tiff -lmylib
unix:LIBS    += -lm -lpthread -lz
#LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads

QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.7
QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64

TARGET        = $$qtLibraryTarget(ireg)
#DESTDIR       = ../../v3d/plugins/image_registration/ireg
DESTDIR       =$$V3DMAINDIR/../bin/plugins/image_registration/ireg
