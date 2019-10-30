
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
#VAA3DPATH = /Users/ding/v3d_external
VAA3DPATH= ../../../../v3d_external

INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing

HEADERS	+= adaboost_plugin.h
SOURCES	+= adaboost_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES     += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/neuron_editing/neuron_sim_scores.cpp

TARGET	= $$qtLibraryTarget(adaboost)
DESTDIR	= $$VAA3DPATH/bin/plugins/adaboost/
