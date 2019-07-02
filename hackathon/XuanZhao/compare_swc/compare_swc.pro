
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= compare_swc_plugin.h \
    neural_class.h \
    neural_class.h
SOURCES	+= compare_swc_plugin.cpp \
    neural_class.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    neural_class.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(compare_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/compare_swc/
