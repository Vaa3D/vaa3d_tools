TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

#ann-config aspects
PRE_TARGETDEPS = ann/lib/libANN.a
INCLUDEPATH     += ann/include
HEADERS += ann/include/ANN/ANN.h
HEADERS += ann/src/bd_tree.h \
    ann/src/kd_fix_rad_search.h \
    ann/src/kd_pr_search.h \
    ann/src/kd_search.h \
    ann/src/kd_split.h \
    ann/src/kd_tree.h \
    ann/src/kd_util.h \
    ann/src/pr_queue.h
SOURCES += ann/src/ANN.cpp \
    ann/src/bd_fix_rad_search.cpp \
    ann/src/bd_pr_search.cpp \
    ann/src/bd_search.cpp \
    ann/src/bd_tree.cpp \
    ann/src/brute.cpp \
    ann/src/kd_dump.cpp \
    ann/src/kd_fix_rad_search.cpp \
    ann/src/kd_pr_search.cpp \
    ann/src/kd_search.cpp \
    ann/src/kd_split.cpp \
    ann/src/kd_tree.cpp \
    ann/src/kd_util.cpp \
    ann/src/perf.cpp

LIBS            += ann/lib/libANN.a


ANN.target = ann/lib/libANN.a
ANN.commands = mkdir ann/lib && cd ann && make linux-g++
ANN.depends = ann/Makefile
QMAKE_EXTRA_TARGETS += ANN

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/boost/graph
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/

HEADERS	+= consensus_skeleton_plugin.h \
    median_swc.h \
    point3d_util.h \
    sort_eswc.h \
    dark_pruning.h \
    neuron_matching_distance.h \
    resample_swc.h
HEADERS	+= consensus_skeleton_func.h
HEADERS	+= consensus_skeleton.h
HEADERS	+= kcentroid_cluster.h
HEADERS += mst_boost_prim.h
HEADERS += converge_trees.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$VAA3DMAINPATH/neuron_editing/neuron_sim_scores.h
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/prim_minimum_spanning_tree.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/kruskal_min_spanning_tree.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/adjacency_list.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/config.hpp

#HEADERS += ann/src/kd_search.h

SOURCES	+= point3d_util.cpp \
    dark_pruning.cpp \
    neuron_matching_distance.cpp \
    resample_swc.cpp
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.h

SOURCES	+= consensus_skeleton_plugin.cpp \
    median_swc.cpp
SOURCES	+= consensus_skeleton_func.cpp
SOURCES	+= consensus_skeleton.cpp
SOURCES	+= sort_eswc.cpp
SOURCES += mst_boost_prim.cpp
SOURCES += converge_trees.cpp

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
#SOURCES += ann/src/ann.cpp
#SOURCES += ann/src/kd_tree.cpp
#SOURCES += ann/src/kd_search.cpp

TARGET	= $$qtLibraryTarget(consensus_swc)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/consensus_swc/
