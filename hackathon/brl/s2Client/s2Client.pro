
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3D_DIR = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3D_DIR/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/common_lib/include
QT           += network

HEADERS  = s2Client.h
HEADERS += s2Client_plugin.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/customary_structs/vaa3d_neurontoolbox_para.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES = s2Client_plugin.cpp 
SOURCES += s2Client.cpp 
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(s2Client)
DESTDIR	= $$VAA3D_DIR/bin/plugins/neuron_utilities/s2Client/
