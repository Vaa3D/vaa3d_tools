
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS	+= neuronfeature_plugin.h
HEADERS += NFmain.h
HEADERS += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.h

HEADERS += openSWCDialog.h

SOURCES	+= neuronfeature_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += NFmain.cpp
SOURCES      += $$VAA3DPATH/v3d_main/neuron_editing/global_feature_compute.cpp
SOURCES      += openSWCDialog.cpp



TARGET	= $$qtLibraryTarget(VRneuronFeature)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/VR_neuron_feature/
