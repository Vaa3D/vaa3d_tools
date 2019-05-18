
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= BrainVesselCPR_plugin.h \
    BrainVesselCPR_gui.h \
    BrainVesselCPR_filter.h \


SOURCES	+= BrainVesselCPR_plugin.cpp \
    BrainVesselCPR_filter.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(BrainVesselCPR)
DESTDIR	= $$VAA3DPATH/bin/plugins/BrainVesselCPR/
