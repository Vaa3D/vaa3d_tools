
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= neuron_connector_plugin.h \
    neuron_connector_func.h
SOURCES	+= neuron_connector_plugin.cpp \
    neuron_connector_func.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(neuron_connector)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/neuron_connector/
