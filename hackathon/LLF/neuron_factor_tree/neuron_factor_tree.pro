
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include

HEADERS	+= neuron_factor_tree_plugin.h

SOURCES	+= neuron_factor_tree_plugin.cpp
SOURCES	+= neuron_factor_tree_tools.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(neuron_factor_tree)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_factor_tree/