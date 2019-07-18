
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= git_neuron_plugin.h \
    git_neuron_func.h
SOURCES	+= git_neuron_plugin.cpp \
    git_neuron_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(git_neuron)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/git_neuron/
