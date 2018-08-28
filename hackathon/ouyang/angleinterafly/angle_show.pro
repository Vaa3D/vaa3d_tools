
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
HEADERS	+= \
    show_func.h \
    show_plugin.h \
    angle_marker.h \
    sort_swc.h


SOURCES	+= \
    show_func.cpp \
    show_plugin.cpp \
    angle_marker.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(angle_show)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_analysis/angle_show/
