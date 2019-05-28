
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Auto_Programing_neurontracing_plugin.h \
    mydialog.h \
    cut_image_p.h \
    cut_image_f.h \
    resampling.h
SOURCES	+= Auto_Programing_neurontracing_plugin.cpp \
    mydialog.cpp \
    cut_image_p.cpp \
    cut_image_f.cpp \
    resampling.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Auto_Programing_neurontracing)
DESTDIR	= $$VAA3DPATH/bin/plugins/Auto_Programing_neurontracing/
