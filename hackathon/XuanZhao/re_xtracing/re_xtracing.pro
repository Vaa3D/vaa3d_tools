
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= re_xtracing_plugin.h \
    some_class.h \
    some_function.h \
    heap.h \
    prune.h \
    smooth_curve.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h
SOURCES	+= re_xtracing_plugin.cpp \
    some_class.cpp \
    some_function.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(re_xtracing)
DESTDIR	= $$VAA3DPATH/bin/plugins/re_xtracing/
