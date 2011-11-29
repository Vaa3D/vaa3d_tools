TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH = ../../../v3d_main/
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += compare_feature.h
HEADERS      += sim_measure.h
HEADERS      += compute.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += compare_feature.cpp
SOURCES      += compute.cpp
SOURCES      += fcompare.cpp

TARGET        = $$qtLibraryTarget(compare_feature)
DESTDIR       = ../../../v3d/plugins/neuron_comparison/compare_feature
