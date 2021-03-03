
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
INCLUDEPATH  += $$V3DMAINDIR/v3d

HEADERS      += y_lsminfo.h
HEADERS      += imageblend.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/imageio_mylib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_4dimage.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h

SOURCES      += imageblend.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/imageio_mylib.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_4dimage.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += basic_4dimage_ori.cpp

LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DMAINDIR/common_lib/src_packages/mylib_tiff -lmylib
LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dfftw3f
LIBS         += -L$$V3DMAINDIR/common_lib/lib -lv3dfftw3f_threads
LIBS         += -lm -lpthread

TARGET        = $$qtLibraryTarget(Janelia_blend_multiscanstacks)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/FlyWorkstation_utilities/Janelia_blend_multiscanstacks

