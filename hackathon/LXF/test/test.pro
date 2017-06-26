
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/lxf/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= test_plugin.h \
    find_feature.h \
    match_swc.h \
    sim_measure.h \
    resample_swc.h \
    sort_swc.h
SOURCES	+= test_plugin.cpp \
    match_swc.cpp \
    find_feature.cpp \
    resample_swc.cpp \
    sort_swc.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(test)
DESTDIR	= $$VAA3DPATH/bin/plugins/test/
