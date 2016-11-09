
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= test1_plugin.h
HEADERS      += ..\..\..\released_plugins\v3d_plugins\sort_neuron_swc\openSWCDialog.h

SOURCES	+= test1_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES      += ..\..\..\released_plugins\v3d_plugins\sort_neuron_swc\openSWCDialog.cpp

TARGET	= $$qtLibraryTarget(test1)
DESTDIR	= $$VAA3DPATH/bin/plugins/test1/

