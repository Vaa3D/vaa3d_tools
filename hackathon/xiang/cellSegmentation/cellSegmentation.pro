TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= cellSegmentation_plugin.h
SOURCES	+= cellSegmentation_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(cellSegmentation)
DESTDIR = ./release