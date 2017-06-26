
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =../../../../v3d_external/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= region_match_plugin.h \
    match_swc.h \
    neuron_utilities/sort_swc.h \
    neuron_utilities/resample_swc.h \
    make_consensus.h \
    get_substructure.h \
    blastneuron/compute_morph.h \
    blastneuron/compute_gmi.h \
    blastneuron/pre_process.h \
    blastneuron/neuron_retrieve.h \
    blastneuron/prune_short_branch.h \
    blastneuron/sim_measure.h
SOURCES	+= region_match_plugin.cpp \
    match_swc.cpp \
    neuron_utilities/sort_swc.cpp \
    neuron_utilities/resample_swc.cpp \
    make_consensus.cpp \
    get_substructure.cpp \
    blastneuron/compute_morph.cpp \
    blastneuron/compute_gmi.cpp \
    blastneuron/pre_process.cpp \
    blastneuron/neuron_retrieve.cpp \
    prune_short_branch.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(region_match)
DESTDIR	= $$VAA3DPATH/bin/plugins/vr_neuron_search/vr_nmotif_bjut/

