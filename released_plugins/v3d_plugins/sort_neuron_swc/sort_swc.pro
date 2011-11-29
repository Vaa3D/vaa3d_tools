TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH = ../../../v3d_main
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += sort_swc.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += sort_swc.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(sort_neuron_swc)
DESTDIR       = ../../v3d/plugins/neuron_utilities/sort_neuron_swc
