
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG += qaxcontainer
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= define_Boundary_plugin.h \
    point_class.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= define_Boundary_plugin.cpp \
    calculate_Boundary.cpp \
    point_class.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(define_Boundary)
DESTDIR	= $$VAA3DPATH/bin/plugins/define_Boundary/
