
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= neuron_stitch_plugin.h \
    neuron_stitch_func.h \
    ui_neuron_geometry_dial.h \
    neuron_geometry_dialog.h
SOURCES	+= neuron_stitch_plugin.cpp \
    neuron_geometry_dialog.cpp \
    ../../../v3d_main/neuron_editing/neuron_xforms.cpp \
    neuron_stitch_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_stitch)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_stitch/
