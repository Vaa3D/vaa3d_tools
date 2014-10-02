
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
#VAA3DPATH = C:\Users\rummig\Desktop\Vaa3d_allfiles\Vaa3d_Downloads\v3d_external
VAA3DPATH = ../../../../v3d_external

INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	= node_connect_plugin.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_interface.h \
    openSWCDialog.h \
    my_surf_objs.h \
    node_connect_func.h


SOURCES	= node_connect_plugin.cpp

SOURCES += openSWCDialog.cpp \
    node_connect_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(node_connect)
DESTDIR        = $$VAA3DPATH/bin/plugins/tmp/node_connect
#DESTDIR	= C:\Users\rummig\Desktop\Vaa3d_allfiles\vaa3d_win7_32bit_v2.707\plugins\node_connect
