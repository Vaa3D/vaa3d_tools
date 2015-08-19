
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/miroslav/vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= NeuronChaserX_plugin.h \
    nf_dialog.h \
    ../NeuronChaser/toolbox.h \
    toolbox.h \
    node.h \
    btracer.h \
    connected.h
SOURCES	+= NeuronChaserX_plugin.cpp \
    toolbox.cpp \
    node.cpp \
    btracer.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(NeuronChaserX)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/NeuronChaserX/
