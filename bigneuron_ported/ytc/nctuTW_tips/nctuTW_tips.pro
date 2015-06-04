
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/cellseg
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2

macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$VAA3DPATH/common_lib/winlib64 -llibtiff
    LIBS     += -L$$VAA3DPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$VAA3DPATH/common_lib/winlib -llibtiff
    LIBS     += -L$$VAA3DPATH/common_lib/winlib -llibnewmat
    }
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/common_lib/lib -ltiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
}


HEADERS	+= nctuTW_tips_plugin.h\
    Dijkstra.h \
    DistanceTransTwo.h \
    Geo.h \
    Graph_AdjacencyList3D.h \
    Image3D.h \
    IPT.h \
    LineSeg3D.h \
    main.h \
    Point3D.h \
    UnionFind.h \
    findingtips.h

HEADERS += $$VAA3DPATH/basic_c_fun/mg_utilities.h \
    $$VAA3DPATH/basic_c_fun/basic_memory.h \
    $$VAA3DPATH/basic_c_fun/stackutil.h\
    $$VAA3DPATH/basic_c_fun/mg_image_lib.h\
    $$VAA3DPATH/basic_c_fun/img_definition.h \
    $$VAA3DPATH/basic_c_fun/volimg_proc_declare.h \
    $$VAA3DPATH/basic_c_fun/volimg_proc.h \
    $$VAA3DPATH/basic_c_fun/v3d_message.h \
    $$VAA3DPATH/basic_c_fun/color_xyz.h \
    $$VAA3DPATH/basic_c_fun/basic_surf_objs.h \
    $$VAA3DPATH/basic_c_fun/basic_landmark.h \
    $$VAA3DPATH/graph/graph.h \
    $$VAA3DPATH/graph/graph_basic.h \
    $$VAA3DPATH/graph/dijk.h \
    $$VAA3DPATH/worm_straighten_c/bdb_minus.h \
    $$VAA3DPATH/worm_straighten_c/mst_prim_c.h \
    $$VAA3DPATH/worm_straighten_c/bfs.h \
    $$VAA3DPATH/worm_straighten_c/spline_cubic.h \
    $$VAA3DPATH/neuron_editing/apo_xforms.h \
    $$VAA3DPATH/neuron_editing/neuron_xforms.h \
    $$VAA3DPATH/neuron_editing/neuron_sim_scores.h \
    $$VAA3DPATH/neuron_editing/v_neuronswc.h \
    $$VAA3DPATH/cellseg/template_matching_seg.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/FL_bwdist.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h
SOURCES	+= nctuTW_tips_plugin.cpp\
    Dijkstra.cpp \
    DistanceTransTwo.cpp \
    Geo.cpp \
    Graph_AdjacencyList3D.cpp \
    Image3D.cpp \
    IPT.cpp \
    LineSeg3D.cpp \
    Point3D.cpp \
    main.cpp


SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp \
    $$VAA3DPATH/basic_c_fun/basic_memory.cpp \
    $$VAA3DPATH/basic_c_fun/stackutil.cpp\
    $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp\
    $$VAA3DPATH/graph/dijk.cpp \
    $$VAA3DPATH/worm_straighten_c/bdb_minus.cpp \
    $$VAA3DPATH/worm_straighten_c/mst_prim_c.cpp \
    $$VAA3DPATH/worm_straighten_c/bfs_1root.cpp \
    $$VAA3DPATH/worm_straighten_c/spline_cubic.cpp \
    $$VAA3DPATH/neuron_editing/apo_xforms.cpp \
    $$VAA3DPATH/neuron_editing/neuron_xforms.cpp \
    $$VAA3DPATH/neuron_editing/neuron_sim_scores.cpp \
    $$VAA3DPATH/neuron_editing/v_neuronswc.cpp \
    $$VAA3DPATH/cellseg/template_matching_seg.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp \

TARGET	= $$qtLibraryTarget(nctuTW_GD)
DESTDIR	= $$VAA3DPATH/../bin/plugins/bigneuronhackathon/nctuTW_GD/

