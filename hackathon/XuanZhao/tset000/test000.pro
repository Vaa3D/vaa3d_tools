
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/v3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= test000_plugin.h \
    some_function.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= test000_plugin.cpp \
    some_function.cpp \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(test000)
DESTDIR	= $$VAA3DPATH/bin/plugins/test000/
