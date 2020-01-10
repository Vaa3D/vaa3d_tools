
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_CXXFLAGS += /MP
#CONFIG	+= x86_64

VAA3DPATH = ../../../../v3d_external/v3d_main
IMGMANAGERPATH = ../v3d_imgManagerMK
V3DTOOLPATH = ../../../released_plugins/v3d_plugins
BOOSTPATH = $$(BOOST_PATH)

INCLUDEPATH += $$BOOSTPATH
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$IMGMANAGERPATH
INCLUDEPATH += $$IMGMANAGERPATH/imgAnalyzer
INCLUDEPATH += $$IMGMANAGERPATH/imgProcessor
INCLUDEPATH += ../NeuronStructNavigator
INCLUDEPATH += ./
INCLUDEPATH += $$V3DTOOLPATH/swc2mask_cylinder
INCLUDEPATH += $$VAA3DPATH/neuron_editing

LIBS += -L$$BOOSTPATH/lib64-msvc-12.0
LIBS += -L$$IMGMANAGERPATH -lv3d_imgManagerMK
LIBS += -L../NeuronStructNavigator -lNeuronStructNavigator

FORMS += brainRegionListTest.ui
FORMS += brainAtlasTest.ui

HEADERS	+= BrainAtlas_plugin.h
HEADERS += BrainAtlasManager.h

SOURCES	+= BrainAtlas_plugin.cpp
SOURCES += BrainAtlasManager.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BrainAtlas)
DESTDIR	= ../../../../v3d_external/bin/plugins/BrainAtlas/
