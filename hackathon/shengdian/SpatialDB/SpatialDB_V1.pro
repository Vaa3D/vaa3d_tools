
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= SpatialDB_V1_plugin.h
SOURCES	+= SpatialDB_V1_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(SpatialDB_V1)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/SpatialDB_V1/
