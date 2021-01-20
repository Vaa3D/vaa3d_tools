
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += ../app3/app2
INCLUDEPATH     += ../app3
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing

INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11


macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
}

LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat


HEADERS	+= swcPruning_plugin.h \
    pruning.h \
    ../app3/swc_convert.h \
    ../app3/app2/my_surf_objs.h \
    ../app3/app2/hierarchy_prune.h \
    branchtree.h
SOURCES	+= swcPruning_plugin.cpp \
    pruning.cpp \
    ../app3/swc_convert.cpp \
    ../app3/app2/my_surf_objs.cpp \
    branchtree.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

HEADERS += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.h
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp

TARGET	= $$qtLibraryTarget(swcPruning)
DESTDIR	= $$VAA3DPATH/bin/plugins/swcPruning/
