
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc

HEADERS	+= morphoQC_post_plugin.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    morphoqc_func.h \
    soma_confirmation.h \
    topo_validation.h \
    type_check.h
SOURCES	+= morphoQC_post_plugin.cpp \
    morphoqc_func.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp \
    soma_confirmation.cpp \
    topo_validation.cpp \
    type_check.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(morphoQC_post)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/morphoQC_post/

