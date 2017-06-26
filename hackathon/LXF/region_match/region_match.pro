
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/hys/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= region_match_plugin.h \
    match_swc.h \
    neuron_utilities/sort_swc.h \
    neuron_utilities/resample_swc.h \
    find_feature.h \
    sim_measure.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/v3d_message.h
SOURCES	+= region_match_plugin.cpp \
    match_swc.cpp \
    neuron_utilities/sort_swc.cpp \
    neuron_utilities/resample_swc.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/v3d_message.cpp \
    find_feature.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(region_match)
DESTDIR	= $$VAA3DPATH/bin/plugins/region_match/
