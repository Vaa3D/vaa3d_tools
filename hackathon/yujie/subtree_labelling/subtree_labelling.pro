
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= subtree_labelling_plugin.h \
    subtree_dialog.h
SOURCES	+= subtree_labelling_plugin.cpp \
    subtree_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(subtree_labelling)
DESTDIR	= $$VAA3DPATH/bin/plugins/subtree_labelling/
