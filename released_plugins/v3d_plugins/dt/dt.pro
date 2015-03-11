
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main 

HEADERS += dt.h
HEADERS += my_surf_objs.h

SOURCES  = dt.cpp
SOURCES += my_surf_objs.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(fast_dt)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_filters/Fast_Distance_Transform
