
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= ./
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= Advantra_plugin.h \
    nf_dialog.h \
    toolbox.h \
    node.h \
    btracer.h \
    connected.h
SOURCES	+= Advantra_plugin.cpp \
    btracer.cpp \
    node.cpp \
    toolbox.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Advantra)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/Advantra/
