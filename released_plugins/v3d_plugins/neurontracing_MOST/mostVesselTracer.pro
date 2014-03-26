
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#QT+=opengl
#CONFIG      += x86_64

V3DMAINPATH = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun 
INCLUDEPATH  += $$V3DMAINPATH/v3d
INCLUDEPATH  += $$V3DMAINPATH/neuron_editing
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include

HEADERS       = mostimage.h
HEADERS      += tools.h
HEADERS      += mostVesselTracer.h
HEADERS      += voxelcluster.h
HEADERS      += srb.h
HEADERS      += $$V3DMAINPATH/neuron_editing/neuron_format_converter.h
HEADERS      += $$V3DMAINPATH/neuron_editing/v_neuronswc.h

SOURCES       = mostimage.cpp
SOURCES      += tools.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += mostVesselTracer.cpp
SOURCES      += voxelcluster.cpp
SOURCES      += srb.c
SOURCES      += $$V3DMAINPATH/neuron_editing/neuron_format_converter.cpp
SOURCES      += $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp



TARGET        = $$qtLibraryTarget(mostVesselTracer)

DESTDIR       = $$V3DMAINPATH/../bin/plugins/neuron_tracing/MOST_tracing/



