
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/pengh/work/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= demo_opensurfacefile_plugin.h
SOURCES	+= demo_opensurfacefile_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(demo_opensurfacefile)
DESTDIR	= $$VAA3DPATH/bin/plugins/Vaa3D_PluginInterface_Demos/demo_opensurfacefile/
