
TEMPLATE      = lib

CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_external/v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS       = multithreaded_imgloading.h
HEADERS      += y_img_thread.h
				
SOURCES       = multithreaded_imgloading.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp

LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff

TARGET        = $$qtLibraryTarget(multithreaded_imgloading)
DESTDIR       = $$V3DMAINDIR/../v3d/plugins/data_IO/multithreaded_image_loading

