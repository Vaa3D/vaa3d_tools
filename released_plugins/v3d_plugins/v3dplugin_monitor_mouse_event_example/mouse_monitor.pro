
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun

HEADERS	+= mouse_monitor.h
HEADERS	+= mouse_monitor_plugin.h
HEADERS	+= mouse_monitor_func.h
HEADERS	+= mouse_monitor_gui.h

SOURCES	= mouse_monitor_plugin.cpp
SOURCES	+= mouse_monitor_func.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(mouse_monitor)
DESTDIR = ../../../../v3d_external/bin/plugins/Vaa3D_PluginInterface_Demos/mouse_event_monitor
