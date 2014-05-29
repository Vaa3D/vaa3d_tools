
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include


HEADERS	+= APP2_large_scale_plugin.h
SOURCES	+= APP2_large_scale_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += my_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp


LIBS         += -lm -L$$VAA3DPATH/common_lib/lib -lv3dtiff
LIBS         += -lpthread
LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads

TARGET	= $$qtLibraryTarget(APP2_large_scale)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/APP2_large_scale/
