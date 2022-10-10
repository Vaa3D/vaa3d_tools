# 20170623: by RZC. Change name of terafly/teramanager.pro to terafly.pro
# 20201112: by RZC. Add 4 new cpp files
QMAKE_CXXFLAGS += -DWITH_NONAMESPACES
QMAKE_CXXFLAGS += -DWITH_PURE_VIRTUAL
QMAKE_CXXFLAGS += -DTEEM_STATIC

VAA3DPATH = C:\Users\82700\v3d_external\bin




INCLUDEPATH += C:\Users\82700\v3d_external\v3d_main\common_lib\include\glew\GL  #for VR, by PHC 20170615
LIBS += -L../common_lib/mingw64  -lSDL2 -lopenvr_api -lglew32

# Resources such as icons
RESOURCES += C:\Users\82700\v3d_external\v3d_main\neuron_annotator\resources.qrc
 #   res.qrc
RESOURCES += C:\Users\82700\v3d_external\v3d_main\v3d\v3d.qrc C:\Users\82700\v3d_external\v3d_main\3drenderer\3drenderer.qrc
DEFINES += __ALLOW_VR_FUNCS__

SOURCES += ../terafly/src/terarepo/src/imagemanager/MultiCycleVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/MultiSliceVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/ComposedVolume.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/TPAlgoLQP.cpp

SOURCES +=  C:/Users/82700/v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES +=  C:/Users/82700/v3d_external/v3d_main/3drenderer/v3dr_control_signal.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_core.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_glwidget.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\mapview.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\atlas_viewer.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\ChannelTable.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\mainwindow.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3dimg_proc_neuron.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_editing\neuron_format_converter.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer_hit.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\basic_4dimage.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\my4dimage.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3dimgproc_entry.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer_obj.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_editing\v_neuronswc.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_mainwindow.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\old_arthurwidgets.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\dialog_rotate.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\stackutil.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\multithreadimageIO\v3d_multithreadimageIO.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\landmark_property_dialog.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\cellseg\template_matching_seg.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_version_info.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_application.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer_gl2.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer_tex.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\mainwindow_interface.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\nstroke_tracing.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_colormapDialog.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_surfaceDialog.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\plugin_loader\v3d_plugin_loader.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_actions.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\DownloadManager.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\import_filelistname.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\import_tiffseries.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\custom_toolbar\v3d_custom_toolbar.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\pluginfunchandler.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\imaging\v3d_imaging.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_tracing\dij_bgl.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\worm_straighten_c\bdb_minus.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\barFigureDialog.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_editing\neuron_sim_scores.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\nstroke.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\surfaceobj_geometry_dialog.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_toolbox\vaa3d_neurontoolbox.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\io\v3d_nrrd.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\colormap.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\jba\c++\histeq.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\basic_4dimage_create.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\histogramsimple.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer_labelfield.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\io\asc_to_swc.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\mg_image_lib.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\glsl_r.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\gradients.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\plugin_loader\pluginDialog.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\io\io_bioformats.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\graph\dijk.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\mg_utilities.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3d_hoverpoints.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_editing\neuron_xforms.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_editing\apo_xforms.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\worm_straighten_c\mst_prim_c.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\neuron_editing\global_feature_compute.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\worm_straighten_c\bfs_1root.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\main.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_commandlineparser.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\CommandManager.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\VR_MainWindow.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\v3d\vr_vaa3d_call.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\v3dr_gl_vr.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\Matrices.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\Sphere.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\Cylinder.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\lodepng.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\RenderableObject.cpp
SOURCES +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\VRFinger.cpp












HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\VR_MainWindow.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\xformwidget.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_glwidget.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\renderer_gl2.h
#HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\old_arthurwidgets.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_core.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_mainwindow.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\template_matching_cellseg_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\dialog_rotate.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\ChannelTable.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\old_arthurwidgets.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_application.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\plugin_loader\v3d_plugin_loader.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\atlas_viewer.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\mainwindow.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\basic_thread.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_global_preference_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_version_info.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\basic_c_fun\v3d_curvetracepara.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\dialog_curve_trace_para.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\surfaceobj_geometry_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\surfaceobj_annotation_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\landmark_property_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\colormap.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\dialog_maskroi.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\dialog_imagecrop_bbox.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\dialog_imageresample.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_colormapDialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3dr_surfaceDialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\plugin_loader\pluginDialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\v3d_actions.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\DownloadManager.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\import_filelist_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\import_images_tool_dialog.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\gradients.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\3drenderer\v3d_hoverpoints.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\custom_toolbar\v3d_custom_toolbar.h
HEADERS +=  C:\Qt6\6.1.3\mingw81_64\include\QtCore\qglobal.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\v3d\vr_vaa3d_call.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\v3dr_gl_vr.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\Matrices.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\Sphere.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\Cylinder.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\lodepng.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\RenderableObject.h
HEADERS +=  C:\Users\82700\v3d_external\v3d_main\vrrenderer\VRFinger.h



QT += core5compat widgets opengl openglwidgets




#CONFIG   += c++11
# enable Vaa3D-TeraFly mode
DEFINES += _VAA3D_TERAFLY_PLUGIN_MODE
#QMAKE_CXXFLAGS += -std=c++11
#generic set up
CONFIG += use_experimental_features
#DEFINES += QT_NO_DEBUG_STREAM
#DEFINES += QT_NO_DEBUG_OUTPUT
#DEFINES += QT_NO_WARNING_OUTPUT
#DEFINES += _USE_QT_DIALOGS
#DEFINES += terafly_enable_debug_annotations

# set optimization for g++/clang compilers
#win32{
#}
#else{
#    # remove possible other optimization flags
#    QMAKE_CXXFLAGS -= -O
#    QMAKE_CXXFLAGS -= -O1
#    QMAKE_CXXFLAGS -= -O2

#    # add the desired -O3 if not present
#    QMAKE_CXXFLAGS += -O3
#}

#QMAKE_CXXFLAGS += -Wall
#QMAKE_CXXFLAGS += -pedantic
#QMAKE_CXXFLAGS += -Werror


#set up Vaa3D and Qt source path
V3DMAINPATH = C:\Users\82700\v3d_external\v3d_main
QT_PATH = $$dirname(QMAKE_QMAKE)/..

#HDF5 headers and precompiled library and dependencies (libz and libszip)
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5

win32 {
     message("WARNING: hdf5: no support for 32 bit windows")
   ## 64bit
     INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5-win64
     LIBS += -L$$V3DMAINPATH/common_lib/mingw64 -lhdf5 -lszip -lzlib
}

macx {
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5
LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lhdf5 -lszip -lz
}

unix:!macx {
    INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5

    LIBS += -L$$V3DMAINPATH/common_lib/lib_ubuntu -lhdf5 -lszip -lz -ldl
}
#Vaa3D headers and sources needed by the plugin
INCLUDEPATH+= $$QT_PATH/demos/shared
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/3drenderer
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun/customary_structs
INCLUDEPATH += $$V3DMAINPATH
INCLUDEPATH += $$V3DMAINPATH/v3d
INCLUDEPATH += $$V3DMAINPATH/vrrenderer

#enable experimental features
DEFINES += USE_EXPERIMENTAL_FEATURES
#DEFINES += terafly_enable_debug_max_level

#enable gui progress bar
DEFINES += WITH_QT

#enable HDF5
DEFINES += ENABLE_BDV_HDF5
DEFINES += ENABLE_IMS_HDF5

#just define dummy symbols needed by TeraStitcher code
DEFINES += TERASTITCHER_MAJOR
DEFINES += TERASTITCHER_MINOR
DEFINES += TERASTITCHER_PATCH
DEFINES += TERASTITCHER2_MAJOR
DEFINES += TERASTITCHER2_MINOR
DEFINES += TERASTITCHER2_PATCH
DEFINES += TERACONVERTER_MAJOR
DEFINES += TERACONVERTER_MINOR
DEFINES += TERACONVERTER_PATCH

#setup iomanager
INCLUDEPATH += ../terafly/src/terarepo/src/iomanager

HEADERS += ../terafly/src/terarepo/src/iomanager/iomanager.config.h
HEADERS += ../terafly/src/terarepo/src/iomanager/ioplugins.h
HEADERS += ../terafly/src/terarepo/src/iomanager/IOPluginAPI.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.h
#HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../terafly/src/terarepo/src/iomanager/iomanager.config.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.cpp
#SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp

#setup imagemanager
INCLUDEPATH += ../terafly/src/terarepo/src/imagemanager
HEADERS += ../terafly/src/terarepo/src/imagemanager/BDVVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/HDF5Mngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/imBlock.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/dirent_win.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/IM_config.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/RawVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/SimpleVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/Stack.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/StackRaw.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/StackedVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/TiledVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/TimeSeries.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/VirtualVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/UnstitchedVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.h
SOURCES += ../terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/BDVVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/HDF5Mngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/imBlock.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/IM_config.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/RawVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/Stack.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/StackRaw.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/StackedVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp
#SOURCES += ../terafly/src/terarepo/src/imagemanager/TiledVolume.cpp
SOURCES += ../terafly/src/TiledVolume_partialdata.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/TimeSeries.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/UnstitchedVolume.cpp

# set up stitcher
INCLUDEPATH += ../terafly/src/terarepo/src/stitcher
HEADERS += ../terafly/src/terarepo/src/stitcher/S_config.h
HEADERS += ../terafly/src/terarepo/src/stitcher/Displacement.h
HEADERS += ../terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
HEADERS += ../terafly/src/terarepo/src/stitcher/PDAlgo.h
HEADERS += ../terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
HEADERS += ../terafly/src/terarepo/src/stitcher/StackRestorer.h
HEADERS += ../terafly/src/terarepo/src/stitcher/StackStitcher.h
HEADERS += ../terafly/src/terarepo/src/stitcher/TPAlgo.h
HEADERS += ../terafly/src/terarepo/src/stitcher/TPAlgoMST.h
HEADERS += ../terafly/src/terarepo/src/stitcher/resumer.h
SOURCES += ../terafly/src/terarepo/src/stitcher/Displacement.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/MergeTiles.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/PDAlgo.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/StackRestorer.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/StackStitcher.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/TPAlgo.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/resumer.cpp

# set up volumeconverter
INCLUDEPATH += ../terafly/src/terarepo/src/utils/volumeconverter
#HEADERS += ../terafly/src/terarepo/src/utils/volumeconverter/S_config.h
HEADERS += ../terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.h
SOURCES += ../terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.cpp
HEADERS += ../terafly/src/terarepo/src/utils/volumeconverter/vcresumer.h
SOURCES += ../terafly/src/terarepo/src/utils/volumeconverter/vcresumer.cpp

# set up volumemanager
INCLUDEPATH += ../terafly/src/terarepo/src/volumemanager
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmBlock.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmStack.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmCacheManager.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/volumemanager.config.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmMCVolume.h
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmBlock.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmStack.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmCacheManager.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmMCVolume.cpp

# set up crossmips
INCLUDEPATH += ../terafly/src/terarepo/src/crossmips
HEADERS += ../terafly/src/terarepo/src/crossmips/my_defs.h
HEADERS += ../terafly/src/terarepo/src/crossmips/CrossMIPs.h
HEADERS += ../terafly/src/terarepo/src/crossmips/compute_funcs.h
SOURCES += ../terafly/src/terarepo/src/crossmips/compute_funcs.cpp
SOURCES += ../terafly/src/terarepo/src/crossmips/libcrossmips.cpp

# set up tinyxml
INCLUDEPATH += ../terafly/src/terarepo/src/3rdparty/tinyxml
HEADERS += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
HEADERS += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp

# set up common
INCLUDEPATH += ../terafly/src/terarepo/src/common
HEADERS += ../terafly/src/terarepo/src/common/config.h
HEADERS += ../terafly/src/terarepo/src/common/ProgressBar.h
HEADERS += ../terafly/src/terarepo/src/common/QProgressSender.h
SOURCES += ../terafly/src/terarepo/src/common/config.cpp
SOURCES += ../terafly/src/terarepo/src/common/ProgressBar.cpp
SOURCES += ../terafly/src/terarepo/src/common/QProgressSender.cpp


#set up TeraFly (control and presentation classes)
RESOURCES += ../terafly/icons.qrc
INCLUDEPATH += ../terafly/src/control
HEADERS += ../terafly/src/control/V3Dsubclasses.h
HEADERS += ../terafly/src/control/CAnnotations.h
HEADERS += ../terafly/src/control/CConverter.h
HEADERS += ../terafly/src/control/CViewer.h
HEADERS += ../terafly/src/control/CImport.h
HEADERS += ../terafly/src/control/CPlugin.h
HEADERS += ../terafly/src/control/CSettings.h
HEADERS += ../terafly/src/control/CVolume.h
HEADERS += ../terafly/src/control/CImageUtils.h
HEADERS += ../terafly/src/control/V3Dsubclasses.h
HEADERS += ../terafly/src/control/VirtualPyramid.h
HEADERS += ../terafly/src/control/COperation.h
INCLUDEPATH += ../terafly/src/presentation
HEADERS += ../terafly/src/presentation/PConverter.h
HEADERS += ../terafly/src/presentation/PDialogImport.h
HEADERS += ../terafly/src/presentation/PDialogProofreading.h
HEADERS += ../terafly/src/presentation/PDialogVirtualPyramid.h
HEADERS += ../terafly/src/presentation/PMain.h
HEADERS += ../terafly/src/presentation/PTabVolumeInfo.h
HEADERS += ../terafly/src/presentation/QArrowButton.h
HEADERS += ../terafly/src/presentation/QGradientBar.h
HEADERS += ../terafly/src/presentation/QHelpBox.h
HEADERS += ../terafly/src/presentation/QGLRefSys.h
HEADERS += ../terafly/src/presentation/QPixmapToolTip.h
HEADERS += ../terafly/src/presentation/QPrefixSuffixLineEdit.h
HEADERS += ../terafly/src/presentation/PAbout.h
HEADERS += ../terafly/src/presentation/PLog.h
HEADERS += ../terafly/src/presentation/PAnoToolBar.h
HEADERS += ../terafly/src/control/QUndoMarkerCreate.h
HEADERS += ../terafly/src/control/QUndoMarkerDelete.h
HEADERS += ../terafly/src/control/QUndoMarkerDeleteROI.h

win32{
HEADERS += ../terafly/src/presentation/fileserver.h
SOURCES += ../terafly/src/presentation/fileserver.cpp
}

SOURCES += ../terafly/src/control/CAnnotations.cpp
SOURCES += ../terafly/src/control/CConverter.cpp
SOURCES += ../terafly/src/control/CViewer.cpp
SOURCES += ../terafly/src/control/CImport.cpp
SOURCES += ../terafly/src/control/CPlugin.cpp
SOURCES += ../terafly/src/control/CSettings.cpp
SOURCES += ../terafly/src/control/CVolume.cpp
SOURCES += ../terafly/src/control/CImageUtils.cpp
SOURCES += ../terafly/src/control/COperation.cpp
SOURCES += ../terafly/src/control/V3Dsubclasses.cpp
SOURCES += ../terafly/src/control/VirtualPyramid.cpp
SOURCES += ../terafly/src/presentation/PConverter.cpp
SOURCES += ../terafly/src/presentation/PDialogImport.cpp
SOURCES += ../terafly/src/presentation/PDialogProofreading.cpp
SOURCES += ../terafly/src/presentation/PDialogVirtualPyramid.cpp
SOURCES += ../terafly/src/presentation/PMain.cpp
SOURCES += ../terafly/src/presentation/PTabVolumeInfo.cpp
SOURCES += ../terafly/src/presentation/PAbout.cpp
SOURCES += ../terafly/src/presentation/PLog.cpp
SOURCES += ../terafly/src/presentation/PAnoToolBar.cpp
SOURCES += ../terafly/src/presentation/QPixmapToolTip.cpp
SOURCES += ../terafly/src/presentation/QArrowButton.cpp
SOURCES += ../terafly/src/presentation/QGradientBar.cpp
SOURCES += ../terafly/src/presentation/QHelpBox.cpp
SOURCES += ../terafly/src/presentation/QGLRefSys.cpp
SOURCES += ../terafly/src/presentation/QPrefixSuffixLineEdit.cpp
SOURCES += ../terafly/src/control/QUndoMarkerCreate.cpp
SOURCES += ../terafly/src/control/QUndoMarkerDelete.cpp
SOURCES += ../terafly/src/control/QUndoMarkerDeleteROI.cpp
