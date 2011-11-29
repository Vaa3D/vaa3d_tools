TEMPLATE      = lib
CONFIG       += qt plugin warn_off

INCLUDEPATH  += ../../../v3d_main/basic_c_fun

HEADERS      +=../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += global_neuron_feature.h

HEADERS      += Nfmain.h
HEADERS      += compute.h

SOURCES      +=../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += global_neuron_feature.cpp
SOURCES      += Nfmain.cpp
SOURCES      += compute.cpp

TARGET        = $$qtLibraryTarget(global_neuron_feature)
DESTDIR       = ../../../v3d/plugins/neuron_utilities/global_neuron_feature
