
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	 = blastneuron_bjut_plugin.h \
    blastneuron_bjut_func.h \
    dynamic.h \
    overlap/overlap_gold.h \
    sort_swc.h \
    overlap/pre_overlap.h \
    prune_alignment.h
SOURCES= blastneuron_bjut_plugin.cpp \
    resample_func.cpp \
    prune_alignment.cpp \
    blastneuron_func.cpp \
    overlap/overlap_gold.cpp \
    sort_swc.cpp \
    overlap/pre_overlap.cpp
SOURCES+= blastneuron_bjut_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES+=my_surf_objs.cpp
SOURCES+=seg_weight.cpp
SOURCES+=swc_utils.cpp

TARGET	= $$qtLibraryTarget(blastneuron_bjut)
DESTDIR	= $$VAA3DPATH/bin/plugins/blastneuron_bjut/
