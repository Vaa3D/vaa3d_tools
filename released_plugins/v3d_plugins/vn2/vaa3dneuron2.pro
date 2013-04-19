
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3D_DIR = ../../v3d_external
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/common_lib/include
INCLUDEPATH     += app2
INCLUDEPATH     += app1
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/worm_straighten_c

macx {
LIBS += -L$$VAA3D_DIR/v3d_main/common_lib/lib_mac64 -lv3dtiff
LIBS += -L$$VAA3D_DIR/v3d_main/common_lib/src_packages/mylib_tiff -lmylib
}

unix:!macx {
LIBS += -L$$VAA3D_DIR/v3d_main/common_lib/lib -lv3dtiff
LIBS += -L$$VAA3D_DIR/v3d_main/common_lib/src_packages/mylib_tiff -lmylib
}

LIBS += -L$$VAA3D_DIR/v3d_main/jba/c++ 
LIBS += -lm -lv3dtiff -lv3dnewmat


HEADERS	+= vaa3dneuron2_plugin.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_memory.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/imageio_mylib.h
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
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_4dimage.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/imageio_mylib.cpp \
    $$VAA3D_DIR/v3d_main/worm_straighten_c/bdb_minus.cpp \
    $$VAA3D_DIR/v3d_main/worm_straighten_c/mst_prim_c.cpp \
    $$VAA3D_DIR/v3d_main/worm_straighten_c/bfs_1root.cpp \
    $$VAA3D_DIR/v3d_main/graph/dijk.cpp \
    \ #$$VAA3D_DIR/v3d_main/neuron_editing/apo_xforms.cpp \
    \ #$$VAA3D_DIR/v3d_main/neuron_editing/neuron_xforms.cpp \
    $$VAA3D_DIR/v3d_main/neuron_editing/neuron_sim_scores.cpp \
    $$VAA3D_DIR/v3d_main/neuron_editing/v_neuronswc.cpp 



TARGET	= $$qtLibraryTarget(vn2)
DESTDIR	= $$VAA3D_DIR/bin/plugins/neuron_tracing/Vaa3D_Neuron2
