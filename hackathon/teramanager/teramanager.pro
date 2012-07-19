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

#QT_PATH = "/usr/local/Trolltech/Qt-4.7.3"

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

#SOURCES += $$V3DMAINPATH/v3d/my4dimage.cpp              #if included, the plugin instantiation fails
SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.cpp

unix:HEADERS += $$V3DMAINPATH/common_lib/src_packages/mylib_tiff/image.h
unix:HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h 

LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib \
            -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib

#set up TeraManager tool
INCLUDEPATH += $$TERAMANAGER_PATH/ImageManager
SOURCES += $$TERAMANAGER_PATH/ImageManager/*.cpp

#set up TeraManager plugin
HEADERS += control/*.h
HEADERS += presentation/*.h
SOURCES += control/*.cpp
SOURCES += presentation/*.cpp
TARGET	= $$qtLibraryTarget(teramanagerplugin)
DESTDIR	= ../../bin/plugins/teramanager
RESOURCES += \
    icons.qrc
