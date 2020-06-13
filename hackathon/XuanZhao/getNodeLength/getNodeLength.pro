
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= getNodeLength_plugin.h \
    function.h
SOURCES	+= getNodeLength_plugin.cpp \
    function.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(getNodeLength)
DESTDIR	= $$VAA3DPATH/bin/plugins/getNodeLength/
