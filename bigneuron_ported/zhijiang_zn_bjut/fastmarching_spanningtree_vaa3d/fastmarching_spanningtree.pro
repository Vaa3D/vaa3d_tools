
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
#VAA3DPATH = ../Vaa3D
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

#include the headers used in the project
HEADERS	+= fastmarching_spanningtree_plugin.h
HEADERS += \
    myplugin_func.h \
    global.h \
    ../../APP2_ported/heap.h \
    tree.h \
    graph.h \
    node.h

SOURCES	+= fastmarching_spanningtree_plugin.cpp
SOURCES	+= myplugin_func.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(fastmarching_spanningtree)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_tracing/BJUT_fastmarching_spanningtree/
#DESTDIR = C:/Users/Administrator/Desktop/vaa3d_win7_32bit_v2.707/plugins/myplugin/myplugin/
