
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include

HEADERS	+= cropped3DImageSeries_plugin.h
HEADERS	+= ../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h
HEADERS	+= ../../../released_plugins/v3d_plugins/istitch/y_imglib.h


SOURCES	+= cropped3DImageSeries_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp


SOURCES	+=  ../../../hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_image_lib.cpp

LIBS         += -lm -L$$VAA3DPATH/common_lib/lib -lv3dtiff
LIBS         += -lpthread
LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads

TARGET	= $$qtLibraryTarget(cropped3DImageSeries)
DESTDIR	= $$VAA3DPATH/../bin/plugins/image_geometry/crop3d_image_series
