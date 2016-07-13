TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/boost/graph
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/
INCLUDEPATH     += ann/include

LIBS            += -Lann/lib -lANN

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
HEADERS += ann/include/ANN/ANN.h
#HEADERS += ann/src/kd_tree.h
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
