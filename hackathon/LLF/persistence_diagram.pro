#an example plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

#set the Vaa3D main path
V3DMAINPATH     =  ../../../../v3d_external/v3d_main

#include necessary paths
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
LIBS += -L$$V3DMAINPATH/common_lib/lib

#include the headers used in the project
HEADERS	+= persistence_diagram_plugin.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h

#include the source files used in the project
SOURCES	= persistence_diagram_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

#specify target name and directory
TARGET	= $$qtLibraryTarget(persistence_diagram)
DESTDIR	= ../../../../vaa3d_tools/bin/plugins/persistence_diagram/