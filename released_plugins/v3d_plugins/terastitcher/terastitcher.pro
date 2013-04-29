#terastitcher plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG += release
CONFIG += x86_64

#-----------------------------------------------------------------------------------
#  BEGIN SECTION TO BE MODIFIED IN ORDER TO COMPILE THE PLUGIN.
#  What you need before compiling:
#   1. OpenCV shared libraries installed and OpenCV headers
#   2. Qt >= 4.7.x (5.x should work too but it has not been tested) 
#-----------------------------------------------------------------------------------

#set Vaa3D main path
V3DMAINPATH =  ../../../v3d_main

#set up OpenCV (platform-dependent)
INCLUDEPATH += ./include/opencv
INCLUDEPATH += ./include
mac{
LIBS += -L./lib/opencv/mac_x86_64
}
unix:!mac{
LIBS += -L./lib/opencv/unix_x86_64
}
win32{
LIBS += -L./lib/opencv/win32
}
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui

#set up Qt
INCLUDEPATH+= $$QT_PATH/demos/shared

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
INCLUDEPATH += ./src/core/TinyXML
INCLUDEPATH += ./src/core/CrossMIPs
INCLUDEPATH += ./src/core/IOManager
INCLUDEPATH += ./src/core/VolumeManager
INCLUDEPATH += ./src/core/Stitcher
HEADERS += ./src/core/TinyXML/*.h
HEADERS += ./src/core/CrossMIPs/*.h
HEADERS += ./src/core/IOManager/*.h
HEADERS += ./src/core/VolumeManager/*.h
HEADERS += ./src/core/Stitcher/*.h
SOURCES += ./src/core/TinyXML/*.cpp
SOURCES += ./src/core/CrossMIPs/*.cpp
SOURCES += ./src/core/IOManager/*.cpp
SOURCES += ./src/core/VolumeManager/*.cpp
SOURCES += ./src/core/Stitcher/*.cpp
HEADERS += ./src/control/*.h
HEADERS += ./src/presentation/*.h
SOURCES += ./src/control/*.cpp
SOURCES += ./src/presentation/*.cpp



TARGET	= $$qtLibraryTarget(terastitcherplugin)
DESTDIR	= ../../../bin/plugins/image_stitching/terastitcher
RESOURCES += icons.qrc
