
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../..//v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= neuron_image_profiling_plugin.h
SOURCES	+= neuron_image_profiling_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_image_profiling)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_image_profiling/
