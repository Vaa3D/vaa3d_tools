
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= linker_terafly_plugin.h
HEADERS	+= ../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.h

SOURCES	+= linker_terafly_plugin.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.cpp


TARGET	= $$qtLibraryTarget(linker_terafly)
DESTDIR	= $$V3DMAINPATH/bin/plugins/linker_file/linker_terafly/
