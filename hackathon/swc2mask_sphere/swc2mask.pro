
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3D_DIR = /Users/xiaoh10/work/v3d_external
INCLUDEPATH	+= $$VAA3D_DIR/v3d_main/basic_c_fun
LIBS += -L$$VAA3D_DIR/v3d_main/common_lib/lib -lv3dtiff

HEADERS	= raw_split_plugin.h
HEADERS += src/raw_split.h
HEADERS += stackutil.h
HEADERS += mg_utilities.h
HEADERS += mg_image_lib.h

SOURCES += src/raw_split.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/stackutil.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/mg_utilities.cpp

TARGET	= $$qtLibraryTarget(raw_split)
DESTDIR	= $$VAA3D_DIR/bin/plugins/raw_split/
