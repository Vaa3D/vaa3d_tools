
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/Vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
HEADERS	+= neuronQC_plugin.h \
    neuronqc_func.h
SOURCES	+= neuronQC_plugin.cpp \
    neuronqc_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp

TARGET	= $$qtLibraryTarget(neuronQC)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuronQC/

QMAKE_CXXFLAGS = -std=c++0x
