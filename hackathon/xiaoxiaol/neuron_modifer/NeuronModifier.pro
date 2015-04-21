
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /local1/xiaoxiaol/work/v3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter
HEADERS	+= NeuronModifier_plugin.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h
SOURCES	+= NeuronModifier_plugin.cpp
SOURCES	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter/eswc_core.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(NeuronModifier)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/NeuronModifier/
