include_directories( ../../../v3d/v3d_main/common_lib/include )
include_directories(../../../v3d/v3d_main/v3d )
include_directories( ../../../v3d/v3d_main/neuron_editing )

set(PLUGIN_DIRECTORY_NAME mostVesselTracer/ )
set(PLUGIN_SOURCES mostVesselTracer.cpp mostimage.cpp voxelcluster.cpp srb.c)
set(PLUGIN_HEADERS mostVesselTracer.h mostimage.h voxelcluster.h srb.h)

configure_v3d_plugin(mostVesselTracer)
