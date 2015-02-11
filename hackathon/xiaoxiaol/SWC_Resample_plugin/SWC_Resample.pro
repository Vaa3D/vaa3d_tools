
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= SWC_Resample_plugin.h
HEADERS += ../../../released_plugins/v3d_plugins/resample_swc/resampling.h

SOURCES	+= SWC_Resample_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(SWC_Resample)
DESTDIR	= $$VAA3DPATH/bin/plugins/SWC_Resample/
