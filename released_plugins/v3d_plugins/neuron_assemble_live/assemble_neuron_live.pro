
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../
QT_PATH = $$dirname(QMAKE_QMAKE)/..

INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH     += $$VAA3DPATH/v3d_main/3drenderer
INCLUDEPATH     += $$VAA3DPATH/v3d_main/basic_c_fun/customary_structs
INCLUDEPATH     += $$VAA3DPATH/v3d_main
INCLUDEPATH     += $$QT_PATH/demos/shared

macx{
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff
    } else {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibtiff
    }
}

unix {
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include/hdf5
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_unix64 -lhdf5 -lszip -lz -ldl
}

HEADERS	+= assemble_neuron_live_plugin.h \
        openSWCDialog.h \
    assemble_neuron_live_dialog.h
SOURCES	+= assemble_neuron_live_plugin.cpp \
        openSWCDialog.cpp \
    assemble_neuron_live_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../neurontracing_vn2/app2/my_surf_objs.cpp

INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/crossmips
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/common
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/control
INCLUDEPATH += $$VAA3DPATH/v3d_main/terafly/src/presentation

HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/iomanager.config.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/ioplugins.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/IOPluginAPI.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/common/ProgressBar.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/crossmips/CrossMIPs.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/crossmips/my_defs.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/crossmips/compute_funcs.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/S_config.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/Displacement.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/PDAlgo.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/StackRestorer.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/StackStitcher.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/TPAlgo.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/TPAlgoMST.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/resumer.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/dirent_win.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmBlock.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmStack.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/volumemanager.config.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/imBlock.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/dirent_win.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/IM_config.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/RawVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/Stack.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/StackRaw.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/StackedVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/TiledVolume.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/TimeSeries.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/VirtualVolume.h
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/iomanager.config.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/common/ProgressBar.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/crossmips/compute_funcs.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/crossmips/libcrossmips.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/Displacement.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/PDAlgo.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/StackRestorer.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/StackStitcher.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/TPAlgo.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/MergeTiles.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/stitcher/resumer.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmBlock.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmStack.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/imBlock.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/IM_config.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/RawVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/Stack.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/StackRaw.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/StackedVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/TiledVolume.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/TimeSeries.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp

#setup TeraStitcher I/O plugins
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h
#HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
#SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp
SOURCES += $$VAA3DPATH/v3d_main/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp


TARGET	= $$qtLibraryTarget(assemble_neuron_live)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/assemble_neuron_live/
