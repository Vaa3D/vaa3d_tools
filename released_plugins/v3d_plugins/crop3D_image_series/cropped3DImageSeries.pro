
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d

HEADERS	+= cropped3DImageSeries_plugin.h
HEADERS	+= ../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h
HEADERS	+= ../../../released_plugins/v3d_plugins/istitch/y_imglib.h


SOURCES	+= cropped3DImageSeries_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp


SOURCES	+=  ../../../hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES      +=	$$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp

win32{
LIBS         += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
}

unix{
LIBS         += -lpthread
LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads
LIBS         += -lm -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
}

TARGET	= $$qtLibraryTarget(cropped3DImageSeries)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_geometry/crop3d_image_series
