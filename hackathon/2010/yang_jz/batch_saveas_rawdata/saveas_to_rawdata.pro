TEMPLATE      = lib
CONFIG       += qt plugin warn_off
V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
QT_DIR = $$dirname(QMAKE_QMAKE)/..


HEADERS      +=../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += saveas_to_rawdata.h

#HEADERS      +=../../../v3d_main/basic_c_fun/stackutil.h
#HEADERS      +=../../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS      +=../../../v3d_main/basic_c_fun/mg_utilities.h 
HEADERS      +=../../../v3d_main/basic_c_fun/imageio_mylib.h

HEADERS      += mg_image_lib11.h
HEADERS      += stackutil-11.h

SOURCES      +=../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += saveas_to_rawdata.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
#SOURCES      +=../../../v3d_main/basic_c_fun/stackutil.cpp
#SOURCES      +=../../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES      +=../../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES       += mg_image_lib11.cpp 
SOURCES       += stackutil-11.cpp

SOURCES      +=../../../v3d_main/basic_c_fun/imageio_mylib.cpp

LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
             += -L$$QTDIR/demos/shared/
LIBS         += -lm -L$$V3DMAINDIR/common_lib/src_packages/mylib_tiff -lmylib
LIBS         += -lpthread
LIBS	        += -lv3dfftw3f -lv3dfftw3f_threads

TARGET        = $$qtLibraryTarget(saveas_to_rawdata)
DESTDIR       = ../../../v3d/plugins/saveas_to_rawdata


