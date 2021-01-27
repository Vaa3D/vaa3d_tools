
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH += ../../../released_plugins/v3d_plugins/sort_neuron_swc

HEADERS	+= subtree_labelling_plugin.h \
    subtree_dialog.h \
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h

SOURCES	+= subtree_labelling_plugin.cpp \
    subtree_dialog.cpp \
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp \
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.cpp

SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(subtree_labelling)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/subtree_labelling/
