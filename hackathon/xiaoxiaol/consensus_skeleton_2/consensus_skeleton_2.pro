TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun

HEADERS	+= consensus_skeleton_plugin.h
HEADERS	+= consensus_skeleton_func.h
HEADERS	+= consensus_skeleton.h
HEADERS	+= kcentroid_cluster.h
HEADERS	+= mst_dij.h
#HEADERS += mst_prim.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h


SOURCES	+= consensus_skeleton_plugin.cpp
SOURCES	+= consensus_skeleton_func.cpp
SOURCES	+= consensus_skeleton.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(consensus_skeleton_2)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/consensus_skeleton_2/
