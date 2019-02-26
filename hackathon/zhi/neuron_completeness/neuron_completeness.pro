
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	+= neuron_completeness_plugin.h
HEADERS += ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h


SOURCES	+= neuron_completeness_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(neuron_completeness)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/neuron_completeness/
