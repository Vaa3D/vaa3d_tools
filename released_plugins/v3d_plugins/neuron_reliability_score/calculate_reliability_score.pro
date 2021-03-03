
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d
INCLUDEPATH	+= $$V3DMAINPATH/neuron_editing

INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
    CONFIG += x86_64
}

win32{
}

unix:!macx {
    #LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

HEADERS	+= calculate_reliability_score_plugin.h
SOURCES	+= calculate_reliability_score_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/neuron_format_converter.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp

HEADERS += src/common_macro.h
HEADERS += src/fastmarching_linker.h
HEADERS += src/fastmarching_tree.h
HEADERS += src/heap.h
HEADERS += src/simple_c.h
HEADERS += src/topology_analysis.h
HEADERS += src/my_surf_objs.h

SOURCES += src/simple_c.cpp
SOURCES += src/my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(calculate_reliability_score)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/calculate_reliability_score/
