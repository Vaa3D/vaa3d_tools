
TEMPLATE = lib
CONFIG	 += qt plugin warn_off


V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d


macx{
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    }
    else {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

unix:!macx{
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}


HEADERS	+= NeuronPopulator_plugin.h
HEADERS	+= populate_neurons.h
HEADERS	+= ../neuron_image_profiling/openSWCDialog.h
HEADERS	+= ../affine_transform_swc/apply_transform_func.h
HEADERS	+= ../affine_transform_swc/io_affine_transform.h
HEADERS	+= $$V3DMAINPATH/basic_c_fun/v3d_message.h

SOURCES	+= NeuronPopulator_plugin.cpp
SOURCES	+= populate_neurons.cpp
SOURCES	+= ../neuron_image_profiling/openSWCDialog.cpp
SOURCES	+= ../affine_transform_swc/apply_transform_func.cpp
SOURCES	+= ../affine_transform_swc/io_affine_transform.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp



TARGET	= $$qtLibraryTarget(NeuronPopulator)
DESTDIR	= $$V3DMAINPATH/bin/plugins/AllenApps/neuron_populator/
