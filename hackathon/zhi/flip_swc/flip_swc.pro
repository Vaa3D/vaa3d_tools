
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include

HEADERS	+= flip_swc_plugin.h
HEADERS	+= ../IVSCC_sort_swc/openSWCDialog.h
HEADERS	+= ../APP2_large_scale/readRawfile_func.h


SOURCES	+= flip_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../IVSCC_sort_swc/openSWCDialog.cpp
SOURCES	+= ../APP2_large_scale/readrawfile_func.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp


TARGET	= $$qtLibraryTarget(flip_swc)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/flip_swc/
