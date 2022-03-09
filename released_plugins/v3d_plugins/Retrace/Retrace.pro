
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += widgets
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11

#unix {
#LIBS += -L$$VAA3DPATH/v3d_main/jba/c++
#LIBS += -lv3dnewmat
#}
#win32 {
#LIBS += -L$$VAA3DPATH/v3d_main/common_lib/mingw
#LIBS += -L$$VAA3DPATH/v3d_main/common_lib/mingw -lv3dnewmat
#}

#LIBS            += -L$$VAA3DPATH/v3d_main/common_lib/mingw -lv3dnewmat

macx{
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    DEFINES += WIN32
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/mingw -lv3dnewmat
}

unix:!macx {
    DEFINES += WIN32
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}

HEADERS	+= Retrace_plugin.h \
    app2.h \
    imgpreprocess.h \
    app2/fastmarching_dt.h \
    app2/fastmarching_tree.h \
    app2/hierarchy_prune.h \
    retracefunction.h \
    branchtree.h \
    retracedialog.h \
    swc_convert.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    app2/my_surf_objs.h \
    judgebranch.h \
    swc2mask.h \
    pca.h
#HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neuron_reliability_score/src/fastmarching_linker.h
#HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neuron_reliability_score/src/fastmarching_tree.h
#HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h

SOURCES	+= Retrace_plugin.cpp \
    app2.cpp \
    imgpreprocess.cpp \
    retracefunction.cpp \
    branchtree.cpp \
    retracedialog.cpp \
    swc_convert.cpp \
    judgebranch.cpp \
    swc2mask.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += app2/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp

#meanshift
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp

TARGET	= $$qtLibraryTarget(Retrace)
DESTDIR	= $$VAA3DPATH/bin/plugins/Retrace/
