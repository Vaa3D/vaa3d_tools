#last edit: by PHC, 20130722. make sure the static link of opencv works

#generic set up

TEMPLATE    = lib
CONFIG  += qt plugin warn_off
QT += opengl
#CONFIG += x86_64
CONFIG += use_static_libs
CONFIG += use_experimental_features
DEFINES += _VAA3D_PLUGIN_MODE
#DEFINES += _USE_QT_DIALOGS


#QMAKE_CXXFLAGS += -Wall
#QMAKE_CXXFLAGS += -pedantic
#QMAKE_CXXFLAGS += -Werror

#set up third party libraries
use_static_libs{
    INCLUDEPATH += ./include/opencv
    INCLUDEPATH += ./include
    mac{
    LIBS += -L./lib/opencv/mac_x86_64
    LIBS += -L./lib/opencv/mac_x86_64/3rdparty
    }
    unix:!mac{
    LIBS += -L./lib/opencv/unix_x86_64
    LIBS += -L./lib/opencv/unix_x86_64/3rdparty
    }
    win32{
    LIBS += -L./lib/opencv/win64
    }
} else{
    #OpenCV headers and library folders
    INCLUDEPATH += /usr/local/include/opencv
    LIBS+= -L/usr/local/lib
}
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui \
 -lzlib \
 -llibtiff \
 -llibjpeg \
 -lIlmImf \
 -llibjasper 


#to solve GTK libraries issue with OpenCV on linux
unix:!mac{
        CONFIG += link_pkgconfig
        PKGCONFIG += gtk+-2.0
        PKGCONFIG += gmodule-2.0
}

#set up Vaa3D stuff needed by the plugin
V3DMAINPATH =  ../../../v3d_main
QT_PATH = $$dirname(QMAKE_QMAKE)/..

INCLUDEPATH+= $$QT_PATH/demos/shared
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/3drenderer
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun/customary_structs

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

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

    #undefined symbols referenced from Vaa3D objects are ignored and checked dynamically at execution time
    mac{
        LIBS += -undefined dynamic_lookup
    }
}


#set up plugin
DEFINES += _VAA3D_PLUGIN_MODE
RESOURCES += icons.qrc
HEADERS += ./src/control/V3Dsubclasses.h
HEADERS += ./src/control/CAnnotations.h
HEADERS += ./src/control/CConverter.h
HEADERS += ./src/control/CExplorerWindow.h
HEADERS += ./src/control/CImport.h
HEADERS += ./src/control/CPlugin.h
HEADERS += ./src/control/CSettings.h
HEADERS += ./src/control/CVolume.h
HEADERS += ./src/control/V3Dsubclasses.h
HEADERS += ./src/presentation/PConverter.h
HEADERS += ./src/presentation/PDialogImport.h
HEADERS += ./src/presentation/PMain.h
HEADERS += ./src/presentation/QArrowButton.h
HEADERS += ./src/presentation/QGradientBar.h
HEADERS += ./src/presentation/QHelpBox.h
HEADERS += ./src/presentation/QLineTree.h
HEADERS += ./src/presentation/PAbout.h
HEADERS += ./src/presentation/PLog.h
HEADERS += ./src/core/ImageManager/ProgressBar.h
HEADERS += ./src/core/ImageManager/RawFmtMngr.h
HEADERS += ./src/core/ImageManager/RawVolume.h
HEADERS += ./src/core/ImageManager/SimpleVolume.h
HEADERS += ./src/core/ImageManager/SimpleVolumeRaw.h
HEADERS += ./src/core/ImageManager/Stack.h
HEADERS += ./src/core/ImageManager/StackRaw.h
HEADERS += ./src/core/ImageManager/StackedVolume.h
HEADERS += ./src/core/ImageManager/VirtualVolume.h
HEADERS += ./src/core/ImageManager/TiledVolume.h
HEADERS += ./src/core/ImageManager/TiledMCVolume.h
HEADERS += ./src/core/ImageManager/Block.h
HEADERS += ./src/core/VolumeConverter/VolumeConverter.h
HEADERS += ./src/core/VolumeConverter/check_and_makedir.h


SOURCES += ./src/control/CAnnotations.cpp
SOURCES += ./src/control/CConverter.cpp
SOURCES += ./src/control/CExplorerWindow.cpp
SOURCES += ./src/control/CImport.cpp
SOURCES += ./src/control/CPlugin.cpp
SOURCES += ./src/control/CSettings.cpp
SOURCES += ./src/control/CVolume.cpp
SOURCES += ./src/control/V3Dsubclasses.cpp
SOURCES += ./src/presentation/PConverter.cpp
SOURCES += ./src/presentation/PDialogImport.cpp
SOURCES += ./src/presentation/PMain.cpp
SOURCES += ./src/presentation/PAbout.cpp
SOURCES += ./src/presentation/PLog.cpp
SOURCES += ./src/presentation/QArrowButton.cpp
SOURCES += ./src/presentation/QGradientBar.cpp
SOURCES += ./src/presentation/QHelpBox.cpp
SOURCES += ./src/presentation/QLineTree.cpp
SOURCES += ./src/core/ImageManager/ProgressBar.cpp
SOURCES += ./src/core/ImageManager/RawFmtMngr.cpp
SOURCES += ./src/core/ImageManager/RawVolume.cpp
SOURCES += ./src/core/ImageManager/SimpleVolume.cpp
SOURCES += ./src/core/ImageManager/SimpleVolumeRaw.cpp
SOURCES += ./src/core/ImageManager/Stack.cpp
SOURCES += ./src/core/ImageManager/StackRaw.cpp
SOURCES += ./src/core/ImageManager/StackedVolume.cpp
SOURCES += ./src/core/ImageManager/VirtualVolume.cpp
SOURCES += ./src/core/ImageManager/TiledVolume.cpp
SOURCES += ./src/core/ImageManager/TiledMCVolume.cpp
SOURCES += ./src/core/ImageManager/Block.cpp
SOURCES += ./src/core/VolumeConverter/VolumeConverter.cpp
SOURCES += ./src/core/VolumeConverter/check_and_makedir.cpp


#set up target
TARGET	= $$qtLibraryTarget(teramanagerplugin)
DESTDIR	= ../../../bin/plugins/teramanager
