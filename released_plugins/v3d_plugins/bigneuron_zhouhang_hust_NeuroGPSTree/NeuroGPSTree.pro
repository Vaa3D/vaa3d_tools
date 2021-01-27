
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH	+= ./eigen3.2.2
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include/eigen_3_3_4

win32 {
LIBS += $$V3DMAINPATH/v3d_main/common_lib/src_packages/Pre-built.2/lib/pthreadVC2.lib
}

unix {
LIBS            += -lpthread  #-lgomp
}

#QMAKE_CXXFLAGS  += -std=c++11

unix:!macx {
    QMAKE_CXXFLAGS  += -std=c++11 -fopenmp
    LIBS            += -lgomp
}

HEADERS	+= NeuroGPSTree_plugin.h \
    Function/binaryfilter.h \
    Function/contourutil.h \
    Function/ineuronio.h \
    Function/ineuronprocessobject.h \
    Function/volumealgo.h \
    Function/Trace/bridgebreaker.h \
    Function/Trace/neurotreecluster.h \
    Function/Trace/tracefilter.h \
    Function/Trace/traceutil.h \
    ngtypes/basetypes.h \
    ngtypes/ineurondataobject.h \
    ngtypes/shape.h \
    ngtypes/soma.h \
    ngtypes/tree.h \
    ngtypes/volume.h \
    ngtypes/volumecreator.h \
    Function/Trace/WeakSWCFilter.h \
    Function/IO/treewriter.h
SOURCES	+= NeuroGPSTree_plugin.cpp \
    Function/binaryfilter.cpp \
    Function/contourutil.cpp \
    Function/volumealgo.cpp \
    Function/Trace/bridgebreaker.cpp \
    Function/Trace/neurotreecluster.cpp \
    Function/Trace/TraceCrossAnalysis.cpp \
    Function/Trace/tracefilter.cpp \
    Function/Trace/traceutil.cpp \
    ngtypes/shape.cpp \
    ngtypes/soma.cpp \
    ngtypes/tree.cpp \
    ngtypes/volume.cpp \
    Function/Trace/WeakSWCFilter.cpp \
    Function/IO/treewriter.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(NeuroGPSTree)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/HUST_NeuroGPSTree/
