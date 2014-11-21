
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= auto_identify_plugin.h
SOURCES	+= auto_identify_plugin.cpp
SOURCES += $$VAA3DPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(auto_identify)
DESTDIR	= $$VAA3DPATH/../bin/plugins/cell_counting/auto_identify
