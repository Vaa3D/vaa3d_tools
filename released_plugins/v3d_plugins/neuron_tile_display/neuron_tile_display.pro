
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../..
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= neuron_tile_display_plugin.h \
    neuron_tile_display_dialog.h
SOURCES	+= neuron_tile_display_plugin.cpp \
    neuron_tile_display_dialog.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS += $$V3DMAINPATH/v3d_main/neuron_editing/neuron_xforms.h
SOURCES += $$V3DMAINPATH/v3d_main/neuron_editing/neuron_xforms.cpp

TARGET	= $$qtLibraryTarget(neuron_tile_display)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/tile_display_multiple_neurons/
