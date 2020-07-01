
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc

HEADERS	+= Retrace_plugin.h \
    app2.h \
    imgpreprocess.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h \
    retracefunction.h \
    branchtree.h \
    retracedialog.h \
    swc_convert.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h
SOURCES	+= Retrace_plugin.cpp \
    app2.cpp \
    imgpreprocess.cpp \
    retracefunction.cpp \
    branchtree.cpp \
    retracedialog.cpp \
    swc_convert.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp
#SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.cpp

TARGET	= $$qtLibraryTarget(Retrace)
DESTDIR	= $$VAA3DPATH/bin/plugins/Retrace/
