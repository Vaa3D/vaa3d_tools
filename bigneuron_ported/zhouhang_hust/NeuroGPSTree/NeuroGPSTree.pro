
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH	+= ./eigen3.2.2
LIBS            += -lpthread  #-lgomp

#QMAKE_CXXFLAGS  += -std=c++11

unix:!macx {
    QMAKE_CXXFLAGS  += -fopenmp
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
    ngtypes/volumecreator.h
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
    ngtypes/volume.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(NeuroGPSTree)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/NeuroGPSTree/
