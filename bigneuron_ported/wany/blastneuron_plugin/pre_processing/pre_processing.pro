# pre-processing module for BlastNeuron
# by Yinan Wan
# 20140113

TEMPLATE = app
CONFIG += qt warn_off
CONFIG -= app_bundle

V3DMAINPATH = ../../../v3d_external/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun

HEADERS += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += prune_short_branch.h
HEADERS += align_axis.h
HEADERS += pca1.h
HEADERS += resampling.h

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += main.cpp
SOURCES += prune_short_branch.cpp
SOURCES += align_axis.cpp
SOURCES += pca1.cpp

