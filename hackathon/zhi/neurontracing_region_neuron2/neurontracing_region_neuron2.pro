
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =  ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include

macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
}

HEADERS	+= neurontracing_region_neuron2_plugin.h

SOURCES	+= neurontracing_region_neuron2_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(neurontracing_region_neuron2)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/Region_Neuron2/
