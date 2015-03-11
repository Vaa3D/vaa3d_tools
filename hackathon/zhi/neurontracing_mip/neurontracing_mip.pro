
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include


HEADERS	+= neurontracing_mip_plugin.h
SOURCES	+= neurontracing_mip_plugin.cpp

SOURCES += my_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += ../APP2_large_scale/readrawfile_func.cpp

win32{
    LIBS         += -L$$VAA3DPATH/common_lib/winlib64 -llibtiff
    LIBS	 += -L$$VAA3DPATH/common_lib/winlib64 -llibfftw3f-3
}

unix{
    LIBS         += -lm -L$$VAA3DPATH/common_lib/lib -lv3dtiff
    LIBS         += -lpthread
    LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads
}

TARGET	= $$qtLibraryTarget(neurontracing_mip)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/TReMap/
