
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
LIBS += -L$$V3DMAINPATH/v3d_main/jba/c++ -lv3dnewmat

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

unix:!macx {
    QMAKE_CXXFLAGS += -std=c++0x
    QMAKE_CXXFLAGS  += -std=c++11
}

# From V3D Main
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.h
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.h
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.cpp

# Main Plugin
HEADERS	+= rivulet.h
SOURCES	+= rivulet_utils.cpp
SOURCES	+= rivulet_tracer.cpp
HEADERS	+= Rivulet_plugin.h
SOURCES	+= Rivulet_plugin.cpp

# Utils
HEADERS += utils/rk4.h
SOURCES += utils/rk4.cpp
HEADERS += utils/graph.h
SOURCES += utils/graph.cpp
HEADERS += utils/marker_radius.h

# Fastmarching
HEADERS += fastmarching/fastmarching_dt.h
HEADERS += fastmarching/msfm.h
SOURCES += fastmarching/msfm.cpp
SOURCES += fastmarching/common.c
HEADERS += fastmarching/my_surf_objs.h
SOURCES += fastmarching/my_surf_objs.cpp

LIBS         += -lm -L$$V3DMAINPATH/v3d_main/common_lib/lib -lv3dtiff
LIBS         += -lpthread
LIBS         += -lv3dfftw3f -lv3dfftw3f_threads

TARGET	= $$qtLibraryTarget(Rivulet)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/Rivulet2/
