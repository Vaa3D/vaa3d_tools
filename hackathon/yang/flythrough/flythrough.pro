TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
QMAKE_CXXFLAGS += -std=c++11

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
VAA3DPLUGINPATH = ../../../released_plugins/v3d_plugins

#
INCLUDEPATH += $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH += $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH += $$VAA3DMAINPATH/common_lib/tiff4/include

# basic func
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.h
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/neuron_format_converter.h

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/neuron_format_converter.cpp

# get level 0 data
HEADERS	+= flythrough_plugin.h
HEADERS	+= flythrough_func.h

SOURCES	+= flythrough_plugin.cpp
SOURCES	+= flythrough_func.cpp

#
macx{
    QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing -mmacosx-version-min=10.7
    LIBS += -L$$VAA3DMAINPATH/common_lib/tiff4/lib_mac -ltiff -lc++
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp -pthread
    LIBS += -fopenmp
    LIBS += -L$$VAA3DMAINPATH/common_lib/tiff4/lib_linux -ltiff
}

TARGET	= $$qtLibraryTarget(flythrough)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/image_filters/flythrough/

