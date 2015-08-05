
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64


VAA3DPATH  = ../../../..//v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun


HEADERS	+= NeuronPopulator_plugin.h \
    populate_neurons.h
SOURCES	+= NeuronPopulator_plugin.cpp \
    populate_neurons.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp



SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h


TARGET	= $$qtLibraryTarget(NeuronPopulator)
DESTDIR	= $$VAA3DPATH/bin/plugins/NeuronPopulator/
