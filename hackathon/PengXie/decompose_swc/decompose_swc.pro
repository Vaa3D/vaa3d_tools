
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
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
HEADERS	+= decompose_swc_plugin.h \
    decompose_swc_main.h
SOURCES	+= decompose_swc_plugin.cpp \
    decompose_swc_main.cpp

HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS	+= $$VAA3DPATH/v3d_main/neuron_editing/neuron_format_converter.h
HEADERS	+= $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.h

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp

TARGET	= $$qtLibraryTarget(decompose_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/decompose_swc/
