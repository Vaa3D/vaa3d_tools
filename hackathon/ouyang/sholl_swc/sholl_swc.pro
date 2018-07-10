
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	= \
    sholl_swc.h \
    sholl_plugin.h \
    sholl_func.h \
    openSWCDialog.h
HEADERS	+=
HEADERS +=
HEADERS +=

SOURCES	= \
    sholl_func.cpp \
    sholl_plugin.cpp \
    sholl_swc.cpp \
    openSWCDialog.cpp
SOURCES	+=
SOURCES +=
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(sholl_swc)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_analysis/sholl_analysis_swc/
