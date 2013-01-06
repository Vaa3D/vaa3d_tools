#TeraManager plugin project file
TEMPLATE    = lib
CONFIG  += qt plugin warn_off
QT += opengl

#-----------------------------------------------------------------------------------
#  BEGIN SECTION TO BE MODIFIED IN ORDER TO COMPILE THE PLUGIN.
#  What you need before compiling:
#   1. OpenCV >= 2.2.x (both shared libraries and headers)
#   2. Qt >= 4.7.x (5.x should work too but it has not been tested) 
#-----------------------------------------------------------------------------------

#set Vaa3D main path
V3DMAINPATH =  ../../v3d_main

#set Qt main path

QT_PATH = $$dirname(QMAKE_QMAKE)/..

#set up OpenCV library (please modify include and lib paths if necessary)
INCLUDEPATH += /usr/local/include/opencv
LIBS+= -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui

#-----------------------------------------------------------------------------------
#  END SECTION TO BE MODIFIED IN ORDER TO COMPILE THE PLUGIN.
#-----------------------------------------------------------------------------------

#set TeraManager main path
TERAMANAGER_PATH = "./core"

#set up Qt
INCLUDEPATH+= $$QT_PATH/demos/shared

#set up Vaa3D plugin
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/3drenderer
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/common_lib/src_packages/mylib_tiff
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun/customary_structs
SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.cpp
HEADERS += $$V3DMAINPATH/common_lib/src_packages/mylib_tiff/image.h \
    control/CConverter.h
HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h

#needed to enable the "double-click zoom-in" feature --> removed, simple inclusion impedes the plugin to be loaded
#SOURCES += $$V3DMAINPATH/3drenderer/renderer_hit.cpp

LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib \
            -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib

#set up TeraManager tool
INCLUDEPATH += $$TERAMANAGER_PATH/ImageManager
INCLUDEPATH += $$TERAMANAGER_PATH/VolumeConverter
SOURCES += $$TERAMANAGER_PATH/ImageManager/*.cpp
SOURCES += $$TERAMANAGER_PATH/VolumeConverter/*.cpp

#set up TeraManager plugin
HEADERS += control/*.h
HEADERS += presentation/*.h
SOURCES += control/*.cpp
SOURCES += presentation/*.cpp
TARGET	= $$qtLibraryTarget(teramanagerplugin)
DESTDIR	= ../../bin/plugins/teramanager
RESOURCES += \
    icons.qrc
