
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= ../neurontracing_vn2/neuron_radius/neuron_radius_plugin.h
HEADERS	+= ../neurontracing_vn2/neuron_radius/my_surf_objs.h
HEADERS	+= ../neurontracing_vn2/neuron_radius/marker_radius.h 
SOURCES	+= ../neurontracing_vn2/neuron_radius/neuron_radius_plugin.cpp
SOURCES += ../neurontracing_vn2/neuron_radius/my_surf_objs.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(neuron_radius)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/neuron_radius/
