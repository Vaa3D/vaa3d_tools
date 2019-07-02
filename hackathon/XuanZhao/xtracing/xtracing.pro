
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= xtracing_plugin.h \
    t_class.h \
    t_function.h
SOURCES	+= xtracing_plugin.cpp \
    t_class.cpp \
    t_function.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(xtracing)
DESTDIR	= $$VAA3DPATH/bin/plugins/xtracing/
