
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/hys/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= wrong_area_search_plugin.h \
    sort_swc.h \
    resampling.h \
    find_wrong_area.h \
    my_surf_objs.h \
    local_aligment.h \
    seq_weight.h \
    neuron_tree_align.h \
    swc_utils.h \
    data_training.h \
    get_sample_area.h \
    node.h \
    graph.h
SOURCES	+= wrong_area_search_plugin.cpp \
    sort_swc.cpp \
    resampling.cpp \
    find_wrong_area.cpp \
    my_surf_objs.cpp \
    local_alignment.cpp \
    seq_weight.cpp \
    swc_utils.cpp \
    data_training.cpp \
    get_sample_area.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp




TARGET	= $$qtLibraryTarget(wrong_area_search)
DESTDIR	= $$VAA3DPATH/bin/plugins/wrong_area_search/
