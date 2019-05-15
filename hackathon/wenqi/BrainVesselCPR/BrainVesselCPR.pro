
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= BrainVesselCPR_plugin.h


SOURCES	+= BrainVesselCPR_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BrainVesselCPR)
DESTDIR	= $$VAA3DPATH/bin/plugins/BrainVesselCPR/
