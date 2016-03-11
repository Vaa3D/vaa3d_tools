TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing

HEADERS	+= consensus_skeleton_plugin.h \
    median_swc.h \
    point3d_util.h \
    sort_eswc.h \
    dark_pruning.h \
    neuron_matching_distance.h
HEADERS	+= consensus_skeleton_func.h
HEADERS	+= consensus_skeleton.h
HEADERS	+= kcentroid_cluster.h
HEADERS	+= mst_dij.h
HEADERS += mst_prim.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.h
HEADERS += $$VAA3DMAINPATH/neuron_editing/neuron_sim_scores.h
HEADERS	+= $$VAA3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/resample_swc/resampling.h
SOURCES	+= point3d_util.cpp \
    dark_pruning.cpp \
    neuron_matching_distance.cpp
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.h

SOURCES	+= consensus_skeleton_plugin.cpp \
    median_swc.cpp
SOURCES	+= consensus_skeleton_func.cpp
SOURCES	+= consensus_skeleton.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(consensus_swc)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/consensus_swc/
