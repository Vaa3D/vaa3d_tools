
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ../libs/ITKlibs_MAC
}
else{
    ITKLIBPATH = ../libs/ITKlibs_Linux
    SOURCES = ../libs/ITK_include/itkLightProcessObject.cxx
}

VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/v3d_main/worm_straighten_c
INCLUDEPATH     += $$VAA3DPATH/v3d_main/cellseg


INCLUDEPATH += ../libs/liblbfgs-1.10/include
INCLUDEPATH += ../libs/sqb_0.1/include
INCLUDEPATH += ../libs/libconfig-1.4.9/lib
INCLUDEPATH += ../libs/ITK_include
INCLUDEPATH += ../libs/regression
INCLUDEPATH += ../libs/sqb_0.1/src/MatrixSQB
INCLUDEPATH += ../libs/boost_1_58_0 # download and unzip it (not in svn because it is big)

LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib

LIBS += -L"../libs/liblbfgs-1.10/lib"
LIBS += -llbfgs

LIBS += -L"../libs/libconfig-1.4.9/lib/.libs"
LIBS += -lconfig++

HEADERS += ../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h
HEADERS += ../libs/regression/sep_conv.h
HEADERS += ../libs/regression/util.h
HEADERS += ../libs/regression/sampling.h
HEADERS += ../libs/regression/regressor.h

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.h\
    $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.h\
    $$VAA3DPATH/v3d_main/basic_c_fun/img_definition.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/volimg_proc_declare.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/volimg_proc.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/color_xyz.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_landmark.h \
    $$VAA3DPATH/v3d_main/graph/graph.h \
    $$VAA3DPATH/v3d_main/graph/graph_basic.h \
    $$VAA3DPATH/v3d_main/graph/dijk.h \
    $$VAA3DPATH/v3d_main/worm_straighten_c/bdb_minus.h \
    $$VAA3DPATH/v3d_main/worm_straighten_c/mst_prim_c.h \
    $$VAA3DPATH/v3d_main/worm_straighten_c/bfs.h \
    $$VAA3DPATH/v3d_main/worm_straighten_c/spline_cubic.h \
    $$VAA3DPATH/v3d_main/neuron_editing/apo_xforms.h \
    $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.h \
    $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.h \
    $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.h \
    $$VAA3DPATH/v3d_main/cellseg/template_matching_seg.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/FL_bwdist.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h
HEADERS += mst_functions/neurontracing_mst_plugin.h

HEADERS	+= RegMST_plugin.h


SOURCES	+= ../libs/regression/sep_conv.cpp
SOURCES	+= ../libs/regression/sampling.cpp
SOURCES	+= ../libs/regression/regressor.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp \
    $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp\
    $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp\
    $$VAA3DPATH/v3d_main/graph/dijk.cpp \
    $$VAA3DPATH/v3d_main/worm_straighten_c/bdb_minus.cpp \
    $$VAA3DPATH/v3d_main/worm_straighten_c/mst_prim_c.cpp \
    $$VAA3DPATH/v3d_main/worm_straighten_c/bfs_1root.cpp \
    $$VAA3DPATH/v3d_main/worm_straighten_c/spline_cubic.cpp \
    $$VAA3DPATH/v3d_main/neuron_editing/apo_xforms.cpp \
    $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.cpp \
    $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp \
    $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp \
    $$VAA3DPATH/v3d_main/cellseg/template_matching_seg.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES	+= mst_functions/neurontracing_mst_plugin.cpp

SOURCES	+= RegMST_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

QMAKE_CXXFLAGS += -DLIBCONFIGXX_VER_REVISION
QMAKE_CXXFLAGS += -fPIC
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

TARGET	= $$qtLibraryTarget(RegMST)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/RegMST/
