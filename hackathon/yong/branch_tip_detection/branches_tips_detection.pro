
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= branches_tips_detection_plugin.h \
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h \
    branch_tip_detection.h \
    my_surf_objs.h
SOURCES	+= branches_tips_detection_plugin.cpp \
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp \
    branch_tip_detection.cpp \
    my_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(branches_tips_detection)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/branches_tips_detection/
