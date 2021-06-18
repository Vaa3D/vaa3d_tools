
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =  ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= NeuronSynthesis_plugin.h \
    nsfunctions.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h \
    Utility_function.h
SOURCES	+= NeuronSynthesis_plugin.cpp \
    nsfunctions.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp \
    Utility_function.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NeuronSynthesis)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/NeuronSynthesis/
