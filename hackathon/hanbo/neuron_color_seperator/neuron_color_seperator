
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= neuronPicker_plugin.h \
    neuronpicker_func.h \
    neuronpicker_templates.h
SOURCES	+= neuronPicker_plugin.cpp \
    neuronpicker_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neuron_color_seperator)
DESTDIR = $$VAA3DPATH/bin/plugins/neuron_color_seperator/
