
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/neuron_editing

HEADERS	+= neuron_completeness_plugin.h
HEADERS	+= neuron_completeness_funcs.h

HEADERS += ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h


SOURCES	+= neuron_completeness_plugin.cpp
SOURCES	+= neuron_completeness_funcs.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/neuron_format_converter.cpp
SOURCES	+= $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp


TARGET	= $$qtLibraryTarget(neuron_completeness)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_utilities/neuron_completeness/
