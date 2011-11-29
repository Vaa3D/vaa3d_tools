
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun

HEADERS	= v3dneuron_tracing_plugin.h
HEADERS	+= v3dneuron_tracing_func.h
HEADERS += ../../../v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES	= v3dneuron_tracing_plugin.cpp
SOURCES	+= v3dneuron_tracing_func.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(v3dneuron_tracing)
DESTDIR	= ~/Applications/v3d/plugins/v3dneuron_tracing/
