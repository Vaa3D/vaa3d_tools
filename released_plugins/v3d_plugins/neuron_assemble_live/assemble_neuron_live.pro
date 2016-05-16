
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= assemble_neuron_live_plugin.h \
        openSWCDialog.h \
    assemble_neuron_live_dialog.h
SOURCES	+= assemble_neuron_live_plugin.cpp \
        openSWCDialog.cpp \
    assemble_neuron_live_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(assemble_neuron_live)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/assemble_neuron_live/
