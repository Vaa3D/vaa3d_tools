TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH = ../../../v3d_main
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h \
    filter_dialog.h
HEADERS      += swc_to_maskimage.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp \
    filter_dialog.cpp
SOURCES      += swc_to_maskimage.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(swc_to_maskimage_sphere)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/neuron_utilities/swc_to_maskimage_sphere_unit
