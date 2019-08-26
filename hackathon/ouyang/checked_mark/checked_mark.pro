
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/braincenter10/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= checked_mark_plugin.h \
    change_type.h
SOURCES	+= checked_mark_plugin.cpp \
    change_type.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h

TARGET	= $$qtLibraryTarget(checked_mark)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_tracing/checked_mark/
