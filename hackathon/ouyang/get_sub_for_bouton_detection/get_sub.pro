
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= get_sub_plugin.h
HEADERS	+= loc_input.h
HEADERS	+= my_surf_objs.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES	+= get_sub_plugin.cpp
SOURCES	+= loc_input.cpp
SOURCES	+= my_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(get_sub)
DESTDIR	= $$VAA3DPATH/bin/plugins/Ouyang/get_sub/
