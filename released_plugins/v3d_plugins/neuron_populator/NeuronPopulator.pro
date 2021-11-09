
TEMPLATE = lib
CONFIG	 += qt plugin warn_off


VAA3DPATH   = ../../../../v3d_external/v3d_main
#V3DMAINPATH = ../../../..//v3d_external/v3d_main
INCLUDEPATH += $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/jba/newmat11
QT += widgets

macx{
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$VAA3DPATH/common_lib/mingw -lv3dnewmat
    }
    else {
    LIBS     += -L$$VAA3DPATH/common_lib/mingw -ibnewmat
    }

}

unix:!macx{
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}


HEADERS	+= NeuronPopulator_plugin.h
HEADERS	+= populate_neurons.h
HEADERS	+= ../neuron_image_profiling/openSWCDialog.h
HEADERS	+= ../affine_transform_swc/apply_transform_func.h
HEADERS	+= ../affine_transform_swc/io_affine_transform.h
HEADERS	+= $$VAA3DPATH/basic_c_fun/v3d_message.h

SOURCES	+= NeuronPopulator_plugin.cpp
SOURCES	+= populate_neurons.cpp
SOURCES	+= ../neuron_image_profiling/openSWCDialog.cpp
SOURCES	+= ../affine_transform_swc/apply_transform_func.cpp
SOURCES	+= ../affine_transform_swc/io_affine_transform.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp



TARGET	= $$qtLibraryTarget(NeuronPopulator)
DESTDIR	= $$VAA3DPATH/bin/plugins/AllenApps/neuron_populator/
