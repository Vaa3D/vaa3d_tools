
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2
#INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/swc_to_maskimage
#INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/v3d_main/worm_straighten_c
INCLUDEPATH     += $$VAA3DPATH/v3d_main/cellseg
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += ../../zhi/IVSCC_sort_swc

unix {
LIBS += -L$$VAA3DPATH/v3d_main/jba/c++
LIBS += -lv3dnewmat
}


HEADERS	+= compare_swc_plugin.h \
           n_class.h \
    $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc/sort_swc.h\
    $$VAA3DPATH/released_plugins_more/v3d_plugins/swc_to_maskimage/filter_dialog.h


SOURCES	+= compare_swc_plugin.cpp \
           n_class.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \


SOURCES	+=  $$VAA3DPATH/v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES	+=  $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/swc_to_maskimage/filter_dialog.cpp


TARGET	= $$qtLibraryTarget(compare_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/compare_swc/
#DESTDIR	= /home/penglab/PBserver/tmp/ding/vaa3d/plugins/compare_swc
