
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ../bigneuron_siqi_stalker_v3d/lib/ITKlibs_MAC
}
else{
    ITKLIBPATH = ../bigneuron_siqi_stalker_v3d/lib/ITKlibs_Linux
    SOURCES = lib/ITK_include/itkLightProcessObject.cxx
}


VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += lib/ITK_include
INCLUDEPATH += lib/Filters
#LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib
LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat

HEADERS	+= Rivulet_plugin.h
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp
SOURCES	+= Rivulet_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.h
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp
HEADERS += fastmarching_dt.h
HEADERS += utils/msfm.h
SOURCES += utils/msfm.cpp
SOURCES += utils/common.c
HEADERS += rk4.h
SOURCES += rk4.cpp

HEADERS += lib/ImageOperation.h\
           lib/PointOperation.h\
           lib/SnakeOperation.h
SOURCES	+= lib/ImageOperation.cpp\
           lib/PointOperation.cpp\
           lib/SnakeOperation.cpp\
	   lib/ITK_include/vcl_deprecated.cxx	
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.h
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp

LIBS         += -lm -L$$VAA3DPATH/v3d_main/common_lib/lib -lv3dtiff
LIBS         += -lpthread
LIBS         += -lv3dfftw3f -lv3dfftw3f_threads

TARGET	= $$qtLibraryTarget(Rivulet)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_tracing/Rivulet/
