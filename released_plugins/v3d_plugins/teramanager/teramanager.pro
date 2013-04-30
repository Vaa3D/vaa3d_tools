#TeraManager plugin project file
TEMPLATE    = lib
CONFIG  += qt plugin warn_off
QT += opengl
QT += network
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
HEADERS += $$V3DMAINPATH/common_lib/src_packages/mylib_tiff/image.h \
    src/control/V3Dsubclasses.h \
    src/presentation/QLineTree.h
HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h
LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/common_lib/lib \
            -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib

#Vaa3D sources needed to specialize Vaa3D into subclasses
INCLUDEPATH += $$V3DMAINPATH
INCLUDEPATH += $$V3DMAINPATH/v3d
#SOURCES += $$V3DMAINPATH/3drenderer/renderer.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/GLee_r.c
#SOURCES += $$V3DMAINPATH/3drenderer/renderer_hit.cpp
#SOURCES += $$V3DMAINPATH/v3d/landmark_property_dialog.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_landmark_property_dialog.cpp
#SOURCES += $$V3DMAINPATH/v3d/surfaceobj_geometry_dialog.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_surfaceobj_geometry_dialog.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_surfaceobj_annotation_dialog.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat1.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat2.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat3.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat4.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat5.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat6.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat7.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat8.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmat9.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/newmatex.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/v3dr_glwidget.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_v3dr_glwidget.cpp
##additional sources required on MacOS X
#SOURCES += $$V3DMAINPATH/v3d/my4dimage.cpp
#SOURCES += $$V3DMAINPATH/neuron_tracing/dij_bgl.cpp
#SOURCES += $$V3DMAINPATH/v3d/v3d_core.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/submat.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/svd.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/barFigureDialog.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/renderer_obj.cpp
#SOURCES += $$V3DMAINPATH/v3d/mainwindow.cpp
#SOURCES += $$V3DMAINPATH/imaging/v3d_imaging.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/nstroke_tracing.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/nstroke.cpp
#SOURCES += $$V3DMAINPATH/neuron_editing/neuron_sim_scores.cpp
#SOURCES += $$V3DMAINPATH/v3d/v3dimg_proc_neuron.cpp
#SOURCES += $$V3DMAINPATH/neuron_toolbox/vaa3d_neurontoolbox.cpp
#SOURCES += $$V3DMAINPATH/neuron_editing/neuron_xforms.cpp
#SOURCES += $$V3DMAINPATH/neuron_editing/apo_xforms.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/myexcept.cpp
#SOURCES += $$V3DMAINPATH/jba/newmat11/bandmat.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/v3dr_surfaceDialog.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/v3dr_colormapDialog.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_v3dr_surfaceDialog.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_v3dr_colormapDialog.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/renderer_tex.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/renderer_gl2.cpp

#SOURCES += $$V3DMAINPATH/3drenderer/renderer_labelfield.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_ChannelTable.cpp
#SOURCES += $$V3DMAINPATH/v3d/moc_v3d_core.cpp
#SOURCES += $$V3DMAINPATH/3drenderer/hoverpoints.cpp


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
