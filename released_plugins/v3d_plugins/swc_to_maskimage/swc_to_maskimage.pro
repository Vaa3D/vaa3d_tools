TEMPLATE      = lib
CONFIG       += qt plugin warn_off

INCLUDEPATH  += ../../../v3d_main/basic_c_fun

HEADERS      +=../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += swc_to_maskimage.h

SOURCES      +=../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += swc_to_maskimage.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(swc_to_maskimage)
DESTDIR       = ../../v3d/plugins/neuron_utilities/swc_to_maskimage_sphere_unit
