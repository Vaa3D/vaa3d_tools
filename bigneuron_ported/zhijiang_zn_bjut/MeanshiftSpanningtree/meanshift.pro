#an example plugin project file
TEMPLATE        = lib
CONFIG  += qt plugin warn_off
#QT += core gui widgets
#CONFIG += x86_64

#set the Vaa3D main path
V3DMAINPATH     =  ../../../../v3d_external/v3d_main

#include necessary paths
INCLUDEPATH     += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include

win32{
    LIBS +=  -L$$V3DMAINPATH/common_lib/winlib -llibtiff
}

macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
}



#include the headers used in the project
HEADERS += \
    meanshift_plugin.h \
    meanshift_func.h \
    node.h \
../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/heap.h\
$$V3DMAINPATH/neuron_editing/v_neuronswc.h

   
#include the source files used in the project
SOURCES = \
    meanshift_plugin.cpp \
    meanshift_func.cpp \
    node.cpp
SOURCES +=
SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp


#specify target name and directory
TARGET  = $$qtLibraryTarget(meanshift)
DESTDIR = $$V3DMAINPATH/../bin/plugins/BJUT_meanshift/
