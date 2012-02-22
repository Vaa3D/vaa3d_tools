TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH  += ../../../v3d_main/
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += global_neuron_feature.h

HEADERS      += Nfmain.h
HEADERS      += compute.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += global_neuron_feature.cpp
SOURCES      += Nfmain.cpp
SOURCES      += compute.cpp

TARGET        = $$qtLibraryTarget(global_neuron_feature)
DESTDIR       = ../../v3d/plugins/neuron_utilities/global_neuron_feature
