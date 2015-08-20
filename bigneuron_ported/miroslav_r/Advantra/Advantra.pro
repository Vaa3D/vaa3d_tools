
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/miroslav/vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= Advantra_plugin.h \
    nf_dialog.h \
    toolbox.h \
    node.h \
    btracer.h \
    connected.h
SOURCES	+= Advantra_plugin.cpp \
    btracer.cpp \
    node.cpp \
    toolbox.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Advantra)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/Advantra/
