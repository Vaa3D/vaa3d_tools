TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3D_DIR = /Users/xiaoh10/work/v3d_external
INCLUDEPATH	+= . $$VAA3D_DIR/v3d_main/basic_c_fun
LIBS += -L$$VAA3D_DIR/v3d_main/common_lib/lib -lv3dtiff

HEADERS	+= swc2mask_plugin.h
HEADERS += my_surf_objs.h
HEADERS += swc_convert.h
HEADERS += src/swc2mask.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/customary_structs/vaa3d_neurontoolbox_para.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/stackutil.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/mg_utilities.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/mg_image_lib.h

SOURCES += my_surf_objs.cpp
SOURCES += swc_convert.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/stackutil.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/mg_utilities.cpp

TARGET	= $$qtLibraryTarget(swc2mask)
DESTDIR	= $$VAA3D_DIR/bin/plugins/swc2mask/
