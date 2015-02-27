
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= combine_file_plugin.h
SOURCES	+= combine_file_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp



TARGET	= $$qtLibraryTarget(combine_file)
DESTDIR	= $$VAA3DPATH/../bin/plugins/combine_file/
