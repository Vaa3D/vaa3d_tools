#terastitcher plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

#-----------------------------------------------------------------------------------
#  BEGIN SECTION TO BE MODIFIED IN ORDER TO COMPILE THE PLUGIN.
#  What you need before compiling:
#   1. OpenCV shared libraries installed and OpenCV headers
#   2. Qt >= 4.7.x (5.x should work too but it has not been tested) 
#-----------------------------------------------------------------------------------

#set Vaa3D main path
V3DMAINPATH =  ../../v3d_main

#set TeraStitcher main path
TERASTITCHER_PATH = ./core

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

#set up TeraStitcher tool
DEFINES += _VAA3D_PLUGIN_MODE
INCLUDEPATH += $$TERASTITCHER_PATH/TinyXML
INCLUDEPATH += $$TERASTITCHER_PATH/CrossMIPs
INCLUDEPATH += $$TERASTITCHER_PATH/IOManager
INCLUDEPATH += $$TERASTITCHER_PATH/VolumeManager
INCLUDEPATH += $$TERASTITCHER_PATH/Stitcher
SOURCES += $$TERASTITCHER_PATH/TinyXML/*.cpp
SOURCES += $$TERASTITCHER_PATH/CrossMIPs/*.cpp
SOURCES += $$TERASTITCHER_PATH/IOManager/*.cpp
SOURCES += $$TERASTITCHER_PATH/VolumeManager/*.cpp
SOURCES += $$TERASTITCHER_PATH/Stitcher/*.cpp

#set up TeraStitcher plugin
HEADERS += control/*.h
HEADERS += presentation/*.h
SOURCES += control/*.cpp
SOURCES += presentation/*.cpp
TARGET	= $$qtLibraryTarget(terastitcherplugin)
DESTDIR	= ../../bin/plugins/image_stitching/terastitcher
RESOURCES += \
    icons.qrc
