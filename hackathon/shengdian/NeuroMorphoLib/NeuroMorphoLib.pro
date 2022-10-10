
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= NeuroMorphoLib_plugin.h \
    neuro_morpho_lib.h
SOURCES	+= NeuroMorphoLib_plugin.cpp \
    neuro_morpho_lib.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NeuroMorphoLib)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/NeuroMorphoLib/
