
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = E:/vaa3D/vaa3d_source/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= shrink_plugin.h \
    shrink_math.h \
    shrink_3dmorph.h \
    shrink.h \
    shrink_threshold.h
SOURCES	+= shrink_plugin.cpp \
    shrink_3dmorph.cpp \
    shrink.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(shrink)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/shrink/
