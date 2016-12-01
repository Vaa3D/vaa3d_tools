
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	 = blastneuron_bjut_plugin.h \
    blastneuron_bjut_func.h \
    resample_func.h
SOURCES= blastneuron_bjut_plugin.cpp \
    resample_func.cpp
SOURCES+= blastneuron_bjut_func.cpp
SOURCES+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(blastneuron_bjut)
DESTDIR	= $$VAA3DPATH/bin/plugins/blastneuron_bjut/
