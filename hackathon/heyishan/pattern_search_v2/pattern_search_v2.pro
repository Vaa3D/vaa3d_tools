
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
HEADERS	+= pattern_search_v2_plugin.h \
    pattern_analysis.h \
    get_subtrees.h \
    ../pattern_search/trees_retrieve.h \
    ../pattern_search/my_sort.h
SOURCES	+= pattern_search_v2_plugin.cpp \
    pattern_analysis.cpp \
    get_subtrees.cpp \
    ../pattern_search/trees_retrieve.cpp \
    ../pattern_search/my_sort.cpp
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
TARGET	= $$qtLibraryTarget(pattern_search_v2)
DESTDIR	= $$VAA3DPATH/bin/plugins/vr_neuron_search/vr_nmotif_bjut/
