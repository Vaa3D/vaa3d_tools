
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Hackathon_demo_plugin.h \
    some_function.h\
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h \
    some_class.h
SOURCES	+= Hackathon_demo_plugin.cpp \
    some_function.cpp \
    some_class.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(Hackathon_demo)
DESTDIR	= $$VAA3DPATH/bin/plugins/Hackathon_demo/
