
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include


HEADERS	+= \
    readtree_plugin.h \
    openSWCDialog.h \
    readmain.h
SOURCES	+= \
    readtree_plugin.cpp \
    openSWCDialog.cpp \
    readmain.cpp

# ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(readtree)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_analysis/readtree/

#OTHER_FILES +=
