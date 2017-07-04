
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3D_DIR = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3D_DIR/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/common_lib/include
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/include
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager

QT           += network

HEADERS  = s2_plugin.h \
    stackAnalyzer.h \
    noteTaker.h \
    targetList.h \
    eventLogger.h \
    tileInfo.h \ 
    s2monitor.h
HEADERS += s2UI.h
HEADERS += s2Controller.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/customary_structs/vaa3d_neurontoolbox_para.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.h


HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h
HEADERS +=../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/VirtualVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/IOPluginAPI.h

HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
#HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
#SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff3D/tiff3D.cpp



SOURCES = s2_plugin.cpp \
    stackAnalyzer.cpp \
    noteTaker.cpp \
    targetList.cpp \
    eventLogger.cpp \
    tileInfo.cpp \ 
    s2monitor.cpp
SOURCES += s2UI.cpp
SOURCES += s2Controller.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2_connector.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(s2)
DESTDIR	= $$VAA3D_DIR/bin/plugins/s2/

FORMS += \
    s2plot.ui

OTHER_FILES += \
    taskList.txt
#DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git rev-parse HEAD)\\\""

INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/include
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml/tinyxml.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml/tinystr.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/iomanager.config.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/ioplugins.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/IOPluginAPI.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/ProgressBar.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips/CrossMIPs.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips/my_defs.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips/compute_funcs.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/S_config.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/Displacement.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/DisplacementMIPNCC.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/PDAlgo.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/PDAlgoMIPNCC.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/StackRestorer.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/StackStitcher.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/TPAlgo.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/TPAlgoMST.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/resumer.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/dirent_win.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmBlock.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmBlockVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmStack.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmStackedVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmVirtualStack.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmVirtualVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/volumemanager.config.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/imBlock.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/dirent_win.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/IM_config.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/ProgressBar.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/RawFmtMngr.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/RawVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/SimpleVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/SimpleVolumeRaw.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/Stack.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/StackRaw.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/StackedVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/Tiff3DMngr.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/TiledMCVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/TiledVolume.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/TimeSeries.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/VirtualFmtMngr.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/VirtualVolume.h
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml/tinyxml.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml/tinystr.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml/tinyxmlerror.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/tinyxml/tinyxmlparser.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/iomanager.config.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/ProgressBar.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips/compute_funcs.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/crossmips/libcrossmips.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/Displacement.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/DisplacementMIPNCC.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/PDAlgo.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/PDAlgoMIPNCC.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/StackRestorer.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/StackStitcher.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/TPAlgo.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/TPAlgoMST.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/MergeTiles.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/stitcher/resumer.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmBlock.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmBlockVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmStack.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmStackedVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmVirtualStack.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/vmVirtualVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/volumemanager/volumemanager.config.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/imBlock.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/IM_config.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/imProgressBar.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/RawFmtMngr.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/RawVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/SimpleVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/SimpleVolumeRaw.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/Stack.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/StackRaw.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/StackedVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/Tiff3DMngr.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/TiledMCVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/TiledVolume.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/TimeSeries.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/VirtualFmtMngr.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/imagemanager/VirtualVolume.cpp

#setup TeraStitcher I/O plugins
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
#HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
#SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../../../released_plugins/v3d_plugins/terastitcher/src/core/iomanager/plugins/tiff3D/tiff3D.cpp


unix {
DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git rev-parse HEAD)\\\""
}

win32 {
    INCLUDEPATH     += $$VAA3D_DIR/v3d_main/common_lib/winlib64
}
