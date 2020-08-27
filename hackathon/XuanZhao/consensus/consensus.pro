
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc

HEADERS	+= consensus_plugin.h \
    branchtree.h \
    imgpreprocess.h \
    swc_convert.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    concensusfunction.h \
    multiApp2.h
HEADERS += app2/fastmarching_tree.h
HEADERS += app2/hierarchy_prune.h
HEADERS += app2/fastmarching_dt.h

SOURCES	+= consensus_plugin.cpp \
    branchtree.cpp \
    imgpreprocess.cpp \
    swc_convert.cpp \
    concensusfunction.cpp \
    multiApp2.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp

SOURCES += app2/my_surf_objs.cpp

#meanshift
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp

TARGET	= $$qtLibraryTarget(consensus)
DESTDIR	= $$VAA3DPATH/bin/plugins/consensus/
