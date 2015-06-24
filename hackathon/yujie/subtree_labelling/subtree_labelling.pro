
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += ../../../released_plugins/v3d_plugins/sort_neuron_swc

HEADERS	+= subtree_labelling_plugin.h \
    subtree_dialog.h \
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h

SOURCES	+= subtree_labelling_plugin.cpp \
    subtree_dialog.cpp \
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp \
#    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(subtree_labelling)
DESTDIR	= $$VAA3DPATH/bin/plugins/subtree_labelling/
