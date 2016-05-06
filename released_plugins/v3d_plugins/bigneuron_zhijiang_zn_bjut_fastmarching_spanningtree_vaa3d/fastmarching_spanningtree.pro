
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun 
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include

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

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(fastmarching_spanningtree)
DESTDIR = $$V3DMAINPATH/../bin/plugins/neuron_tracing/BJUT_fastmarching_spanningtree
