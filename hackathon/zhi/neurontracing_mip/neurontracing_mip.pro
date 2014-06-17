
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= neurontracing_mip_plugin.h
SOURCES	+= neurontracing_mip_plugin.cpp

SOURCES += my_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(neurontracing_mip)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/neurontracing_mip/
