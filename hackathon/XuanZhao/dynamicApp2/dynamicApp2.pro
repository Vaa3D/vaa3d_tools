
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH     += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += app2
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/jba/newmat11
INCLUDEPATH	+= $$V3DMAINPATH/neuron_editing

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

LIBS            += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat

DEFINES += QT_MESSAGELOGCONTEXT

HEADERS	+= dynamicApp2_plugin.h \
    axontrace.h \
    dlog.h \
    regiongrow.h \
    test.h
HEADERS += vn_imgpreprocess.h
HEADERS += vn.h
HEADERS += vn_app2.h
HEADERS += app2/fastmarching_tree.h
HEADERS += app2/hierarchy_prune.h
HEADERS += app2/fastmarching_dt.h
HEADERS += app2/my_surf_objs.h
HEADERS += app2/marker_radius.h
HEADERS += swc_convert.h

HEADERS += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h

HEADERS += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h

HEADERS += $$V3DMAINPATH/neuron_editing/neuron_format_converter.h

HEADERS += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/resample_swc/resampling.h
#HEADERS += $$V3DMAINPATH/basic_c_fun/stackutil.cpp

SOURCES	+= dynamicApp2_plugin.cpp \
    axontrace.cpp \
    test.cpp
SOURCES += vn_imgpreprocess.cpp
SOURCES += app2_connector.cpp
SOURCES += swc_convert.cpp
SOURCES += app2/my_surf_objs.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp

SOURCES	+= $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$V3DMAINPATH/neuron_editing/global_feature_compute.cpp
SOURCES += $$V3DMAINPATH/neuron_editing/neuron_format_converter.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp

#SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp

TARGET	= $$qtLibraryTarget(dynamicApp2)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/dynamicApp2/
