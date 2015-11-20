
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= IVSCC_smoothing_swc_plugin.h
SOURCES	+= IVSCC_smoothing_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../eliminate_neuron_swc/my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(IVSCC_smoothing_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/IVSCC_smoothing_swc/
