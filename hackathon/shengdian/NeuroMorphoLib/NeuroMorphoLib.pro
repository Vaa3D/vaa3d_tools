
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= NeuroMorphoLib_plugin.h \
    neuro_morpho_lib.h \
    plugin_dofunc.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../v3d_main/basic_c_fun/color_xyz.h
SOURCES	+= NeuroMorphoLib_plugin.cpp \
    neuro_morpho_lib.cpp \
    plugin_dofunc.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NeuroMorphoLib)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/NeuroMorphoLib/
