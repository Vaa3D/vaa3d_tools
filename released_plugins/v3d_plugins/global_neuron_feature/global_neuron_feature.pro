TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH  += ../../../../v3d_external/v3d_main/
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/v3d

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += global_neuron_feature.h

HEADERS      += Nfmain.h
HEADERS      += $$V3DMAINPATH/../v3d_main/neuron_editing/global_feature_compute.h

HEADERS      += ../sort_neuron_swc/openSWCDialog.h


SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += global_neuron_feature.cpp
SOURCES      += Nfmain.cpp
SOURCES      += $$V3DMAINPATH/../v3d_main/neuron_editing/global_feature_compute.cpp
SOURCES      += ../sort_neuron_swc/openSWCDialog.cpp


TARGET        = $$qtLibraryTarget(global_neuron_feature)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/neuron_utilities/global_neuron_feature
