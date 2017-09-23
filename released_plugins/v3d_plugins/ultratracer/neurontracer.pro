
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH  += $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include
INCLUDEPATH  += $$VAA3DPATH/terafly
INCLUDEPATH     += ../neurontracing_vn2
INCLUDEPATH     += ../neurontracing_vn2/app2
INCLUDEPATH     += ../neurontracing_vn2/app1
INCLUDEPATH     += ../neuron_image_profiling
INCLUDEPATH	+= ../eswc_converter
INCLUDEPATH	+= $$VAA3DPATH/jba/newmat11

INCLUDEPATH     += $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c

macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

}



HEADERS	+= neurontracer_plugin.h
HEADERS	+= tracing_func.h

HEADERS	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.h

HEADERS += ../neurontracing_vn2/vn_imgpreprocess.h
HEADERS += ../neurontracing_vn2/vn.h
HEADERS += ../neurontracing_vn2/vn_app2.h
HEADERS += ../neurontracing_vn2/vn_app1.h

HEADERS += ../neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += ../neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += ../neurontracing_vn2/app2/fastmarching_dt.h
HEADERS += ../neuron_image_profiling/openSWCDialog.h

SOURCES	+= neurontracer_plugin.cpp
SOURCES	+= tracing_func.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp

SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp

SOURCES += ../neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += ../neurontracing_vn2/app2_connector.cpp
SOURCES += ../neurontracing_vn2/app1_connector.cpp
SOURCES += ../neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES += ../neurontracing_vn2/app1/gd.cpp
SOURCES += ../neurontracing_vn2/app1/calculate_cover_scores.cpp

SOURCES += ../neurontracing_vn2/swc_convert.cpp
SOURCES += ../neuron_image_profiling/profile_swc.cpp
SOURCES += ../neuron_image_profiling/compute_tubularity.cpp
SOURCES += ../neuron_image_profiling/openSWCDialog.cpp


SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$$$VAA3DPATH/neuron_editing/v_neuronswc.cpp \
    $$VAA3DPATH/graph/dijk.cpp

#meanshift
HEADERS += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp


INCLUDEPATH += ../terastitcher/include
INCLUDEPATH += ../terastitcher/src/core/iomanager
INCLUDEPATH += ../terastitcher/src/core/tinyxml
INCLUDEPATH += ../terastitcher/src/core/crossmips
INCLUDEPATH += ../terastitcher/src/core/volumemanager
INCLUDEPATH += ../terastitcher/src/core/stitcher
INCLUDEPATH += ../terastitcher/src/core/imagemanager
HEADERS += ../terastitcher/src/core/tinyxml/tinyxml.h
HEADERS += ../terastitcher/src/core/tinyxml/tinystr.h
HEADERS += ../terastitcher/src/core/iomanager/iomanager.config.h
HEADERS += ../terastitcher/src/core/iomanager/ioplugins.h
HEADERS += ../terastitcher/src/core/iomanager/IOPluginAPI.h
HEADERS += ../terastitcher/src/core/iomanager/ProgressBar.h
HEADERS += ../terastitcher/src/core/crossmips/CrossMIPs.h
HEADERS += ../terastitcher/src/core/crossmips/my_defs.h
HEADERS += ../terastitcher/src/core/crossmips/compute_funcs.h
HEADERS += ../terastitcher/src/core/stitcher/S_config.h
HEADERS += ../terastitcher/src/core/stitcher/Displacement.h
HEADERS += ../terastitcher/src/core/stitcher/DisplacementMIPNCC.h
HEADERS += ../terastitcher/src/core/stitcher/PDAlgo.h
HEADERS += ../terastitcher/src/core/stitcher/PDAlgoMIPNCC.h
HEADERS += ../terastitcher/src/core/stitcher/StackRestorer.h
HEADERS += ../terastitcher/src/core/stitcher/StackStitcher.h
HEADERS += ../terastitcher/src/core/stitcher/TPAlgo.h
HEADERS += ../terastitcher/src/core/stitcher/TPAlgoMST.h
HEADERS += ../terastitcher/src/core/stitcher/resumer.h
HEADERS += ../terastitcher/src/core/volumemanager/dirent_win.h
HEADERS += ../terastitcher/src/core/volumemanager/vmBlock.h
HEADERS += ../terastitcher/src/core/volumemanager/vmBlockVolume.h
HEADERS += ../terastitcher/src/core/volumemanager/vmStack.h
HEADERS += ../terastitcher/src/core/volumemanager/vmStackedVolume.h
HEADERS += ../terastitcher/src/core/volumemanager/vmVirtualStack.h
HEADERS += ../terastitcher/src/core/volumemanager/vmVirtualVolume.h
HEADERS += ../terastitcher/src/core/volumemanager/volumemanager.config.h
HEADERS += ../terastitcher/src/core/imagemanager/imBlock.h
HEADERS += ../terastitcher/src/core/imagemanager/dirent_win.h
HEADERS += ../terastitcher/src/core/imagemanager/IM_config.h
HEADERS += ../terastitcher/src/core/imagemanager/ProgressBar.h
HEADERS += ../terastitcher/src/core/imagemanager/RawFmtMngr.h
HEADERS += ../terastitcher/src/core/imagemanager/RawVolume.h
HEADERS += ../terastitcher/src/core/imagemanager/SimpleVolume.h
HEADERS += ../terastitcher/src/core/imagemanager/SimpleVolumeRaw.h
HEADERS += ../terastitcher/src/core/imagemanager/Stack.h
HEADERS += ../terastitcher/src/core/imagemanager/StackRaw.h
HEADERS += ../terastitcher/src/core/imagemanager/StackedVolume.h
HEADERS += ../terastitcher/src/core/imagemanager/Tiff3DMngr.h
HEADERS += ../terastitcher/src/core/imagemanager/TiledMCVolume.h
HEADERS += ../terastitcher/src/core/imagemanager/TiledVolume.h
HEADERS += ../terastitcher/src/core/imagemanager/TimeSeries.h
HEADERS += ../terastitcher/src/core/imagemanager/VirtualFmtMngr.h
HEADERS += ../terastitcher/src/core/imagemanager/VirtualVolume.h
SOURCES += ../terastitcher/src/core/tinyxml/tinyxml.cpp
SOURCES += ../terastitcher/src/core/tinyxml/tinystr.cpp
SOURCES += ../terastitcher/src/core/tinyxml/tinyxmlerror.cpp
SOURCES += ../terastitcher/src/core/tinyxml/tinyxmlparser.cpp
SOURCES += ../terastitcher/src/core/iomanager/iomanager.config.cpp
SOURCES += ../terastitcher/src/core/iomanager/ProgressBar.cpp
SOURCES += ../terastitcher/src/core/crossmips/compute_funcs.cpp
SOURCES += ../terastitcher/src/core/crossmips/libcrossmips.cpp
SOURCES += ../terastitcher/src/core/stitcher/Displacement.cpp
SOURCES += ../terastitcher/src/core/stitcher/DisplacementMIPNCC.cpp
SOURCES += ../terastitcher/src/core/stitcher/PDAlgo.cpp
SOURCES += ../terastitcher/src/core/stitcher/PDAlgoMIPNCC.cpp
SOURCES += ../terastitcher/src/core/stitcher/StackRestorer.cpp
SOURCES += ../terastitcher/src/core/stitcher/StackStitcher.cpp
SOURCES += ../terastitcher/src/core/stitcher/TPAlgo.cpp
SOURCES += ../terastitcher/src/core/stitcher/TPAlgoMST.cpp
SOURCES += ../terastitcher/src/core/stitcher/MergeTiles.cpp
SOURCES += ../terastitcher/src/core/stitcher/resumer.cpp
SOURCES += ../terastitcher/src/core/volumemanager/vmBlock.cpp
SOURCES += ../terastitcher/src/core/volumemanager/vmBlockVolume.cpp
SOURCES += ../terastitcher/src/core/volumemanager/vmStack.cpp
SOURCES += ../terastitcher/src/core/volumemanager/vmStackedVolume.cpp
SOURCES += ../terastitcher/src/core/volumemanager/vmVirtualStack.cpp
SOURCES += ../terastitcher/src/core/volumemanager/vmVirtualVolume.cpp
SOURCES += ../terastitcher/src/core/volumemanager/volumemanager.config.cpp
SOURCES += ../terastitcher/src/core/imagemanager/imBlock.cpp
SOURCES += ../terastitcher/src/core/imagemanager/IM_config.cpp
SOURCES += ../terastitcher/src/core/imagemanager/imProgressBar.cpp
SOURCES += ../terastitcher/src/core/imagemanager/RawFmtMngr.cpp
SOURCES += ../terastitcher/src/core/imagemanager/RawVolume.cpp
SOURCES += ../terastitcher/src/core/imagemanager/SimpleVolume.cpp
SOURCES += ../terastitcher/src/core/imagemanager/SimpleVolumeRaw.cpp
SOURCES += ../terastitcher/src/core/imagemanager/Stack.cpp
SOURCES += ../terastitcher/src/core/imagemanager/StackRaw.cpp
SOURCES += ../terastitcher/src/core/imagemanager/StackedVolume.cpp
SOURCES += ../terastitcher/src/core/imagemanager/Tiff3DMngr.cpp
SOURCES += ../terastitcher/src/core/imagemanager/TiledMCVolume.cpp
SOURCES += ../terastitcher/src/core/imagemanager/TiledVolume.cpp
SOURCES += ../terastitcher/src/core/imagemanager/TimeSeries.cpp
SOURCES += ../terastitcher/src/core/imagemanager/VirtualFmtMngr.cpp
SOURCES += ../terastitcher/src/core/imagemanager/VirtualVolume.cpp

#setup TeraStitcher I/O plugins
HEADERS += ../terastitcher/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
#HEADERS += ../terastitcher/src/core/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../terastitcher/src/core/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../terastitcher/src/core/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../terastitcher/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
#SOURCES += ../terastitcher/src/core/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../terastitcher/src/core/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../terastitcher/src/core/iomanager/plugins/tiff3D/tiff3D.cpp


TARGET	= $$qtLibraryTarget(ultratracer)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/ultratracer/
