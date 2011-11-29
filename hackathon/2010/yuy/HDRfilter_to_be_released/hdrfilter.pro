
TEMPLATE      = lib
CONFIG       += qt plugin warn_off

QT += opengl

V3DMAINDIR = ../../../v3d_main

# cross-OS-platform, cross-Qt-version
QT_DIR = $$dirname(QMAKE_QMAKE)/..  # cross-Qt-version

SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets

INCLUDEPATH  += $$SHARED_FOLDER	
INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
INCLUDEPATH  += $$QT_DIR
INCLUDEPATH  += $$V3DMAINDIR/v3d

macx {
    # Mac possible location of arthurwidgets.h with official Qt 4.7 installer
    QTINST_SHARED_FOLDER = /Developer/Examples/Qt/Demos/shared
    include($$QTINST_SHARED_FOLDER/shared.pri)
    INCLUDEPATH += $$QTINST_SHARED_FOLDER
    LIBS += -L$$QTINST_SHARED_FOLDER
}

HEADERS      += hdrfilter.h
HEADERS      += $$V3DMAINDIR/neuron_annotator/gui/NaZStackWidget.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h

HEADERS      += $$V3DMAINDIR/neuron_annotator/gui/BrightnessCalibrator.h
HEADERS      += $$V3DMAINDIR/neuron_annotator/gui/NaViewer.h
HEADERS      += $$V3DMAINDIR/neuron_annotator/gui/Na2DViewer.h
HEADERS      += $$V3DMAINDIR/neuron_annotator/gui/MouseClickManager.h
HEADERS      += $$V3DMAINDIR/neuron_annotator/geometry/CameraModel.h
HEADERS      += $$V3DMAINDIR/neuron_annotator/geometry/Rotation3D.h

SOURCES      += hdrfilter.cpp
SOURCES      += $$V3DMAINDIR/neuron_annotator/gui/NaZStackWidget.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp

SOURCES      += $$V3DMAINDIR/neuron_annotator/gui/BrightnessCalibrator.cpp
SOURCES      += $$V3DMAINDIR/neuron_annotator/gui/NaViewer.cpp
SOURCES      += $$V3DMAINDIR/neuron_annotator/gui/Na2DViewer.cpp
SOURCES      += $$V3DMAINDIR/neuron_annotator/gui/MouseClickManager.cpp
SOURCES      += $$V3DMAINDIR/neuron_annotator/geometry/CameraModel.cpp
SOURCES      += $$V3DMAINDIR/neuron_annotator/geometry/Rotation3D.cpp

LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -L$$SHARED_FOLDER 

TARGET        = $$qtLibraryTarget(hdrfilter)
DESTDIR       = ../../v3d/plugins/image_filters/HDR_Filter

