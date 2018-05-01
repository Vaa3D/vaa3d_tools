
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/lxf/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= TMD_plugin.h \
    preprocess.h \
    createTMD.h
SOURCES	+= TMD_plugin.cpp \
    preprocess.cpp \
    createTMD.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
TARGET	= $$qtLibraryTarget(TMD)
DESTDIR	= $$VAA3DPATH/bin/plugins/TMD/
