
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= test_app2_plugin.h \
    function_.h \
    block.h \
    app2_block.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h \

SOURCES	+= test_app2_plugin.cpp \
    function_.cpp \
    block.cpp \
    app2_block.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(test_app2)
DESTDIR	= $$VAA3DPATH/bin/plugins/test_app2/
