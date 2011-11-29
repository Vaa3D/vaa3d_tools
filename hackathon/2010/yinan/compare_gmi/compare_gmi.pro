TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH = ../../../v3d_main/
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include/

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += compare_gmi.h
HEADERS		 += compute_gmi.h
HEADERS      += sim_measure.h
HEADERS      += fcompare.h
HEADERS	     += $$V3DMAINPATH/common_lib/include/boost/math/distributions/students_t.hpp


SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += compare_gmi.cpp
SOURCES      += fcompare.cpp

TARGET        = $$qtLibraryTarget(compare_gmi)
DESTDIR       = ../../../v3d/plugins/neuron_comparison/compare_gmi
