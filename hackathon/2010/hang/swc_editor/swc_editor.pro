
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun

HEADERS	= swc_editor_plugin.h
HEADERS	+= swc_editor_func.h
HEADERS	+= swc_editor_gui.h
HEADERS += v3d_monitor.h
HEADERS += ../../../v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES	= swc_editor_plugin.cpp
SOURCES	+= swc_editor_func.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(swc_editor)
DESTDIR	= ~/Applications/v3d/plugins/swc_editor/
