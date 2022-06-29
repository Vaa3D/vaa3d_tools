
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing

HEADERS	+= BoutonDetection_plugin.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../v3d_main/basic_c_fun/color_xyz.h \
    boutonDetection_fun.h \
    ../../../v3d_main/neuron_editing/neuron_format_converter.h \
    ../../../v3d_main/neuron_editing/v_neuronswc.h \
    neuronbranchtree.h \
    ../../SGuo/imPreProcess/helpFunc.h \
    ../../SGuo/imPreProcess/bilateral_filter.h
SOURCES	+= BoutonDetection_plugin.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp \
    boutonDetection_fun.cpp \
    ../../../v3d_main/neuron_editing/neuron_format_converter.cpp \
    ../../../v3d_main/neuron_editing/v_neuronswc.cpp \
    neuronbranchtree.cpp \
    ../../SGuo/imPreProcess/helpFunc.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BoutonDetection)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/BoutonDetection/
