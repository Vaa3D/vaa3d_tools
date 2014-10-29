
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= neuron_tile_display_plugin.h
SOURCES	+= neuron_tile_display_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.cpp

TARGET	= $$qtLibraryTarget(neuron_tile_display)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_tile_display/
