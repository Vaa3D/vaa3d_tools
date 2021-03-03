
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= NeuronChaser_plugin.h \
    btracer.h \
    nf_dialog.h \
    toolbox.h \
    model.h \
    node.h \
    connected.h
SOURCES	+= NeuronChaser_plugin.cpp \
    btracer.cpp \
    toolbox.cpp \
    model.cpp \
    node.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(NeuronChaser)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/NeuronChaser/
