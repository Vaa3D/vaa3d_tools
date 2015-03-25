
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= Shrink_plugin.h \
    q_morphology3D.h
SOURCES	+= Shrink_plugin.cpp \
    q_morphology3D.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Shrink)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/CASBiophysc_Shrink/
