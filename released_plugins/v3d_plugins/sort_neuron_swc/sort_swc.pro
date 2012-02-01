
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	= sort_plugin.h
HEADERS	+= sort_func.h
HEADERS += sort_swc.h

SOURCES	= sort_plugin.cpp
SOURCES	+= sort_func.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(sort_neuron_swc)
DESTDIR	= $$V3DMAINPATH/../v3d/plugins/neuron_utilities/sort_neuron_swc/
