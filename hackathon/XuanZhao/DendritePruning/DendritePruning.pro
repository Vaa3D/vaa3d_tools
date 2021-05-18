
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
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibtiff
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibnewmat
    }
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -ltiff
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
}

LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat

HEADERS	+= DendritePruning_plugin.h \
    branchtree.h \
    swccompare.h

HEADERS += ../app3/app2/fastmarching_tree.h
HEADERS += ../app3/app2/hierarchy_prune.h
HEADERS += ../app3/app2/fastmarching_dt.h
HEADERS += ../app3/app2/my_surf_objs.h
HEADERS += ../app3/app2/marker_radius.h

HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h

SOURCES += ../app3/app2/my_surf_objs.cpp \
    swccompare.cpp

HEADERS += ../app3/swc_convert.h
SOURCES += ../app3/swc_convert.cpp

#meanshift
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp


HEADERS += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.h
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp

SOURCES	+= DendritePruning_plugin.cpp \
    branchtree.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(DendritePruning)
DESTDIR	= $$VAA3DPATH/bin/plugins/DendritePruning/
