TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
#QMAKE_CXXFLAGS += -std=c++11

VAA3DPATH = ../../../../v3d_external
TERASTITCHERPATH = ../../../released_plugins/v3d_plugins/terastitcher
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$TERASTITCHERPATH/src/core/imagemanager
INCLUDEPATH	+= $$TERASTITCHERPATH/src/core/iomanager
INCLUDEPATH	+= $$TERASTITCHERPATH/src/core/iomanager/plugins/tiff2D
INCLUDEPATH	+= $$TERASTITCHERPATH/src/core/iomanager/plugins/tiff3D
INCLUDEPATH	+= $$TERASTITCHERPATH/include
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/IM_config.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/imBlock.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/ProgressBar.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/RawFmtMngr.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/RawVolume.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/SimpleVolume.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/SimpleVolumeRaw.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/Stack.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/StackedVolume.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/StackRaw.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/Tiff3DMngr.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/TiledMCVolume.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/TiledVolume.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/TimeSeries.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/VirtualFmtMngr.h
HEADERS += $$TERASTITCHERPATH/src/core/imagemanager/VirtualVolume.h
HEADERS += $$TERASTITCHERPATH/src/core/iomanager/iomanager.config.h
HEADERS += $$TERASTITCHERPATH/src/core/iomanager/IOPluginAPI.h
HEADERS += $$TERASTITCHERPATH/src/core/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += $$TERASTITCHERPATH/src/core/iomanager/plugins/tiff3D/tiff3D.h
HEADERS	+= Dataset_Generator_plugin.h
HEADERS += datasetGeneratorUI.h
HEADERS += Operator.h
HEADERS += NeuronStructNavigator.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/IM_config.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/imBlock.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/imProgressBar.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/RawFmtMngr.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/RawVolume.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/SimpleVolume.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/SimpleVolumeRaw.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/Stack.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/StackedVolume.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/StackRaw.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/Tiff3DMngr.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/TiledMCVolume.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/TiledVolume.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/TimeSeries.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/VirtualFmtMngr.cpp
SOURCES += $$TERASTITCHERPATH/src/core/imagemanager/VirtualVolume.cpp
SOURCES += $$TERASTITCHERPATH/src/core/iomanager/iomanager.config.cpp
SOURCES += $$TERASTITCHERPATH/src/core/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += $$TERASTITCHERPATH/src/core/iomanager/plugins/tiff3D/tiff3D.cpp
SOURCES	+= Dataset_Generator_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += datasetGeneratorUI.cpp
SOURCES += Operator.cpp
SOURCES += NeuronStructNavigator.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

win32 {
    LIBS += -L$$VAA3DPATH\\v3d_main\\common_lib\\winlib64
    LIBS += -llibtiff
}

macx {
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
}

FORMS += DatasetGenerator.ui

TARGET	= $$qtLibraryTarget(Dataset_Generator)
DESTDIR	= $$VAA3DPATH/bin/plugins/Dataset_Generator/
