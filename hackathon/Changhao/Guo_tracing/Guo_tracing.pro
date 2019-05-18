
#TEMPLATE	= lib
#CONFIG	+= qt plugin warn_off
##CONFIG	+= x86_64
#VAA3DPATH = ../../../../v3d_external
#INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
#INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
#INCLUDEPATH	+= $$VAA3DPATH/v3d_main/v3d_main/neuron_editing
#INCLUDEPATH     += $$VAA3DPATH/v3d_main/worm_straighten_c
#INCLUDEPATH     += app1

#unix {
#LIBS += -L$$VAA3DPATH/v3d_main/jba/c++
#LIBS += -lv3dnewmat
#}
#win32 {
#LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64
#LIBS += -llibnewmat
#}

#HEADERS	+= Guo_tracing_plugin.h
#SOURCES	+= Guo_tracing_plugin.cpp
#SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
#SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
#SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp


#HEADERS += app1/gd.h
#HEADERS += app1/v3dneuron_gd_tracing.h
#HEADERS += app1/marker_radius.h
#HEADERS += app1/sort_swc.h
#HEADERS += vn.h
#HEADERS += vn_imgpreprocess.h
#HEADERS += my_surf_objs.h
##HEADERS += v3dneuron_gd_tracing.h
##HEADERS += v3d_curvetracepara.h
##HEADERS += v_neuronswc.h
##HEADERS += gd.h
##HEADERS += bdb_minus.h
##HEADERS += graph.h
##HEADERS += tip_detection.h
##HEADERS += FL_bwdist.h

#SOURCES += vn_imgpreprocess.cpp
#SOURCES += my_surf_objs.cpp
##SOURCES += v3dneuron_gd_tracing.cpp
##SOURCES += v_neuronswc.cpp
##SOURCES += gd.cpp
##SOURCES += bdb_minus.cpp
##SOURCES += tip_detection.cpp
##SOURCES	+= $$VAA3DPATH/v3d_main/graph/dijk.cpp


TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$V3DMAINPATH/worm_straighten_c
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += app1

unix {
LIBS += -L$$V3DMAINPATH/jba/c++
LIBS += -lv3dnewmat
}
win32 {
LIBS += -L$$V3DMAINPATH/common_lib/winlib64
LIBS += -llibnewmat
}

HEADERS	+= Guo_tracing_plugin.h
HEADERS	+= my_surf_objs.h
HEADERS	+= marker_radius.h
HEADERS	+= app1/gd.h
HEADERS	+= app1/v3dneuron_gd_tracing.h
HEADERS	+= app1/tip_detection.h
HEADERS	+= neuron_tools.h
HEADERS	+= fastmarching_dt.h
HEADERS	+= heap.h
HEADERS	+= fastmarching_macro.h
HEADERS += ClusterAnalysis_20190409.h

SOURCES	+= Guo_tracing_plugin.cpp
SOURCES	+= my_surf_objs.cpp
SOURCES	+= app1/calculate_cover_scores.cpp
SOURCES	+= app1/gd.cpp
SOURCES	+= app1/v3dneuron_gd_tracing.cpp
SOURCES	+= app1/tip_detection.cpp
SOURCES	+= neuron_tools.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$V3DMAINPATH/worm_straighten_c/bdb_minus.cpp
SOURCES += $$V3DMAINPATH/worm_straighten_c/mst_prim_c.cpp
SOURCES += $$V3DMAINPATH/worm_straighten_c/bfs_1root.cpp
SOURCES += $$V3DMAINPATH/graph/dijk.cpp
SOURCES += $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES += $$V3DMAINPATH/neuron_editing/v_neuronswc.cpp

SOURCES +=$$V3DMAINPATH/../released_plugins_more/v3d_plugins/blastneuron_plugin/global_cmp/compute_morph.cpp


TARGET	= $$qtLibraryTarget(Guo_tracing)
DESTDIR	= $$VAA3DPATH/bin/plugins/Guo/Guo_tracing/
