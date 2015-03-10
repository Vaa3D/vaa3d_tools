
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

V3DMAINFOLER = ../../../v3d_main
INCLUDEPATH     += $$V3DMAINFOLER/basic_c_fun
INCLUDEPATH     += $$V3DMAINFOLER/common_lib/include
#LIBS += -L. -lv3dtiff -L$$V3DMAINFOLER/common_lib/lib

HEADERS = lobeseg_plugin.h
HEADERS += lobeseg_func.h
HEADERS += lobeseg_gui.h
HEADERS += getopt_new.h

HEADERS += lobeseg_main/lobeseg.h
#HEADERS += $$V3DMAINFOLER/basic_c_fun/basic_memory.h
#HEADERS += $$V3DMAINFOLER/basic_c_fun/stackutil.h
#HEADERS += $$V3DMAINFOLER/basic_c_fun/mg_utilities.h
#HEADERS += $$V3DMAINFOLER/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINFOLER/basic_c_fun/volimg_proc.h


HEADERS += $$V3DMAINFOLER/worm_straighten_c/bdb_minus.h
HEADERS += $$V3DMAINFOLER/worm_straighten_c/mst_prim_c.h
HEADERS += $$V3DMAINFOLER/worm_straighten_c/graphsupport.h
HEADERS += $$V3DMAINFOLER/worm_straighten_c/graph.h
HEADERS += $$V3DMAINFOLER/worm_straighten_c/bfs.h

SOURCES	= lobeseg_plugin.cpp
SOURCES	+= lobeseg_func.cpp
SOURCES += getopt_new.cpp
SOURCES	+= $$V3DMAINFOLER/basic_c_fun/v3d_message.cpp

SOURCES += lobeseg_main/lobeseg.cpp

#SOURCES	+= $$V3DMAINFOLER/basic_c_fun/basic_memory.cpp
#SOURCES	+= $$V3DMAINFOLER/basic_c_fun/stackutil.cpp
#SOURCES	+= $$V3DMAINFOLER/basic_c_fun/mg_utilities.cpp
#SOURCES	+= $$V3DMAINFOLER/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINFOLER/worm_straighten_c/bdb_minus.cpp
SOURCES += $$V3DMAINFOLER/worm_straighten_c/mst_prim_c.cpp
SOURCES += $$V3DMAINFOLER/worm_straighten_c/bfs_1root.cpp

TARGET	= $$qtLibraryTarget(lobe_seg)
DESTDIR	= $$V3DMAINFOLER/../bin/plugins/image_segmentation/lobe_seger
 
