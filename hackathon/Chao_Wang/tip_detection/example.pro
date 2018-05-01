#an example plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

#set the Vaa3D main path
V3DMAINPATH     =  ../../../../v3d_external/v3d_main

#include necessary paths
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include

win32 {
    LIBS += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
}

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff

}

#include the headers used in the project
HEADERS	+= example_plugin.h
HEADERS	+= example_func.h
HEADERS	+= example_ray.h

#include the source files used in the project
SOURCES	= example_plugin.cpp
SOURCES	+= example_func.cpp
SOURCES	+= example_ray.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp


#specify target name and directory
TARGET	= $$qtLibraryTarget(example)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/Terminations/tip_detection/
