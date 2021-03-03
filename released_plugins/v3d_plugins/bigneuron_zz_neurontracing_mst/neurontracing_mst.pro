
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d
INCLUDEPATH     += $$V3DMAINPATH/neuron_editing
INCLUDEPATH     += $$V3DMAINPATH/worm_straighten_c
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += $$V3DMAINPATH/cellseg
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
}


HEADERS	+= neurontracing_mst_plugin.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h \
    $$V3DMAINPATH/basic_c_fun/basic_memory.h \
    $$V3DMAINPATH/basic_c_fun/stackutil.h\
    $$V3DMAINPATH/basic_c_fun/mg_image_lib.h\
    $$V3DMAINPATH/basic_c_fun/img_definition.h \
    $$V3DMAINPATH/basic_c_fun/volimg_proc_declare.h \
    $$V3DMAINPATH/basic_c_fun/volimg_proc.h \
    $$V3DMAINPATH/basic_c_fun/v3d_message.h \
    $$V3DMAINPATH/basic_c_fun/color_xyz.h \
    $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h \
    $$V3DMAINPATH/basic_c_fun/basic_landmark.h \
    $$V3DMAINPATH/graph/graph.h \
    $$V3DMAINPATH/graph/graph_basic.h \
    $$V3DMAINPATH/graph/dijk.h \
    $$V3DMAINPATH/worm_straighten_c/bdb_minus.h \
    $$V3DMAINPATH/worm_straighten_c/mst_prim_c.h \
    $$V3DMAINPATH/worm_straighten_c/bfs.h \
    $$V3DMAINPATH/worm_straighten_c/spline_cubic.h \
    $$V3DMAINPATH/neuron_editing/apo_xforms.h \
    $$V3DMAINPATH/neuron_editing/neuron_xforms.h \
    $$V3DMAINPATH/neuron_editing/neuron_sim_scores.h \
    $$V3DMAINPATH/neuron_editing/v_neuronswc.h \
    $$V3DMAINPATH/cellseg/template_matching_seg.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/FL_bwdist.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h

SOURCES	+= neurontracing_mst_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp \
    $$V3DMAINPATH/basic_c_fun/basic_memory.cpp \
    $$V3DMAINPATH/basic_c_fun/stackutil.cpp\
    $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp\
    $$V3DMAINPATH/graph/dijk.cpp \
    $$V3DMAINPATH/worm_straighten_c/bdb_minus.cpp \
    $$V3DMAINPATH/worm_straighten_c/mst_prim_c.cpp \
    $$V3DMAINPATH/worm_straighten_c/bfs_1root.cpp \
    $$V3DMAINPATH/worm_straighten_c/spline_cubic.cpp \
    $$V3DMAINPATH/neuron_editing/apo_xforms.cpp \
    $$V3DMAINPATH/neuron_editing/neuron_xforms.cpp \
    $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp \
    $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp \
    $$V3DMAINPATH/cellseg/template_matching_seg.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp \
    $$V3DMAINPATH/neuron_editing/global_feature_compute.cpp

TARGET	= $$qtLibraryTarget(neurontracing_mst)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_tracing/MST_tracing/
