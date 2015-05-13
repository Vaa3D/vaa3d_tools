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
HEADERS      += $$V3DMAINDIR/neuron_editing/v_neuronswc.h

SOURCES      += NeuronSegmentation.cpp
SOURCES      += NeuronEnhancementFilter.cpp

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINDIR/common_lib/winlib64 -llibtiff
    } else {
    LIBS     += -L$$V3DMAINDIR/common_lib/winlib -llibtiff
    }
}

unix {
    LIBS     += -lm -lpthread
    LIBS     += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
}

SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += Neuron_tracing.cpp
SOURCES      += parser.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(SimpleTracing)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/neuron_tracing/SimpleTracing
