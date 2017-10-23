
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH  += $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include
INCLUDEPATH  += $$VAA3DPATH/common_lib/include/hdf5
INCLUDEPATH  += $$VAA3DPATH/terafly
INCLUDEPATH     += ../neurontracing_vn2
INCLUDEPATH     += ../neurontracing_vn2/app2
INCLUDEPATH     += ../neurontracing_vn2/app1
INCLUDEPATH     += ../neuron_image_profiling
INCLUDEPATH	+= ../eswc_converter
INCLUDEPATH	+= $$VAA3DPATH/jba/newmat11

INCLUDEPATH     += $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c
#INCLUDEPATH     += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/libtiff
#INCLUDEPATH     += $$VAA3DPATH/common_lib/build/libtiff


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
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/iomanager
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/crossmips
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/stitcher
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/common
INCLUDEPATH += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5

HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/iomanager.config.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/ioplugins.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/IOPluginAPI.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/common/ProgressBar.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/crossmips/CrossMIPs.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/crossmips/my_defs.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/crossmips/compute_funcs.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/S_config.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/Displacement.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/PDAlgo.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/StackRestorer.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/StackStitcher.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/TPAlgo.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/TPAlgoMST.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/resumer.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/dirent_win.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmBlock.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmStack.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/volumemanager.config.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/imBlock.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/dirent_win.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/IM_config.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/common/ProgressBar.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/RawVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/SimpleVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/Stack.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/StackRaw.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/StackedVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/TiledVolume.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/TimeSeries.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/VirtualVolume.h
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/iomanager.config.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/common/ProgressBar.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/crossmips/compute_funcs.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/crossmips/libcrossmips.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/Displacement.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/PDAlgo.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/StackRestorer.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/StackStitcher.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/TPAlgo.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/MergeTiles.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/stitcher/resumer.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmBlock.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmStack.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/imBlock.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/IM_config.cpp
#SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/common/imProgressBar.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/RawVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/Stack.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/StackRaw.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/StackedVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/TiledVolume.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/TimeSeries.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp

SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/HDF5Mngr.cpp
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/HDF5Mngr.h

SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h

SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/UnstitchedVolume.cpp
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/UnstitchedVolume.h

SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/BDVVolume.cpp
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/imagemanager/BDVVolume.h
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmCacheManager.cpp
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/volumemanager/vmCacheManager.h

SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.cpp
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.h

#setup TeraStitcher I/O plugins
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
#HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
#SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += $$VAA3DPATH/terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp


TARGET	= $$qtLibraryTarget(ultratracer)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/ultratracer/
