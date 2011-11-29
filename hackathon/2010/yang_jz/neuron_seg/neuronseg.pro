TEMPLATE      = lib
CONFIG       += qt plugin warn_off

INCLUDEPATH  += ../../../v3d_main/basic_c_fun

HEADERS      +=../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADES       +=../../../v3d_main/neuron_editing/neuron_format_converter.h
HEADES       +=../../../v3d_main/neuron_editing/v_neuronswc.h

HEADERS      += neuronseg.h

SOURCES      +=../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += neuronseg.cpp
SOURCES      +=../../../v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES      +=../../../v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES      +=../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(neuronseg)
DESTDIR       = ../../../v3d/plugins/neuronseg
