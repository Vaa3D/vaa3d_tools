
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun
LIBS += -L. -lv3dtiff -L../../../v3d_main/common_lib/lib

HEADERS	= lobeseg_plugin.h
HEADERS	+= lobeseg_func.h
HEADERS	+= lobeseg_gui.h

HEADERS += ../lobeseg.h
HEADERS	+= ../../../v3d_main/basic_c_fun/basic_memory.h
HEADERS	+= ../../../v3d_main/basic_c_fun/stackutil.h
HEADERS	+= ../../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS	+= ../../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS	+= ../../../v3d_main/basic_c_fun/volimg_proc.h

HEADERS += ../../../v3d_main/worm_straighten_c/bdb_minus.h
HEADERS += ../../../v3d_main/worm_straighten_c/mst_prim_c.h
HEADERS += ../../../v3d_main/worm_straighten_c/graphsupport.h
HEADERS += ../../../v3d_main/worm_straighten_c/graph.h
HEADERS += ../../../v3d_main/worm_straighten_c/bfs.h

SOURCES	= lobeseg_plugin.cpp
SOURCES	+= lobeseg_func.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += ../lobeseg.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/basic_memory.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += ../../../v3d_main/worm_straighten_c/bdb_minus.cpp
SOURCES += ../../../v3d_main/worm_straighten_c/mst_prim_c.cpp
SOURCES += ../../../v3d_main/worm_straighten_c/bfs_1root.cpp

TARGET	= $$qtLibraryTarget(lobeseg)
DESTDIR	= ~/Applications/v3d/plugins/lobeseg/
