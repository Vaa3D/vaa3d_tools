
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH     += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += app2
INCLUDEPATH     += app1
INCLUDEPATH     += $$V3DMAINPATH/neuron_editing
INCLUDEPATH     += $$V3DMAINPATH/worm_straighten_c

macx {
LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
LIBS += -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
}

unix:!macx {
LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
LIBS += -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
}

win32 {
LIBS += -L$$V3DMAINPATH/common_lib/winlib -ltiff
}

LIBS += -L$$V3DMAINPATH/jba/c++ 
LIBS += -lm -lv3dnewmat


HEADERS	+= vaa3dneuron2_plugin.h
HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h
HEADERS += vn_imgpreprocess.h
HEADERS += vn.h
HEADERS += vn_app2.h
HEADERS += vn_app1.h
HEADERS += app1/v3dneuron_gd_tracing.h
HEADERS += app1/gd.h
HEADERS += app2/fastmarching_tree.h
HEADERS += app2/hierarchy_prune.h
HEADERS += app2/fastmarching_dt.h


SOURCES += vn_imgpreprocess.cpp
SOURCES += app2_connector.cpp
SOURCES += app1_connector.cpp
SOURCES += vaa3dneuron2_plugin.cpp
SOURCES += swc_convert.cpp
SOURCES += app2/my_surf_objs.cpp
SOURCES += app1/v3dneuron_gd_tracing.cpp
SOURCES += app1/gd.cpp
SOURCES += app1/calculate_cover_scores.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.cpp \
    $$V3DMAINPATH/worm_straighten_c/bdb_minus.cpp \
    $$V3DMAINPATH/worm_straighten_c/mst_prim_c.cpp \
    $$V3DMAINPATH/worm_straighten_c/bfs_1root.cpp \
    $$V3DMAINPATH/graph/dijk.cpp \
    \ #$$V3DMAINPATH/neuron_editing/apo_xforms.cpp \
    \ #$$V3DMAINPATH/neuron_editing/neuron_xforms.cpp \
    $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp \
    $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp 



TARGET	= $$qtLibraryTarget(vn2)
#DESTDIR = ../../v3d/plugins/neuron_tracing/Vaa3D_Neuron2
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_tracing/Vaa3D_Neuron2

