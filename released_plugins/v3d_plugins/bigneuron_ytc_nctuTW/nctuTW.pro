
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d


HEADERS	+= nctuTW_plugin.h \
    Dijkstra.h \
    DistanceTransTwo.h \
    Geo.h \
    Graph_AdjacencyList3D.h \
    Image3D.h \
    IPT.h \
    LineSeg3D.h \
    main.h \
    Point3D.h \
    UnionFind.h \
    neurontracing.h \
    QDlgPara.h
SOURCES	+= nctuTW_plugin.cpp \
    Dijkstra.cpp \
    DistanceTransTwo.cpp \
    Geo.cpp \
    Graph_AdjacencyList3D.cpp \
    Image3D.cpp \
    IPT.cpp \
    LineSeg3D.cpp \
    Point3D.cpp \
    main.cpp \
    QDlgPara.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(nctuTW)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_tracing/nctuTW/

FORMS += \
    QDlgPara.ui


