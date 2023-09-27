
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/penglab/GitRepo/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= SmartIndexing_plugin.h \
    volume_connection.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/color_xyz.h
SOURCES	+= SmartIndexing_plugin.cpp \
    volume_connection.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(SmartIndexing)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/SmartIndexing/
