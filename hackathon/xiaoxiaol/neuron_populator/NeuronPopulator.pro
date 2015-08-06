
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

HEADERS	+= NeuronPopulator_plugin.h \
    populate_neurons.h \
    ../neuron_image_profiling/openSWCDialog.h \
    ../affine_transform_swc/apply_transform_func.h
SOURCES	+= NeuronPopulator_plugin.cpp \
    populate_neurons.cpp \
    ../neuron_image_profiling/openSWCDialog.cpp \
    ../affine_transform_swc/apply_transform_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp



SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h


TARGET	= $$qtLibraryTarget(NeuronPopulator)
DESTDIR	= $$VAA3DPATH/bin/plugins/NeuronPopulator/
