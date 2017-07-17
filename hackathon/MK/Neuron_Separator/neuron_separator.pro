
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= neuron_separator_plugin.h
SOURCES	+= neuron_separator_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$INCLUDEPATH/basic_surf_objs.cpp
SOURCES += neuronSeparation_func.cpp

TARGET	= $$qtLibraryTarget(neuron_separator)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_separator/
