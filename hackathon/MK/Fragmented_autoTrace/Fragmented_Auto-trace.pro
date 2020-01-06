
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_CXXFLAGS += /MP
#CONFIG	+= x86_64

VAA3DPATH = ../../../../v3d_external/v3d_main
IMGMANAGERPATH = ../v3d_imgManagerMK
STATSLEARNERPATH = ../StatsLearner
V3DTOOLPATH = ../../../released_plugins/v3d_plugins

INCLUDEPATH += $$IMGMANAGERPATH
INCLUDEPATH += $$IMGMANAGERPATH/imgAnalyzer
INCLUDEPATH += $$IMGMANAGERPATH/imgProcessor
INCLUDEPATH += ../NeuronStructNavigator
INCLUDEPATH += $$STATSLEARNERPATH
INCLUDEPATH += ./
INCLUDEPATH += $$V3DTOOLPATH/swc2mask_cylinder
INCLUDEPATH += $$VAA3DPATH/v3d
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/neuron_editing
INCLUDEPATH += $$VAA3DPATH/common_lib/include
INCLUDEPATH += $$VAA3DPATH/3drenderer

win32: {
    BOOSTPATH = $$(BOOST_PATH)
    QTPATH = $$(QTDIR)

    INCLUDEPATH += $$BOOSTPATH
    INCLUDEPATH += $$QTPATH/demos/shared
    INCLUDEPATH += $$QTPATH/include/QtOpenGL
    LIBS += -L$$BOOSTPATH/lib64-msvc-12.0

    LIBS += -L$$IMGMANAGERPATH -lv3d_imgManagerMK
    LIBS += -L../NeuronStructNavigator -lNeuronStructNavigator
}

FORMS += fragmentedTraceUI.ui

RESOURCES += FragTracer_Resource.qrc

HEADERS	+= Fragmented_Auto-trace_plugin.h
HEADERS += FragTraceControlPanel.h
HEADERS += FragTraceManager.h
HEADERS += $$V3DTOOLPATH/swc2mask_cylinder/my_surf_objs.h
HEADERS += FragTracer_Define.h
HEADERS += FragmentEditor.h

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= Fragmented_Auto-trace_plugin.cpp
SOURCES += FragTraceControlPanel.cpp
SOURCES += FragTraceManager.cpp
SOURCES += FragmentEditor.cpp

unix:!macx {
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -lNeuronStructNavigator
    LIBS += -L/usr/lib64/ -lboost_system
}

TARGET	= $$qtLibraryTarget(Fragmented_Auto-trace)
DESTDIR	= ../../../../v3d_external/bin/plugins/Fragmented_Auto-trace/
