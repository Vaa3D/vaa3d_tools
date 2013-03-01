#ifndef MOSTIMAGE_H
#define MOSTIMAGE_H

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "../../../vaa3d/v3d_main/v3d/v3d_core.h"
#include "v3d_interface.h"
#include "../../../vaa3d/v3d_main/neuron_editing/v_neuronswc.h"
#include "../../../vaa3d/v3d_main/neuron_editing/neuron_format_converter.h"
#include <vector>
#include "voxelcluster.h"
#include "mostVesselTracer.h"
#include "tools.h"

// some constant threshold
#define MinRadius 0
#define MaxRadius 500
//#define InitThreshold 30

// include rayburst sampling
extern "C" {
#include "srb.h"
}



class MOSTImage : public Image4DSimple
{
public:
    MOSTImage();
    ~MOSTImage();

    bool isVisted(const LocationSimple &loc);
    V_NeuronSWC trace_single_seed( const LocationSimple &seed ,const int InitTheshold , int slipsize);
    NeuronTree trace_seed_list(const LandmarkList &seedList, std::vector<bool> &visited_cur,const int threshould,const double res_x_all,const double res_y_all,const double res_z_all,QString swcfile,int slipsize,int pruning_flag);
    void auto_detect_seedz(LandmarkList &seedList,V3DLONG slice,const int InitThreshold,const int seed_size_all);
    void auto_detect_seedx(LandmarkList &seedList,V3DLONG slice,const int InitThreshold,const int seed_size_all);
    void auto_detect_seedy(LandmarkList &seedList,V3DLONG slice,const int InitThreshold,const int seed_size_all);


private:
    int getAdaptiveThreshold( const V_NeuronSWC_unit &pivot, const float &radius );
    V_NeuronSWC_unit createNode(VoxelCluster &conCom, const V_NeuronSWC_unit &preNode, const double &preSize);
    V_NeuronSWC_unit createNode_V2(VoxelCluster &conCom);
        V_NeuronSWC_unit createNode_br(VoxelCluster &conCom);
    float getRayburstRadius( V_NeuronSWC_unit &center, const bool &force, const int &rayburstThreshold );
    V_NeuronSWC_unit recenter( V_NeuronSWC_unit &center , const int &threshold);
    bool regionGrowInSphere_V2 (const V_NeuronSWC_unit &pivot, const float &radius, V_NeuronSWC_unit &seed, VoxelCluster &clt, const int &threshold);
    VoxelCluster findUnvisitedNeighbor(VoxelCluster &clt );
    VoxelCluster voxelScooping (VoxelCluster &clt, const V_NeuronSWC_unit &node, const float &scoopingDistance, const int &threshold);
    VoxelCluster voxelScooping_V2 ( const V_NeuronSWC_unit &node, const float &scoopingDistance, const int &threshold );

    //varibles
    std::vector<bool>  visited;
    LocationSimple seed;
    V_NeuronSWC swc;
    V_NeuronSWC_unit swcUnitTmp;
    NeuronTree vt;
};

#endif // MOSTIMAGE_H
