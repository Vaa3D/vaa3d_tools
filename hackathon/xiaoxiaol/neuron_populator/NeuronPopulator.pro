
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64


VAA3DPATH  = ../../../..//v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH   += $$VAA3DPATH/v3d_main/jba/newmat11
macx{
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}

HEADERS	+= NeuronPopulator_plugin.h
HEADERS	+= populate_neurons.h
HEADERS	+= ../neuron_image_profiling/openSWCDialog.h
HEADERS	+= ../affine_transform_swc/apply_transform_func.h
HEADERS	+= ../affine_transform_swc/io_affine_transform.h
SOURCES	+= NeuronPopulator_plugin.cpp
SOURCES	+= populate_neurons.cpp
SOURCES	+= ../neuron_image_profiling/openSWCDialog.cpp
SOURCES	+= ../affine_transform_swc/apply_transform_func.cpp
SOURCES	+= ../affine_transform_swc/io_affine_transform.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp



SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h


TARGET	= $$qtLibraryTarget(NeuronPopulator)
DESTDIR	= $$VAA3DPATH/bin/plugins/NeuronPopulator/
