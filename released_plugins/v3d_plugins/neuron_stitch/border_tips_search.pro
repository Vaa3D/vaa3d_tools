
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
#INCLUDEPATH	+= $$VAA3DPATH/v3d_main/3drenderer/

HEADERS	+= border_tips_plugin.h \
    neuron_stitch_func.h \
    neuron_tipspicker_dialog.h
    #$$VAA3DPATH/v3d_main/3drenderer/v3dr_mainwindow.h
SOURCES	+= border_tips_plugin.cpp \
    neuron_stitch_func.cpp \
    neuron_tipspicker_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(search_border_tips)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_stitch/2_search_border_tips/
