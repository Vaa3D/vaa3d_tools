
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= IVSCC_scaling_plugin.h
SOURCES	+= IVSCC_scaling_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(IVSCC_scaling)
DESTDIR	=  $$VAA3DPATH/../bin/plugins/IVSCC/IVSCC_scaling/
