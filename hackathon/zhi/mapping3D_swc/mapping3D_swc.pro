
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= mapping3D_swc_plugin.h
HEADERS += openSWCDialog.h


SOURCES	+= mapping3D_swc_plugin.cpp
SOURCES += openSWCDialog.cpp
SOURCES += ../neurontracing_mip/my_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(mapping3D_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/mapping3D_swc/
