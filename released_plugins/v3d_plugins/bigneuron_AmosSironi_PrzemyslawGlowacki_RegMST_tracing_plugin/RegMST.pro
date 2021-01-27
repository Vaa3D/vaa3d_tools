
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/ITKlibs_MAC
}
else{
    ITKLIBPATH = ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/ITKlibs_Linux
    SOURCES = ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/ITK_include/itkLightProcessObject.cxx
}

V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

INCLUDEPATH     += $$V3DMAINPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/worm_straighten_c
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/cellseg


#INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/liblbfgs-1.10/include
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/liblbfgs-1.10_include
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/sqb_0.1/include
#INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/libconfig-1.4.9/lib
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/libconfig-1.4.9_include
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/libconfig-1.4.9/lib
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/ITK_include
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/sqb_0.1/src/MatrixSQB
INCLUDEPATH += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/boost_1_58_0
INCLUDEPATH += $$V3DMAINPATH/v3d_main/common_lib/include

LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib

#LIBS += -L"../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/liblbfgs-1.10/lib"
LIBS += -L"../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/liblbfgs-1.10_Linux/lib"
LIBS += -llbfgs

#LIBS += -L"../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/libconfig-1.4.9/lib/.libs"
LIBS += -L"../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/libconfig-1.4.9_Linux"
LIBS += -lconfig++

HEADERS += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h
HEADERS += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/sep_conv.h
HEADERS += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/util.h
HEADERS += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/sampling.h
HEADERS += ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/regressor.h

HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/basic_memory.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.h\
    $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.h\
    $$V3DMAINPATH/v3d_main/basic_c_fun/img_definition.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/volimg_proc_declare.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/volimg_proc.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/color_xyz.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.h \
    $$V3DMAINPATH/v3d_main/basic_c_fun/basic_landmark.h \
    $$V3DMAINPATH/v3d_main/graph/graph.h \
    $$V3DMAINPATH/v3d_main/graph/graph_basic.h \
    $$V3DMAINPATH/v3d_main/graph/dijk.h \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/bdb_minus.h \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/mst_prim_c.h \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/bfs.h \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/spline_cubic.h \
    $$V3DMAINPATH/v3d_main/neuron_editing/apo_xforms.h \
    $$V3DMAINPATH/v3d_main/neuron_editing/neuron_xforms.h \
    $$V3DMAINPATH/v3d_main/neuron_editing/neuron_sim_scores.h \
    $$V3DMAINPATH/v3d_main/neuron_editing/v_neuronswc.h \
    $$V3DMAINPATH/v3d_main/cellseg/template_matching_seg.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/FL_bwdist.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h
HEADERS += mst_functions/neurontracing_mst_plugin.h

HEADERS	+= RegMST_plugin.h


SOURCES	+= ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/sep_conv.cpp
SOURCES	+= ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/sampling.cpp
SOURCES	+= ../bigneuron_AmosSironi_PrzemyslawGlowacki_SQBTree_plugin/libs/regression/regressor.cpp

SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.cpp \
    $$V3DMAINPATH/v3d_main/basic_c_fun/basic_memory.cpp \
    $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.cpp\
    $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.cpp\
    $$V3DMAINPATH/v3d_main/graph/dijk.cpp \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/bdb_minus.cpp \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/mst_prim_c.cpp \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/bfs_1root.cpp \
    $$V3DMAINPATH/v3d_main/worm_straighten_c/spline_cubic.cpp \
    $$V3DMAINPATH/v3d_main/neuron_editing/apo_xforms.cpp \
    $$V3DMAINPATH/v3d_main/neuron_editing/neuron_xforms.cpp \
    $$V3DMAINPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp \
    $$V3DMAINPATH/v3d_main/neuron_editing/v_neuronswc.cpp \
    $$V3DMAINPATH/v3d_main/cellseg/template_matching_seg.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES	+= mst_functions/neurontracing_mst_plugin.cpp

SOURCES	+= RegMST_plugin.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

QMAKE_CXXFLAGS += -DLIBCONFIGXX_VER_REVISION
QMAKE_CXXFLAGS += -fPIC
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

TARGET	= $$qtLibraryTarget(RegMST)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/RegMST/
