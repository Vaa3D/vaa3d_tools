#terastitcher plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG += x86_64
CONFIG += use_static_libs

#set up third party libraries
use_static_libs{
    INCLUDEPATH += ../teramanager/include/opencv
    INCLUDEPATH += ../teramanager/include
    mac{
    LIBS += -L../teramanager/lib/opencv/mac_x86_64
    }
    unix:!mac{
    LIBS += -L../teramanager/lib/opencv/unix_x86_64
    }
    win32{
    LIBS += -L../teramanager/lib/opencv/win32
    }
} else{
    #OpenCV headers and library folders
    INCLUDEPATH += /usr/local/include/opencv
    LIBS+= -L/usr/local/lib
}
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui

#undefined symbols referenced from OpenCV are ignored and checked dynamically at execution time
mac{
    LIBS += -undefined dynamic_lookup
}


#set up Vaa3D stuff needed by the plugin
V3DMAINPATH =  ../../../v3d_main
QT_PATH = $$dirname(QMAKE_QMAKE)/..
INCLUDEPATH+= $$QT_PATH/demos/shared
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib \
            -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib

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


#set up target
TARGET	= $$qtLibraryTarget(terastitcherplugin)
DESTDIR	= ../../../bin/plugins/image_stitching/terastitcher
RESOURCES += icons.qrc
