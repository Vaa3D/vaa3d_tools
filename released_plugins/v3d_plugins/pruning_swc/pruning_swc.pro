
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d


HEADERS	+= pruning_swc_plugin.h
SOURCES	+= pruning_swc_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(pruning_swc)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/pruning_swc_simple/
