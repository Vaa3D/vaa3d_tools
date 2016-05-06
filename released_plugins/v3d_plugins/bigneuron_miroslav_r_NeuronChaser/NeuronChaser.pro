
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= NeuronChaser_plugin.h \
    btracer.h \
    nf_dialog.h \
    toolbox.h \
    model.h \
    node.h \
    connected.h
SOURCES	+= NeuronChaser_plugin.cpp \
    btracer.cpp \
    toolbox.cpp \
    model.cpp \
    node.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(NeuronChaser)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/NeuronChaser/
