
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= CheckSWC_plugin.h \
    my_surf_objs.h \
    main.h

SOURCES	+= CheckSWC_plugin.cpp \
    CheckSWC_plugin.cpp \
    my_surf_objs.cpp \
    main.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(CheckSWC)
DESTDIR	= $$VAA3DPATH/bin/plugins/Guo/CheckSWC/
