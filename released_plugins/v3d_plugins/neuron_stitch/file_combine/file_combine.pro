
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += ../
#INCLUDEPATH	+= $$VAA3DPATH/v3d_main/3drenderer/

HEADERS	+= file_combine_plugin.h \
    ../neuron_stitch_func.h
    #$$VAA3DPATH/v3d_main/3drenderer/v3dr_mainwindow.h
SOURCES	+= file_combine_plugin.cpp \
    ../neuron_stitch_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(file_combine)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_stitch/4_file_combine/
