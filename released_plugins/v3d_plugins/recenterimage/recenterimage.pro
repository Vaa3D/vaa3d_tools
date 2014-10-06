# revised by Hanchuan Peng
# 2010-05-20. Note that there is a problem about non-VC compliers (e.g. gcc) may complain the redundant compiling of vcdiff.h/.cpp

TEMPLATE      = lib
CONFIG       += plugin warn_off

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS      += recenterimageplugin.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/vcdiff.h
#HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
#HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
#HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
#HEADERS      += $$V3DMAINDIR/basic_c_fun/imageio_mylib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_4dimage.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h

SOURCES      += recenterimageplugin.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/vcdiff.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
#SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
#SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/imageio_mylib.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_4dimage.cpp # THIS IS INCLUDED IN THE cpp FILE.
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
#SOURCES      += ../blend_multiscanstacks/basic_4dimage_ori.cpp

#LIBS         += -L$$V3DMAINDIR/common_lib/lib  -lv3dtiff
#LIBS         += -L$$V3DMAINDIR/common_lib/src_packages/mylib_tiff -lmylib

TARGET        = $$qtLibraryTarget(recenterimage)
DESTDIR       = V3DMAINDIR/../bin/plugins/image_geometry/ReCenter_Image

