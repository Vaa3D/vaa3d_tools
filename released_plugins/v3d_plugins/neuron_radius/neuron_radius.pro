
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH	+= ./

HEADERS	+= ../neurontracing_vn2/neuron_radius/neuron_radius_plugin.h
HEADERS	+= ../neurontracing_vn2/neuron_radius/my_surf_objs.h
HEADERS	+= ../neurontracing_vn2/neuron_radius/marker_radius.h 
SOURCES	+= ../neurontracing_vn2/neuron_radius/neuron_radius_plugin.cpp
SOURCES += ../neurontracing_vn2/neuron_radius/my_surf_objs.cpp

SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_radius)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/neuron_radius/
