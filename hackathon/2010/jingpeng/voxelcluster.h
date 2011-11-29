#ifndef VOXELCLUSTER_H
#define VOXELCLUSTER_H

#include <QList>
#include <QtAlgorithms>
#include <qmath.h>
#include <QtGlobal>
#include <QVector>

#include "basic_landmark.h"
#include "../../../v3d/v3d_main/neuron_editing/v_neuronswc.h"

class Voxel
{
public:
    Voxel();
    ~Voxel();

    double distance(Voxel v);
    double distance( LocationSimple node );
    double distance(long cx, long cy, long cz);
    bool isNeighbor(Voxel v);

    // variables
    int x,y,z,c;
};


class VoxelCluster
{
public:
    VoxelCluster();
    ~VoxelCluster();

    bool appendVoxel(int x, int y, int z);
//    inline bool appendVoxel(int x, int y, int z) {Voxel loc; loc.x = x; loc.y = y; loc.z = z; loc.c = 0; voxels.append(loc);}
//    inline bool appendVoxel(Voxel v)  { voxels.append(v); return true;}
//    bool appendCluster(VoxelCluster clt);
    V_NeuronSWC_unit getCenter();
    QList < VoxelCluster > split();
    QList < VoxelCluster > split_V2();
    bool findSub(int x, int y, int z, int M, int N, int K, int comID, VoxelCluster& tmpClt);
    bool findSub_V2(int x, int y, int z, int M, int N, int K, int comID, VoxelCluster& tmpClt);
    bool getBoundingBox(bool force);
    bool clearVisited();
    bool clear();
    double getSize();
    double getScoopingDistance( V_NeuronSWC_unit node );

    //variables
    std::vector< Voxel >  voxels;
    int NumCom; //number of components
    QVector< bool > flag;
    QVector< int > lable;
    long parent_node_ID;
    int threshold;

private:
    bool connectedComponent();
    float size;
    int xb, xe, yb, ye, zb, ze;
    int MinVoxels;
};

#endif // VOXELCLUSTER_H
