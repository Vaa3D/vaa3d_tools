
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

HEADERS	+= extractZSlices_plugin.h
SOURCES	+= extractZSlices_plugin.cpp

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp

SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET	= $$qtLibraryTarget(extractZSlices)
DESTDIR = $$V3DMAINPATH/../bin/plugins/image_geometry/extract_Z_Slices/

