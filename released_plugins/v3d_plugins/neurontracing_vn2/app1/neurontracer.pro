# #####################################################################
# 2010-05-24
# v3dneuron_tracing.pro
# by Hanchuan Peng
# separate from main v3d program as a command line program
#
# 2010-Dec-21: copy dijk_bgl.cpp as gd.cpp here and make further refinement
# 2012-April-2: change a few folders
# ######################################################################

TEMPLATE = app
TARGET += 
DEPENDPATH += . 
INCLUDEPATH += . 
INCLUDEPATH += ../app2
SOURCES += ../app2/my_surf_objs.cpp

# cross-OS-platform, cross-Qt-version, 
QT_DIR = $$dirname(QMAKE_QMAKE)/..  # cross-Qt-version
LOCAL_DIR = ../../../../../v3d_external/v3d_main 				# unix-liked platform: macx, unix, win32-msys-mingw
INCLUDEPATH += $$LOCAL_DIR/common_lib/include

MINGW_DIR = /mingw # platform: win32-msys-mingw
win32 { # platform: win32-command-mingw
	MINGW_DIR = c:/mingw
	LOCAL_DIR = ../common_lib/      # c:/msys/local
	CONFIG = $$unique(CONFIG)
	CONFIG -= debug # for Qt-win32 which only has release install(no debug)
	CONFIG += console
LIBS += -L$$MINGW_DIR/lib \
	-L$$LOCAL_DIR/lib_win32
}

message(CONFIG=$$unique(CONFIG))
macx {
#     CONFIG += x86_64
     CONFIG -= app_bundle
}

INCLUDEPATH += $$LOCAL_DIR/basic_c_fun 
INCLUDEPATH += $$LOCAL_DIR/common_lib/include 
INCLUDEPATH += $$LOCAL_DIR/neuron_editing
INCLUDEPATH += $$LOCAL_DIR/worm_straighten_c
INCLUDEPATH += $$LOCAL_DIR/cellseg

LIBS += -L$$LOCAL_DIR/lib


SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
include($$SHARED_FOLDER/shared.pri)
INCLUDEPATH += $$SHARED_FOLDER
LIBS += -L$$SHARED_FOLDER \
	-L$$SHARED_FOLDER/release # for Qt-win32 which only has release install(no debug)

CONFIG += warn_off  # only work for complier

V3DMAINDIR = ../../../v3d_external/v3d_main

# Input
HEADERS += $$V3DMAINDIR/basic_c_fun/mg_utilities.h \
    $$V3DMAINDIR/basic_c_fun/mg_image_lib.h \
    $$V3DMAINDIR/basic_c_fun/basic_memory.h \
    $$V3DMAINDIR/basic_c_fun/stackutil.h \
    $$V3DMAINDIR/basic_c_fun/img_definition.h \
    $$V3DMAINDIR/basic_c_fun/volimg_proc_declare.h \
    $$V3DMAINDIR/basic_c_fun/volimg_proc.h \
    $$V3DMAINDIR/basic_c_fun/v3d_message.h \
    $$V3DMAINDIR/basic_c_fun/color_xyz.h \
    $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h \
    $$V3DMAINDIR/basic_c_fun/basic_4dimage.h \
    $$V3DMAINDIR/basic_c_fun/basic_landmark.h \
    $$V3DMAINDIR/graph/graph.h \
    $$V3DMAINDIR/graph/graph_basic.h \
    $$V3DMAINDIR/graph/dijk.h \
    $$V3DMAINDIR/worm_straighten_c/bdb_minus.h \
    $$V3DMAINDIR/worm_straighten_c/mst_prim_c.h \
    $$V3DMAINDIR/worm_straighten_c/bfs.h \
    $$V3DMAINDIR/worm_straighten_c/spline_cubic.h \
    $$V3DMAINDIR/neuron_editing/apo_xforms.h \
    $$V3DMAINDIR/neuron_editing/neuron_xforms.h \
    $$V3DMAINDIR/neuron_editing/neuron_sim_scores.h \
    $$V3DMAINDIR/neuron_editing/v_neuronswc.h \
    $$V3DMAINDIR/cellseg/template_matching_seg.h \ 
    FL_bwdist.h \
    gd.h \
    tip_detection.h \
    autoseed.h \
    v3dneuron_gd_tracing.h
#    $$V3DMAINDIR/v3d/dialog_curve_trace_para.h 
unix:HEADERS += $$V3DMAINDIR/basic_c_fun/imageio_mylib.h

SOURCES += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp \
    $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp \
    $$V3DMAINDIR/basic_c_fun/stackutil.cpp \
    $$V3DMAINDIR/basic_c_fun/basic_memory.cpp \
    $$V3DMAINDIR/basic_c_fun/v3d_message.cpp \
    $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp \
    $$V3DMAINDIR/basic_c_fun/basic_4dimage.cpp \
    $$V3DMAINDIR/graph/dijk.cpp \
    $$V3DMAINDIR/worm_straighten_c/bdb_minus.cpp \
    $$V3DMAINDIR/worm_straighten_c/mst_prim_c.cpp \
    $$V3DMAINDIR/worm_straighten_c/bfs_1root.cpp \
    $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp \
    $$V3DMAINDIR/neuron_editing/apo_xforms.cpp \
    $$V3DMAINDIR/neuron_editing/neuron_xforms.cpp \
    $$V3DMAINDIR/neuron_editing/neuron_sim_scores.cpp \
    $$V3DMAINDIR/neuron_editing/v_neuronswc.cpp \
    $$V3DMAINDIR/cellseg/template_matching_seg.cpp \
    gd.cpp \
    tip_detection.cpp \
    autoseed.cpp \
    calculate_cover_scores.cpp \
	v3dneuron_gd_tracing.cpp \
	main_v3dneuron_tracing.cpp
unix:SOURCES += $$V3DMAINDIR/basic_c_fun/imageio_mylib.cpp

FORMS += $$V3DMAINDIR/v3d/dialog_curve_trace.ui 

RESOURCES +=
#QT += opengl

LIBS += -L$$V3DMAINDIR/jba/c++ -L$$V3DMAINDIR/common_lib/lib
LIBS += -lm -lv3dtiff -lv3dnewmat

unix:LIBS += -L$$V3DMAINDIR/common_lib/src_packages/mylib_tiff -lmylib
    
#win32:LIBS += -lm -lv3dtiff 
#    -lglut32 # win32-mingw, on unix link libglut.a

INCLUDEPATH = $$unique(INCLUDEPATH)
LIBS = $$unique(LIBS)
# CONFIG = $$unique(CONFIG) # this only DOESN'T work on macx, very strange, by RZC 20080923
message(CONFIG=$$CONFIG)
