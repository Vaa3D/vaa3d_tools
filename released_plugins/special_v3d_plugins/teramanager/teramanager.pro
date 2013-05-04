#generic set up
TEMPLATE    = lib
CONFIG  += qt plugin warn_off
QT += opengl
CONFIG += x86_64
#CONFIG += use_static_libs
#CONFIG += use_experimental_features

#set up third party libraries
use_static_libs{
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
} else{
    #OpenCV headers and library folders
    INCLUDEPATH += /usr/local/include/opencv
    LIBS+= -L/usr/local/lib
}
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui


#set up Vaa3D stuff needed by the plugin
V3DMAINPATH =  ../../../v3d_main
QT_PATH = $$dirname(QMAKE_QMAKE)/..
INCLUDEPATH+= $$QT_PATH/demos/shared
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
HEADERS += $$V3DMAINPATH/common_lib/src_packages/mylib_tiff/image.h \
    src/control/V3Dsubclasses.h \
    src/presentation/QLineTree.h
HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib \
            -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib

#Vaa3D sources needed to use experimental features
use_experimental_features{
    DEFINES += USE_EXPERIMENTAL_FEATURES
    INCLUDEPATH += $$V3DMAINPATH
    INCLUDEPATH += $$V3DMAINPATH/v3d
    SOURCES += $$V3DMAINPATH/3drenderer/renderer.cpp
    SOURCES += $$V3DMAINPATH/3drenderer/GLee_r.c
    SOURCES += $$V3DMAINPATH/3drenderer/renderer_hit.cpp
    SOURCES += $$V3DMAINPATH/v3d/landmark_property_dialog.cpp
    SOURCES += $$V3DMAINPATH/v3d/moc_landmark_property_dialog.cpp
    SOURCES += $$V3DMAINPATH/v3d/surfaceobj_geometry_dialog.cpp
    SOURCES += $$V3DMAINPATH/v3d/moc_surfaceobj_geometry_dialog.cpp
    SOURCES += $$V3DMAINPATH/v3d/moc_surfaceobj_annotation_dialog.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat1.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat2.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat3.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat4.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat5.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat6.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat7.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat8.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmat9.cpp
    SOURCES += $$V3DMAINPATH/jba/newmat11/newmatex.cpp
    SOURCES += $$V3DMAINPATH/3drenderer/v3dr_glwidget.cpp
    SOURCES += $$V3DMAINPATH/v3d/moc_v3dr_glwidget.cpp
}


#set up plugin
RESOURCES += icons.qrc
HEADERS += ./src/control/*.h
HEADERS += ./src/presentation/*.h
HEADERS += ./src/core/ImageManager/*.h
HEADERS += ./src/core/VolumeConverter/*.h
SOURCES += ./src/control/*.cpp
SOURCES += ./src/presentation/*.cpp
SOURCES += ./src/core/ImageManager/*.cpp
SOURCES += ./src/core/VolumeConverter/*.cpp


#set up target
TARGET	= $$qtLibraryTarget(teramanagerplugin)
DESTDIR	= ../../../bin/plugins/teramanager
