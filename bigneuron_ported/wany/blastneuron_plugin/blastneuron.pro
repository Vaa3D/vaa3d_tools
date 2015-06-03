
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/jba/newmat11

HEADERS	+= blastneuron_plugin.h
SOURCES	+= blastneuron_plugin.cpp
SOURCES += global_cmp/batch_compute_main.cpp
SOURCES += global_cmp/compute_gmi.cpp
SOURCES += global_cmp/compute_morph.cpp
SOURCES += global_cmp/global_retrieve_main.cpp
SOURCES += global_cmp/neuron_retrieve.cpp
SOURCES += global_cmp/pre_process.cpp

SOURCES += pre_processing/pre_processing_main.cpp
SOURCES += pre_processing/align_axis.cpp
SOURCES += pre_processing/pca1.cpp
SOURCES += pre_processing/prune_short_branch.cpp
SOURCES += pre_processing/resampling.cpp

SOURCES += tree_matching/local_alignment_main.cpp
SOURCES += tree_matching/my_surf_objs.cpp
SOURCES += tree_matching/neuron_tree_align.cpp
SOURCES += tree_matching/seg_weight.cpp
SOURCES += tree_matching/swc_utils.cpp

SOURCES += pointcloud_match/inverse_projection_main.cpp
SOURCES += pointcloud_match/q_pointcloud_match.cpp
SOURCES += pointcloud_match/q_pointcloud_match_refinematch_affine.cpp
SOURCES += pointcloud_match/q_pointcloud_match_refinematch_manifold.cpp
SOURCES += pointcloud_match/q_pointcloud_match_initialmatch.cpp
SOURCES += pointcloud_match/q_pointcloud_match_basic.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibnewmat
    }
}

unix {
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}
TARGET	= $$qtLibraryTarget(blastneuron)
DESTDIR	= $$VAA3DPATH/bin/plugins/blastneuron/
