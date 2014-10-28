TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
V3DMAINDIR = ../../../v3d_main 

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
INCLUDEPATH  += $$V3DMAINDIR/neuron_editing

HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
HEADERS      += Neuron_tracing.h
HEADERS      += parser.h
HEADERS      += NeuronSegmentation.h
HEADERS      += NeuronEnhancementFilter.h
HEADERS      += $$V3DMAINDIR/neuron_editing/neuron_format_converter.h
HEADERS      += $$V3DMAINDIR/neuron_editing/v_neuronswc.h"

HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += NeuronSegmentation.cpp
SOURCES      += NeuronEnhancementFilter.cpp
LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -lpthread


SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += Neuron_tracing.cpp
SOURCES      += parser.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(SimpleTracing)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/neuron_tracing/SimpleTracing
