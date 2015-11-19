
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include


HEADERS	+= my_surf_objs.h
HEADERS	+= marker_radius.h
SOURCES += my_surf_objs.cpp

HEADERS	+= IVSCC_radius_estimation_plugin.h
SOURCES	+= IVSCC_radius_estimation_plugin.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(IVSCC_radius_estimation)
DESTDIR	= $$VAA3DPATH/bin/plugins/IVSCC_radius_estimation/
