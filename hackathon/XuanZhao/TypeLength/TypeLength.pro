
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= TypeLength_plugin.h \
    typelengthfunction.h
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h

SOURCES	+= TypeLength_plugin.cpp \
    typelengthfunction.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(TypeLength)
DESTDIR	= $$VAA3DPATH/bin/plugins/TypeLength/
