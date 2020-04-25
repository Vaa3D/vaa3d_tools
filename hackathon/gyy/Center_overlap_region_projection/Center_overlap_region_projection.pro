
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG += qaxcontainer
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Center_overlap_region_projection_plugin.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= Center_overlap_region_projection_plugin.cpp \
    overlap_region_projection.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Center_overlap_region_projection)
DESTDIR	= $$VAA3DPATH/bin/plugins/Center_overlap_region_projection/
