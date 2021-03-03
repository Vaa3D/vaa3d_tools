
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d


HEADERS	+= neurontracing_mip_plugin.h
SOURCES	+= neurontracing_mip_plugin.cpp

SOURCES += my_surf_objs.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += ../../../hackathon/zhi/APP2_large_scale/readrawfile_func.cpp

win32{
    LIBS         += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
    LIBS	 += -L$$V3DMAINPATH/common_lib/winlib64 -llibfftw3f-3
}

unix{
    LIBS         += -lm -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS         += -lpthread
    LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads
}

TARGET	= $$qtLibraryTarget(neurontracing_mip)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_tracing/TReMap/
