#include "voxelcluster.h"

VoxelCluster::VoxelCluster()
{
    this->size = 0;
    this->NumCom = 0;
    this->voxels.clear();
    this->flag.clear();
    this->lable.clear();

    // init the bounding box range
    xb = 65534, yb = 65534, zb = 65534 ;
    xe = 0, ye = 0, ze = 0 ;
    this->MinVoxels = 3 ;
    this->parent_node_ID = -1;
    this->threshold = 10;
}
VoxelCluster::~VoxelCluster()
{
    this->NumCom = 0;
    this->voxels.clear();
    this->flag.clear();
    this->lable.clear();
}
// append a voxel to this cluster
bool VoxelCluster::appendVoxel(int x, int y, int z)
{
    Voxel loc;
    loc.x = x;
    loc.y = y;
    loc.z = z;

    loc.c = 0;
    voxels.push_back(loc);
    return true;
}

//append a cluster to this cluster
//bool VoxelCluster::appendCluster(VoxelCluster clt)
//{
//    QList < Voxel >::ConstIterator i = clt.begin();
//    while (i!= clt.end())
//    {
//        cluster.append( clt[i] );
//        ++i;
//    }
//    return true;
//}
// get the coordinate center
V_NeuronSWC_unit VoxelCluster::getCenter()
{
    V_NeuronSWC_unit center;

    double cx=0,cy=0,cz=0;
    for ( long i = 0; i < voxels.size(); i++)
    {
        cx += voxels[i].x;
        cy += voxels[i].y;
        cz += voxels[i].z;
    }
    //average
    center.x = cx / voxels.size();
    center.y = cy / voxels.size();
    center.z = cz / voxels.size();
    return center;
}

// create the label buffer and make ordinary connectivity analysis
QList < VoxelCluster > VoxelCluster::split()
{
    VoxelCluster tmpClt;
    QList < VoxelCluster > cltList;

    // get bounding box
    this->getBoundingBox(true);
    int M = xe - xb + 1;
    int N = ye - yb + 1;
    int K = ze - zb + 1;

    // create matrix buffer and init
    flag.clear();
    lable.clear();
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            for (int k = 0; k < K; k++)
            {
                this->flag.append(true);
                lable.append(0);
            }
        }
    }

    //mark the voxel as unvisited
    for (long i = 0; i < this->voxels.size(); i++)
    {
        int xl = this->voxels[i].x - xb;
        int yl = this->voxels[i].y - yb;
        int zl = this->voxels[i].z - zb;
        long offset = zl * M * N + yl * M + xl;
        flag[ offset ] = false;
    }

    //connectivity analysis of label matrix
    int comID = 0;
    bool moreCom = true;
    while(moreCom)
    {
        moreCom = false;
        // whethe there exist unvisited voxel
        for (long i = 0; i < M; i++)
        {
            for (long j = 0; j < N; j++)
            {
                for (long k = 0; k < K; k++)
                {
                    long offset = k * M * N + j * M + i;
                    if (false == flag[offset])
                    {
                        comID += 1;
                        // clear temp cluster
                        tmpClt.clear();
                        tmpClt.appendVoxel( i+xb, j+yb, k+zb );
                        findSub_V2 (i,j,k,M,N,K,comID,tmpClt);
                        // add the cluster
                        if (tmpClt.voxels.size() > MinVoxels)
                        {
                            tmpClt.parent_node_ID = this->parent_node_ID;
                            cltList.append(tmpClt);
                        }
                        // find more new component
                        moreCom = true;
                    }
                }// for k
            }// for j
        }// for i
    }
    return cltList;
}

bool VoxelCluster::findSub_V2( int x, int y, int z, int M, int N, int K, int comID, VoxelCluster& tmpClt )
{
    // initialize voxel list
    QList < Voxel > voxelList;
    Voxel tmpVox;
    tmpVox.x = x;
    tmpVox.y = y;
    tmpVox.z = z;
    voxelList.append(tmpVox);

    while ( ! voxelList.isEmpty() )
    {
        tmpVox = voxelList.last();
        voxelList.removeLast();

        long offset =  M * N * tmpVox.z + M * tmpVox.y + tmpVox.x;
        if ( true == this->flag[ offset ])
            continue;   // visited

        // unvisited
        this->flag[ offset ] = true;
        this->lable[ offset ] = comID;
        tmpClt.appendVoxel( tmpVox.x + xb, tmpVox.y + yb, tmpVox.z + zb );

        for (register int i = tmpVox.x - 1; i <= tmpVox.x + 1; i++)
        {
            if ( i < 0 || i >= M )
                continue;
            for (register int j = tmpVox.y -1; j <= tmpVox.y + 1; j++)
            {
                if ( j < 0 || j >= N )
                    continue;
                for (register int k = tmpVox.z -1; k <= tmpVox.z + 1; k++)
                {
                    if (k < 0 || k >= K )
                        continue;
                    // append this voxel
                    Voxel v;
                    v.x = i;
                    v.y = j;
                    v.z = k;
                    voxelList.append(v);
                }
            }
        }
    }
    return true;
}

bool VoxelCluster::findSub( int x, int y, int z, int M, int N, int K, int comID, VoxelCluster& tmpClt )
{
    for (register int i = x - 1; i <= x + 1; i++)
    {
        if (i < 0 || i >= M)
            continue;
        for (register int j = y -1; j <= y + 1; j++)
        {
            if (j < 0 || j >= N)
                continue;
            for (register int k = z -1; k <= z + 1; k++)
            {
                if (k < 0 || k >= K)
                    continue;
                long offset =  M * N * k + M * j + i;
                if ( false == this->flag[ offset ])
                {   // unvisited
                    this->flag[ offset ] = true;
                    this->lable[ offset ] = comID;
                    tmpClt.appendVoxel( i+xb, j+yb, k+zb );
                    findSub(i,j,k,M,N,K,comID, tmpClt);
                }
            }
        }
    }
    return true;
}

bool VoxelCluster::clear()
{
    this->voxels.clear();
    this->NumCom = 0;
    this->size = 0;
    this->parent_node_ID = -1;
    this->flag.clear();
    this->lable.clear();
    this->threshold = 10;
    return true;
}

double VoxelCluster::getSize()
{
    if (size > 0)
        return size;

    this->getBoundingBox(false);

    // measure size
    size = sqrt( float((xe - xb)*(xe - xb) + (ye - yb)*(ye - yb) + (ze - zb)*(ze - zb) ) );
    return size;
}

bool VoxelCluster::getBoundingBox ( bool force )
{
    if ( (!force) && (xb != 65534) && (yb != 65534) && (zb != 65534) )
    {   // already counted and no force
        return false;
    }

    xb = 65534; yb = 65534; zb = 65534;
    xe = 0;     ye = 0;     ze = 0;

    // get min max coordinate
    for (long i = 0; i < this->voxels.size(); i++)
    {
        if (this->voxels[i].x < xb) xb = this->voxels[i].x;
        if (this->voxels[i].y < yb) yb = this->voxels[i].y;
        if (this->voxels[i].z < zb) zb = this->voxels[i].z;

        if (this->voxels[i].x > xe) xe = this->voxels[i].x;
        if (this->voxels[i].y > ye) ye = this->voxels[i].y;
        if (this->voxels[i].z > ze) ze = this->voxels[i].z;
    }
    return true;
}

double VoxelCluster::getScoopingDistance(V_NeuronSWC_unit node)
{
    if (this->voxels.empty() )
        return 0;

    double maxD = 0;

    for (int i = 0; i < this->voxels.size(); i++ )
    {
        double d = this->voxels[i].distance(node.x,node.y,node.z);
        if ( maxD < d )
            maxD = d;
    }
    return maxD;

    // the following code won't work, any bug here ?
    //    double maxD_sq = 0, d_sq = 0;

    //    for (int i = 0; i < this->voxels.count(); i++ )
    //    {
    //        double vx = this->voxels[i].x;
    //        double vy = this->voxels[i].y;
    //        double vz = this->voxels[i].z;

    //        d_sq = (node.x - vx)*(node.x - vx) + (node.y - vy)*(node.y - vy) + (node.z - vz)*(node.z - vz);
    //        if ( maxD_sq < d_sq )
    //            maxD_sq = d_sq;
    //    }
    //    return sqrt( maxD_sq );
}

// Voxel class
Voxel::Voxel()
{
    x = 0;
    y = 0;
    z = 0;
    c = 0;  // component ID
}
Voxel::~Voxel()
{
    this->c = 0;
    this->x = 0;
    this->y = 0;
    this->z = 0;
}
double Voxel::distance(Voxel v)
{
    return sqrt(double ((v.x-x)*(v.x-x)+(v.y-y)*(v.y-y)+(v.z-z)*(v.z-z) ));
}
double Voxel::distance(long cx, long cy, long cz)
{
    return sqrt(double ((cx-x)*(cx-x)+(cy-y)*(cy-y)+(cz-z)*(cz-z)));
}

double Voxel::distance(LocationSimple node)
{
    return sqrt((node.x-x)*(node.x-x)+(node.y-y)*(node.y-y)+(node.z-z)*(node.z-z));
}

bool Voxel::isNeighbor(Voxel v)
{   // 26 connected analysis
    if( (1==qAbs(v.x-x)) || (1==qAbs(v.y-y)) || (1==qAbs(v.z-z)) )
    {
        return true;
    }
    else
        return false;
}

