
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/v3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Hackathon_demo1_plugin.h \
    some_class.h \
    some_function.h
SOURCES	+= Hackathon_demo1_plugin.cpp \
    some_class.cpp \
    some_function.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Hackathon_demo1)
DESTDIR	= $$VAA3DPATH/bin/plugins/Hackathon_demo1/
