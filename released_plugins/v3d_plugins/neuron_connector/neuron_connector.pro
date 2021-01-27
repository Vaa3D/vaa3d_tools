
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../..
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= neuron_connector_plugin.h \
    neuron_connector_func.h
SOURCES	+= neuron_connector_plugin.cpp \
    neuron_connector_func.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(neuron_connector)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/neuron_connector/
