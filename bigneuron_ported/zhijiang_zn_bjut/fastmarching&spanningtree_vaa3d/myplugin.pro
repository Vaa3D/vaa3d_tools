#an example plugin project file
TEMPLATE        = lib
CONFIG  += qt plugin warn_off
#QT += core gui widgets
#CONFIG += x86_64

#set the Vaa3D main path
V3DMAINPATH     =  ../Vaa3D

#include necessary paths
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include
#LIBS +=  -L$$V3DMAINPATH/v3d_main/common_lib/winlib -llibtiff

#include the headers used in the project
HEADERS += \
    myplugin_func.h \
    myplugin_plugin.h \
    global.h \
    ../APP2_ported/heap.h \
    tree.h \
    graph.h \
    node.h
HEADERS +=

#include the source files used in the project
SOURCES = \
    myplugin_func.cpp \
    myplugin_plugin.cpp
SOURCES +=
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_memory.cpp


#specify target name and directory
TARGET  = $$qtLibraryTarget(myplugin)
DESTDIR = ../Vaa3D/bin/plugins/myplugin/
