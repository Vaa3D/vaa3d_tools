#an example plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += opengl

#CONFIG	+= x86_64

#set the Vaa3D main path
V3DMAINPATH     =  ../../v3d_main

#set Qt main path
#QT_PATH = $$dirname(QMAKE_QMAKE)/..

#set up Qt
#INCLUDEPATH+= $$QT_PATH/demos/shared

#include necessary paths
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib

#include the headers used in the project
HEADERS	+= example_plugin.h
HEADERS	+= example_func.h

#include the source files used in the project
SOURCES	= example_plugin.cpp
SOURCES	+= example_func.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

INCLUDEPATH += $$V3DMAINPATH/v3d
SOURCES += $$V3DMAINPATH/3drenderer/renderer_hit.cpp

#specify target name and directory
TARGET	= $$qtLibraryTarget(example)
DESTDIR	= ../../bin/plugins/example/
