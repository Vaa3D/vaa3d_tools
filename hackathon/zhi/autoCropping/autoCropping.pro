
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH	+= $$VAA3DPATH/neuron_editing


HEADERS	+= autoCropping_plugin.h
SOURCES	+= autoCropping_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/v_neuronswc.cpp


TARGET	= $$qtLibraryTarget(autoCropping)
DESTDIR	= $$VAA3DPATH/../bin/plugins/autoCropping/
