
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += hdf5/include

HEADERS	+= ocp_2_vaa3d_plugin.h
SOURCES	+= ocp_2_vaa3d_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(ocp_2_vaa3d)
DESTDIR	= $$VAA3DPATH/bin/plugins/ocp_2_vaa3d/

LIBS += -L/usr/local/lib -lboost_filesystem -lboost_system
LIBS += -Lhdf5/lib -lhdf5_cpp -lhdf5
