
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/include
HEADERS += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/include/ANN/ANN.h
HEADERS += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/bd_tree.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_fix_rad_search.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_pr_search.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_search.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_split.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_tree.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_util.h \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/pr_queue.h
SOURCES += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/ANN.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/bd_fix_rad_search.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/bd_pr_search.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/bd_search.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/bd_tree.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/brute.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_dump.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_fix_rad_search.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_pr_search.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_search.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_split.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_tree.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/kd_util.cpp \
    $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/src/perf.cpp

unix:!macx{
    PRE_TARGETDEPS = $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/libANN.a
    LIBS            += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/libANN.a
    ANN.target = $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/libANN.a
    ANN.commands = cd $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann && make linux-g++
    ANN.depends = $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/Makefile
    QMAKE_EXTRA_TARGETS += ANN
}
macx {
    PRE_TARGETDEPS = $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/libANN.a
    LIBS            += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/libANN.a
    ANN.target = $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/libANN.a
    ANN.commands = cd $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann && make macosx-g++
    ANN.depends = $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/Makefile
    QMAKE_EXTRA_TARGETS += ANN

}
win32 {
    win32-msvc:LIBS += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/lib/ANN.lib
    INCLUDEPATH += $$V3DMAINPATH/../../vaa3d_tools/hackathon/xiaoxiaol/consensus_skeleton_2/ann/MS_Win32/dll/
}


HEADERS	+= marker_minspanningtree_plugin.h

SOURCES	+= marker_minspanningtree_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(marker2mst)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/marker_utilities/markers_to_minimum_spanning_tree/
