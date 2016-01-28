
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= soma_sorting_swc_plugin.h
SOURCES	+= soma_sorting_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(soma_sorting_swc)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/soma_sorting_swc/
