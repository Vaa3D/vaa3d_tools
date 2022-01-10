
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
QT += widgets
HEADERS	+= linker_terafly_plugin.h

HEADERS	+= ../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.h

SOURCES	+= linker_terafly_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.cpp

TARGET	= $$qtLibraryTarget(linker_terafly)
DESTDIR	= $$VAA3DPATH/../bin/plugins/linker_file/linker_terafly/
