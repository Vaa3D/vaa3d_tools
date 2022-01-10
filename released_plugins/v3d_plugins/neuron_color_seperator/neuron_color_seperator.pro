
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= neuronPicker_plugin.h \
    neuronpicker_func.h \
    neuronpicker_templates.h \
    neuron_seperator_explorer.h
SOURCES	+= neuronPicker_plugin.cpp \
    neuronpicker_func.cpp \
    neuron_seperator_explorer.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_color_seperator)
DESTDIR = $$VAA3DPATH/../bin/plugins/neuron_utilities/neuron_color_seperator/
