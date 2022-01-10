
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =  ../../../../v3d_external/v3d_main
QT += widgets
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
QT += widgets
HEADERS	= node_connect_plugin.h
HEADERS += $$VAA3DPATH/basic_c_fun/v3d_interface.h \
    openSWCDialog.h \
    my_surf_objs.h \
    node_connect_func.h


SOURCES	= node_connect_plugin.cpp

SOURCES += openSWCDialog.cpp \
    node_connect_func.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(node_connect)
DESTDIR = $$VAA3DPATH/../bin/plugins/neuron_utilities/node_connect/
#DESTDIR = C:\Users\rummig\Desktop\Vaa3d_allfiles\vaa3d_win7_32bit_v2.707\plugins\node_connect
