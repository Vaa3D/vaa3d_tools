
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11

LIBS            += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat

HEADERS	+= apTrace_plugin.h \
    imagectrl.h \
    mysurface.h \
    pca.h
SOURCES	+= apTrace_plugin.cpp \
    imagectrl.cpp \
    mysurface.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(apTrace)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/apTrace/
