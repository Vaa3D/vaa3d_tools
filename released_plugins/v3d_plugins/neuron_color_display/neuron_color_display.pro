
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= neuron_color_display_plugin.h
SOURCES	+= neuron_color_display_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_color_display)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/neuron_color_display/
