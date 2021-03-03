
TEMPLATE = lib
CONFIG	+= qt plugin warn_off

macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = lib/ITKlibs_MAC
}
else{
    ITKLIBPATH = lib/ITKlibs_Linux
    SOURCES = lib/ITK_include/itkLightProcessObject.cxx
}

V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH += lib/ITK_include
INCLUDEPATH += lib/Filters

LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib
LIBS += -L$$V3DMAINPATH/v3d_main/jba/c++ -lv3dnewmat

HEADERS	+= NeuroStalker_plugin.h\
           PressureSampler.h\
           DandelionTracer.h
HEADERS += utils/vn_imgpreprocess.h\
           utils/matmath.h

HEADERS += test/unittest.h

HEADERS += lib/ImageOperation.h\
           lib/PointOperation.h\
           lib/SnakeOperation.h

HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.h

SOURCES	+= NeuroStalker_plugin.cpp\
           PressureSampler.cpp\
           DandelionTracer.cpp\
           utils/vn_imgpreprocess.cpp\
           utils/matmath.cpp\
           lib/ImageOperation.cpp\
           lib/PointOperation.cpp\
           lib/SnakeOperation.cpp\
           test/unittest.cpp\
           lib/ITK_include/vcl_deprecated.cxx

SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.cpp

LIBS         += -lm -L$$V3DMAINPATH/v3d_main/common_lib/lib -lv3dtiff
LIBS         += -lpthread
LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads

TARGET	= $$qtLibraryTarget(NeuroStalker)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/NeuroStalker/
