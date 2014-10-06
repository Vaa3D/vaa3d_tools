
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun

HEADERS	+= consensus_skeleton_plugin.h
HEADERS	+= consensus_skeleton_func.h

SOURCES	= consensus_skeleton_plugin.cpp
SOURCES	+= consensus_skeleton_func.cpp
SOURCES	+= consensus_skeleton.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(consensus_skeleton)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/consensus_skeleton_via_clustering/
