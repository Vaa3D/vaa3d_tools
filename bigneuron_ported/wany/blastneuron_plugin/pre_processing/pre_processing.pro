
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG  -= bundle_off
VAA3DPATH = ../../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= pre_processing_plugin.h
SOURCES	+= pre_processing_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += pre_processing_main.cpp
SOURCES += align_axis.cpp
SOURCES += prune_short_branch.cpp
SOURCES += pca1.cpp


TARGET	= $$qtLibraryTarget(pre_processing)
DESTDIR	= $$VAA3DPATH/bin/plugins/blastneuron/pre_processing
