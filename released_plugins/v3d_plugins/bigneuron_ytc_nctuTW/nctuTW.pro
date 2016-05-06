
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/common_lib/include


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

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(nctuTW)
DESTDIR	= $$VAA3DPATH/../bin/plugins/bigneuronhackathon/nctuTW/

FORMS += \
    QDlgPara.ui


