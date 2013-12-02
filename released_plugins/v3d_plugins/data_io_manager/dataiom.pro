
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun

HEADERS	+= dataiom_plugin.h
HEADERS	+= dataiom_func.h
HEADERS	+= dataiom_gui.h

SOURCES	= dataiom_plugin.cpp
SOURCES	+= dataiom_func.cpp
SOURCES	+= dataiom_gui.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(data_io_manager)
DESTDIR        = ../../v3d/plugins/data_IO/data_IO_manager/

