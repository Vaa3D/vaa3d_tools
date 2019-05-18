
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= segment_block_plugin.h \
           my_surf_objs.h\
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h \
    segment_block.h \
    my_surf_objs.h

SOURCES	+= segment_block_plugin.cpp \
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp \
    segment_block.cpp \
    my_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(segment_block)
DESTDIR	= $$VAA3DPATH/bin/plugins/segment_block/
