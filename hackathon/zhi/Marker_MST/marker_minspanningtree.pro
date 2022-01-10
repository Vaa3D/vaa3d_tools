
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
#INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

INCLUDEPATH += main

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
#HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
#HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS	+= marker_minspanningtree_plugin.h

SOURCES	+= marker_minspanningtree_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
#SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
#SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
#SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(marker_minspanningtree)
DESTDIR	= ../../../../v3d_external/bin//plugins/marker_utilities/marker_MST/
