TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINPATH = ../../../v3d_main
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib

HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += swc_to_maskimage.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += swc_to_maskimage.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET        = $$qtLibraryTarget(swc_to_maskimage)
DESTDIR       = ../../v3d/plugins/neuron_utilities/swc_to_maskimage_sphere_unit
