
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= FFT_plugin.h
SOURCES	+= FFT_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

LIBS         += -lm -L$$VAA3DPATH/v3d_main/common_lib/lib -lv3dtiff
LIBS         += -lpthread
LIBS	     += -lfftw3 -lv3dfftw3f_threads


TARGET	= $$qtLibraryTarget(FFT)
DESTDIR	= $$VAA3DPATH/bin/plugins/FFT/
