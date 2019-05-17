
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/V3D/v3d_external
V3DMAINPATH = D:/V3D/v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$V3DMAINPATH/worm_straighten_c
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += app1

unix {
LIBS += -L$$V3DMAINPATH/jba/c++
LIBS += -lv3dnewmat
}
win32 {
LIBS += -L$$V3DMAINPATH/common_lib/winlib64
LIBS += -llibnewmat
}
HEADERS	+= SwcProcess_plugin.h \
    tipdetector.h \
    my_surf_objs.h \
    neurontreepruneorgraft.h \
    neuron_tools.h \
    ray_shooting.h \
    app1/gd.h \
    app1/v3dneuron_gd_tracing.h \
    app1/marker_radius.h \
    app1/sort_swc.h \
    mydialog.h \
    scoreinput.h
SOURCES	+= SwcProcess_plugin.cpp \
    tipdetector.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    my_surf_objs.cpp \
    neurontreepruneorgraft.cpp \
    app1/calculate_cover_scores.cpp \
    app1/gd.cpp \
    app1/v3dneuron_gd_tracing.cpp \
    neuron_tools.cpp \
    ray_shooting.cpp \
    mydialog.cpp \
    scoreinput.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$V3DMAINPATH/worm_straighten_c/bdb_minus.cpp \
    $$V3DMAINPATH/worm_straighten_c/mst_prim_c.cpp \
    $$V3DMAINPATH/worm_straighten_c/bfs_1root.cpp \
    $$V3DMAINPATH/graph/dijk.cpp \
    $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp \
    $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES +=$$V3DMAINPATH/../released_plugins_more/v3d_plugins/blastneuron_plugin/global_cmp/compute_morph.cpp

TARGET	= $$qtLibraryTarget(SwcProcess)
DESTDIR	= $$VAA3DPATH/bin/plugins/SwcProcess/
