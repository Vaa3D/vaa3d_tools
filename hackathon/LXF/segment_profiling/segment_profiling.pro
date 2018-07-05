
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/jba/newmat11



macx{
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}

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
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_dialog.h \
    ../../../../v3d_external/v3d_main/neuron_editing/neuron_format_converter.h \
    ../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.h \
    ../../KLS/neuron_image_snr/profile_swc.h \
    profile_snr.h \
    ../../KLS/neuron_image_snr/compute_tubularity.h \
    ../../../../v3d_external/v3d_main/jba/newmat11/newmatio.h \
    ../../../../v3d_external/v3d_main/jba/newmat11/newmatap.h

SOURCES	+= segment_profiling_plugin.cpp \
    openSWCDialog.cpp \
    segment_profiling_main.cpp \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/neuron_tree_align.cpp \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/seg_weight.cpp \
    ../wrong_area_search/find_wrong_area.cpp \
    ../wrong_area_search/my_surf_objs.cpp \
    ../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/swc_utils.cpp \
    get_sub_block.cpp \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp \
    segment_mean_shift.cpp \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_dialog.cpp \
    ../../../../v3d_external/v3d_main/neuron_editing/neuron_format_converter.cpp \
    ../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.cpp \
    ../../KLS/neuron_image_snr/profile_swc.cpp \
    profile_snr.cpp \
    ../../KLS/neuron_image_snr/compute_tubularity.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(segment_profiling)
DESTDIR	= $$VAA3DPATH/bin/plugins/segment_profiling/
