
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
#V3DMAINPATH = C:\Users\rummig\Desktop\Vaa3d_allfiles\Vaa3d_Downloads\v3d_external
V3DMAINPATH = ../../../../v3d_external

INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	= node_connect_plugin.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_interface.h \
    openSWCDialog.h \
    my_surf_objs.h \
    node_connect_func.h


SOURCES	= node_connect_plugin.cpp

SOURCES += openSWCDialog.cpp \
    node_connect_func.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(node_connect)
DESTDIR = $$V3DMAINPATH/bin/plugins/neuron_utilities/node_connect
#DESTDIR = C:\Users\rummig\Desktop\Vaa3d_allfiles\vaa3d_win7_32bit_v2.707\plugins\node_connect
