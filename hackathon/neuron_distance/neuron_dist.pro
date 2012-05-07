
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= neuron_dist_plugin.h
HEADERS	+= neuron_dist_func.h
HEADERS	+= neuron_dist_gui.h
HEADERS	+= neuron_distance.h

SOURCES	= neuron_dist_plugin.cpp
SOURCES	+= neuron_dist_func.cpp
SOURCES	+= neuron_dist_gui.cpp
SOURCES	+= neuron_distance.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(neuron_dist)
#DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_dist/
