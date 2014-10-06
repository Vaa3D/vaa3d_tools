TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3D_DIR = ../../../v3d_main
INCLUDEPATH	+= $$VAA3D_DIR/basic_c_fun
INCLUDEPATH     += $$VAA3D_DIR/common_lib/include

HEADERS	+= swc2mask_plugin.h
HEADERS += my_surf_objs.h
HEADERS += swc_convert.h
HEADERS += src/swc2mask.h
HEADERS += $$VAA3D_DIR/basic_c_fun/customary_structs/vaa3d_neurontoolbox_para.h
HEADERS += $$VAA3D_DIR/basic_c_fun/basic_surf_objs.h

SOURCES	+= swc2mask_plugin.cpp
SOURCES += my_surf_objs.cpp
SOURCES += swc_convert.cpp
SOURCES += $$VAA3D_DIR/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3D_DIR/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(swc2mask)
DESTDIR	= $$VAA3D_DIR/../bin/plugins/neuron_utilities/swc_to_maskimage_cylinder_unit/
