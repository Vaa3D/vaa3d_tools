
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= morphoQC_post_plugin.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    morphoqc_func.h
SOURCES	+= morphoQC_post_plugin.cpp \
    morphoqc_func.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(morphoQC_post)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/morphoQC_post/
