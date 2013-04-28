#TeraManager plugin project file
TEMPLATE    = lib
CONFIG  += qt plugin warn_off
QT += opengl
CONFIG += release
CONFIG += x86_64

#set Vaa3D main path
V3DMAINPATH =  ../../../v3d_main

#set Qt main path
QT_PATH = $$dirname(QMAKE_QMAKE)/..

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

#set up Vaa3D stuff needed by the plugin
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
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
HEADERS += $$V3DMAINPATH/common_lib/src_packages/mylib_tiff/image.h
HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib \
            -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib

#needed to enable the "double-click zoom-in" feature --> removed, simple inclusion impedes the plugin to be loaded
#INCLUDEPATH += $$V3DMAINPATH/v3d
#SOURCES += $$V3DMAINPATH/3drenderer/renderer_hit.cpp


#set up TeraManager plugin
HEADERS += ./src/control/*.h
HEADERS += ./src/presentation/*.h
HEADERS += ./src/core/ImageManager/*.h
HEADERS += ./src/core/VolumeConverter/*.h
SOURCES += ./src/control/*.cpp
SOURCES += ./src/presentation/*.cpp
SOURCES += ./src/core/ImageManager/*.cpp
SOURCES += ./src/core/VolumeConverter/*.cpp


TARGET	= $$qtLibraryTarget(teramanagerplugin)
DESTDIR	= ../../../bin/plugins/teramanager
RESOURCES += icons.qrc
