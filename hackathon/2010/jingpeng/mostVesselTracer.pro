
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#QT+=opengl
#CONFIG      += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
INCLUDEPATH  += ../../../v3d_main/v3d
INCLUDEPATH  += ../../../v3d_main/neuron_editing
HEADERS       = mostimage.h
HEADERS      += tools.h
HEADERS      += mostVesselTracer.h
HEADERS      += voxelcluster.h
HEADERS      += srb.h
HEADERS      += ../../../v3d_main/neuron_editing/neuron_format_converter.h
HEADERS      += ../../../v3d_main/neuron_editing/v_neuronswc.h

SOURCES       = mostimage.cpp
SOURCES      += tools.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += mostVesselTracer.cpp
SOURCES      += voxelcluster.cpp
SOURCES      += srb.c
SOURCES      += ../../../v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES      += ../../../v3d_main/neuron_editing/v_neuronswc.cpp



TARGET        = $$qtLibraryTarget(mostVesselTracer)

DESTDIR       = /home/most/workspace/v3d/bin/v3d/Linux_64/plugins/most222/



