
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= neuronPicker_plugin.h \
    neuronpicker_func.h \
    neuronpicker_templates.h \
    neuron_seperator_explorer.h
SOURCES	+= neuronPicker_plugin.cpp \
    neuronpicker_func.cpp \
    neuron_seperator_explorer.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_color_seperator)
DESTDIR = $$V3DMAINPATH/bin/plugins/neuron_utilities/neuron_color_seperator/
