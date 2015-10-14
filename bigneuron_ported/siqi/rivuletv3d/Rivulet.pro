
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/siqi/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= Rivulet_plugin.h
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp
SOURCES	+= Rivulet_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.h
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp
HEADERS += fastmarching_dt.h
HEADERS += utils/msfm.h
SOURCES += utils/msfm.cpp
SOURCES += utils/common.c
HEADERS += rk4.h
SOURCES += rk4.cpp

TARGET	= $$qtLibraryTarget(Rivulet)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/Rivulet/
