TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= ENT_plugin.h
HEADERS += vn_imgpreprocess.h
HEADERS += fastmarching_tree.h
HEADERS += hierarchy_prune.h
HEADERS += fastmarching_dt.h
HEADERS += my_surf_objs.h

SOURCES	+= ENT_plugin.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += vn_imgpreprocess.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(ENT)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/ENT/
