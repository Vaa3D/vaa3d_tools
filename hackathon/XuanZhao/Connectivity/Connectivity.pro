
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app1
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/v3d_main/worm_straighten_c

unix {
LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}

HEADERS	+= Connectivity_plugin.h \
    connectivity_func.h \
    kmeans.h

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/volimg_proc.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.h
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h

SOURCES	+= Connectivity_plugin.cpp \
    connectivity_func.cpp \
    kmeans.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp

SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp \
           $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
           $$VAA3DPATH/v3d_main/worm_straighten_c/bdb_minus.cpp \
           $$VAA3DPATH/v3d_main/worm_straighten_c/mst_prim_c.cpp \
           $$VAA3DPATH/v3d_main/worm_straighten_c/bfs_1root.cpp \
           $$VAA3DPATH/v3d_main/graph/dijk.cpp \
           $$VAA3DPATH/v3d_main//neuron_editing/neuron_sim_scores.cpp \
           $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp \
           $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp


TARGET	= $$qtLibraryTarget(Connectivity)
DESTDIR	= $$VAA3DPATH/bin/plugins/Connectivity/
