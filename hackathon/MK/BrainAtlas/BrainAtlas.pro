
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
unix:!macx {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_CXXFLAGS += /MP
}
#CONFIG	+= x86_64

VAA3DPATH = C:/Vaa3D_2013_Qt486/v3d_external/v3d_main
IMGMANAGERPATH = ../v3d_imgManagerMK
V3DTOOLPATH = ../../../released_plugins/v3d_plugins

INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$IMGMANAGERPATH
INCLUDEPATH += $$IMGMANAGERPATH/imgAnalyzer
INCLUDEPATH += $$IMGMANAGERPATH/imgProcessor
INCLUDEPATH += ../NeuronStructNavigator
INCLUDEPATH += ./
INCLUDEPATH += $$V3DTOOLPATH/swc2mask_cylinder
INCLUDEPATH += $$VAA3DPATH/neuron_editing
win32: {
    BOOSTPATH = $$(BOOST_PATH)
    INCLUDEPATH += $$BOOSTPATH
    LIBS += -L$$BOOSTPATH/lib64-msvc-12.0

    LIBS += -L$$IMGMANAGERPATH -lv3d_imgManagerMK
    LIBS += -L../NeuronStructNavigator -lNeuronStructNavigator
}

FORMS += brainAtlasTest.ui

HEADERS	+= BrainAtlas_plugin.h
HEADERS += BrainAtlasManager.h

SOURCES	+= BrainAtlas_plugin.cpp
SOURCES += BrainAtlasManager.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BrainAtlas)
DESTDIR	= ../../../../v3d_external/bin/plugins/BrainAtlas/
