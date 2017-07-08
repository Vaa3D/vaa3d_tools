
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin
INCLUDEPATH += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/global_cmp
INCLUDEPATH += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pointcloud_match
INCLUDEPATH += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pre_processing
INCLUDEPATH += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/tree_matching


HEADERS	+= pattern_search_plugin.h \
    pattern_analysis.h \
    get_subtrees.h \
    trees_retrieve.h \
    my_sort.h
SOURCES	+= pattern_search_plugin.cpp \
    pattern_analysis.cpp \
    get_subtrees.cpp \
    trees_retrieve.cpp \
    my_sort.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/global_cmp/compute_gmi.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/global_cmp/compute_morph.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/global_cmp/neuron_retrieve.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/global_cmp/pre_process.cpp

SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pre_processing/align_axis.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pre_processing/pca1.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pre_processing/resampling.cpp

SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/neuron_tree_align.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/seg_weight.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/swc_utils.cpp

SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pointcloud_match/q_pointcloud_match.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pointcloud_match/q_pointcloud_match_refinematch_affine.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pointcloud_match/q_pointcloud_match_refinematch_manifold.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pointcloud_match/q_pointcloud_match_initialmatch.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/blastneuron_plugin/pointcloud_match/q_pointcloud_match_basic.cpp

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    INCLUDEPATH	+= C:/gnuwin32/include
    INCLUDEPATH	+= ./
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibnewmat
    }
}

unix {
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}

TARGET	= $$qtLibraryTarget(pattern_search)
DESTDIR	= $$VAA3DPATH/bin/plugins/pattern_search/
