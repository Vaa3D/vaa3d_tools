#TeraManager plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

#-----------------------------------------------------------------------------------
#  BEGIN SECTION TO BE MODIFIED IN ORDER TO COMPILE THE PLUGIN.
#  What you need before compiling:
#   1. TeraManager source code (ask to a.bria@unicas.it or g.iannello@unicampus.it)
#      or visit website http://www.iconfoundation.net/
#   2. OpenCV shared libraries installed and OpenCV headers
#   3. Qt >= 4.7.x (5.x should work too but it has not been tested) 
#-----------------------------------------------------------------------------------

#set Vaa3D main path
V3DMAINPATH =  ../../v3d_main

#set TeraManager main path
TERAMANAGER_PATH = "./core"

#set up OpenCV library (please modify include and lib paths if necessary)
INCLUDEPATH += /usr/local/include/opencv
LIBS+= -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui

#-----------------------------------------------------------------------------------
#  END SECTION TO BE MODIFIED IN ORDER TO COMPILE THE PLUGIN.
#-----------------------------------------------------------------------------------

#set up Vaa3D plugin
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage.cpp
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib

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
