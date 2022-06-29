
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	= \
    sort_swc.h \
    sort_plugin.h \
    sort_func.h \
    openSWCDialog.h
HEADERS	+=
HEADERS +=
HEADERS +=

SOURCES	= \
    openSWCDialog.cpp \
    sort_func.cpp \
    sort_plugin.cpp
SOURCES	+=
SOURCES +=
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(sort_neuron_swc_lmg)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/sort_neuron_swc_lmg/
