
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= segment_profiling_plugin.h \
    openSWCDialog.h \
    segment_profiling_main.h \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/neuron_tree_align.h \
    ../wrong_area_search/find_wrong_area.h \
    ../wrong_area_search/my_surf_objs.h \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/swc_utils.h \
    get_sub_block.h \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h \
    segment_mean_shift.h \
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_dialog.h

SOURCES	+= segment_profiling_plugin.cpp \
    openSWCDialog.cpp \
    segment_profiling_main.cpp \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/neuron_tree_align.cpp \
    ../wrong_area_search/find_wrong_area.cpp \
    ../wrong_area_search/my_surf_objs.cpp \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/swc_utils.cpp \
    get_sub_block.cpp \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp \
    segment_mean_shift.cpp \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_dialog.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(segment_profiling)
DESTDIR	= $$VAA3DPATH/bin/plugins/segment_profiling/
