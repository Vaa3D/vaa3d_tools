
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= linker_terafly_plugin.h
HEADERS	+= ../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.h

SOURCES	+= linker_terafly_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.cpp


TARGET	= $$qtLibraryTarget(linker_terafly)
DESTDIR	= $$VAA3DPATH/bin/plugins/linker_file/linker_terafly/
