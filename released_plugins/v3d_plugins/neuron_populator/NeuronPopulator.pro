
TEMPLATE = lib
CONFIG	 += qt plugin warn_off


VAA3DPATH   = ../../../..//v3d_external
V3DMAINPATH = ../../../..//v3d_external/v3d_main
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/jba/newmat11


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
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h

SOURCES	+= NeuronPopulator_plugin.cpp
SOURCES	+= populate_neurons.cpp
SOURCES	+= ../neuron_image_profiling/openSWCDialog.cpp
SOURCES	+= ../affine_transform_swc/apply_transform_func.cpp
SOURCES	+= ../affine_transform_swc/io_affine_transform.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp



TARGET	= $$qtLibraryTarget(NeuronPopulator)
DESTDIR	= $$VAA3DPATH/bin/plugins/AllenApps/neuron_populator/
