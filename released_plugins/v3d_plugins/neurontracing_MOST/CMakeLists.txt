include_directories( "${V3DMAINDIR}/v3d" )
include_directories( "${V3DMAINDIR}/neuron_editing" )

include_directories( "${V3D_BUILD_DIR}/v3d_main/v3d" ) # for the ui_*.h file 

find_path(QT4_DEMOS_INCLUDE_DIR
   arthurwidgets.h
   PATHS
         ${QT_LIBRARY_DIR}/..
         /Developer/Examples/Qt
   /usr/lib64/qt4
   /usr/lib/qt4
   /usr/local/Trolltech/Qt-${QTVERSION}
   "C:/Qt/${QTVERSION}"
   PATH_SUFFIXES
   demos/shared
   Demos/shared
    )
include_directories(${QT4_DEMOS_INCLUDE_DIR})

set(PLUGIN_DIRECTORY_NAME mostVesselTracer/ )
set(PLUGIN_SOURCES mostVesselTracer.cpp mostimage.cpp voxelcluster.cpp srb.c tools.cpp
	"${V3DMAINDIR}/neuron_editing/v_neuronswc.cpp"   "${V3DMAINDIR}/neuron_editing/neuron_format_converter.cpp")
set(PLUGIN_HEADERS mostVesselTracer.h mostimage.h voxelcluster.h srb.h tools.h)

configure_v3d_plugin(mostVesselTracer)
