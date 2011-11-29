#include "mostimage.h"

MOSTImage::MOSTImage()
{
//    this->cleanExistData();
//    this->MinRadius = 1;
}
MOSTImage::~MOSTImage()
{
//    this->~Image4DSimple();
//    this->visited.clear();
//    this->swc.clear();
}

//bool MOSTImage::initVisited()
//{
//    visited.fill(false,getTotalUnitNumberPerChannel());
//    return true;
//}

bool MOSTImage::isVisted( const LocationSimple &loc)
{
    long offset = long(loc.z) * getTotalUnitNumberPerPlane() + long(loc.y) * getXDim() + long(loc.x);
    return visited[offset];
}

float MOSTImage::getRayburstRadius( V_NeuronSWC_unit &center, const bool &force, const int &rayburstThreshold )
{
    if (!force && center.r != 10)
        return center.r;

    //init
    SRBFAN     *fan;
    SRBPARAMS  params;

    //set the params
    params.image = this->getRawData();
    params.voxel_width    = 1.0;
    params.voxel_height   = 1.0;
    params.voxel_length   = 1.0;
    params.fit_percent    = 1.0;
    params.max_iterations = 10;
    params.image_width    = this->getXDim();
    params.image_height   = this->getYDim();
    params.image_length   = this->getZDim();
    params.origin_x       = center.x;
    params.origin_y       = center.y;
    params.origin_z       = center.z;
    //set the threshold for rayburst
    params.threshold      = rayburstThreshold;

    //get the fan and return diameter
    fan  = srb_run_2d( &params );

    if (fan->diameter < MinRadius * 2 )//set the minimun diameter
        center.r = 1;
    else
    {
        if (fan->diameter > MaxRadius * 2)
            center.r = MaxRadius;
        else
            center.r = fan->diameter / 2 ;
    }
    //cout<<"the diameter: "<<fan->diameter<<endl;
    srb_fan_destroy( fan );

    return center.r;
}

int MOSTImage::getAdaptiveThreshold(const V_NeuronSWC_unit &pivot, const float &radius)
{
    // use constant global threshold ?
    return InitThreshold;

    // get bounding box
    int x1 = int(pivot.x - radius);
    int y1 = int(pivot.y - radius);
    int z1 = int(pivot.z - radius);

    int x2 = int(pivot.x + radius);
    int y2 = int(pivot.y + radius);
    int z2 = int(pivot.z + radius);

    if ( x1 < 0 )   x1 = 0;
    if ( y1 < 0 )   y1 = 0;
    if ( z1 < 0 )   z1 = 0;

    if ( x2 >= this->getXDim() )    x2 = this->getXDim() - 1;
    if ( y2 >= this->getYDim() )    y2 = this->getYDim() - 1;
    if ( z2 >= this->getZDim() )    z2 = this->getZDim() - 1;

    // get histogram
    int hist[256];
    memset(hist, 0, sizeof(hist));
    for (int k = z1; k <= z2; k++)
    {
        long offsetz = k * this->getTotalUnitNumberPerPlane();
        for (int j = y1; j <= y2; j++)
        {
            long offsety = j * this->getXDim();
            for (int i = x1; i <= x2; i++)
            {
                long offset = offsetz + offsety + i;
                int tmp = this->getRawData()[offset];
                hist[ this->getRawData()[offset] ] ++;
            }
        }
    }

    // get otsu threshold
    return otsu(hist);

}

V_NeuronSWC_unit MOSTImage::recenter(V_NeuronSWC_unit &seed, const int &threshold)
{
    // if radius equale to initiate state , get the real diameter
    //    if ( seed.radius == 5)
    float r = this->getRayburstRadius( seed, true, threshold);

    // get bounding box
    int rBox = 2 * seed.r;
    int xb = ( (seed.x-rBox) > 0 ) ? (seed.x-rBox) : 0;
    int xe = ( (seed.x+rBox) < this->getXDim() ) ? (seed.x+rBox) : this->getXDim();
    int yb = ( (seed.y-rBox) > 0 ) ? (seed.y-rBox) : 0;
    int ye = ( (seed.y+rBox) < this->getYDim() ) ? (seed.y+rBox) : this->getYDim();
    int zb = ( (seed.z-rBox) > 0 ) ? (seed.z-rBox) : 0;
    int ze = ( (seed.z+rBox) < this->getZDim() ) ? (seed.z+rBox) : this->getZDim();

    // init sum
    long double xsum = 0, ysum = 0, zsum = 0, gsum = 0;

    for (register int i = long(zb); i < long(ze); i++)
    {
        for (register int j = long(yb); j < long(ye); j++ )
        {
            for (register int k = long(xb); k < long(xe); k++)
            {
                long offset = i*this->getTotalUnitNumberPerPlane() + j*this->getXDim() + k;
                if (this->getRawData()[offset] > threshold)
                {
                    zsum += i;
                    ysum += j;
                    xsum += k;
                    gsum++;
                }
                //                zsum += i * this->getRawData()[ offset ];
                //                ysum += j * this->getRawData()[ offset ];
                //                xsum += k * this->getRawData()[ offset ];
                //                gsum += this->getRawData()[ offset ];
            }//for k
        }//for j
    }//for i
    seed.x = long(xsum / gsum);
    seed.y = long(ysum / gsum);
    seed.z = long(zsum / gsum);
    seed.r = this->getRayburstRadius(seed,true, threshold);
    return seed;
}

VoxelCluster MOSTImage::findUnvisitedNeighbor(VoxelCluster &clt)
{
    VoxelCluster uNbClt;
    uNbClt.clear();

    // the same parent node
    uNbClt.parent_node_ID = clt.parent_node_ID;

    for ( long n = 0; n < clt.voxels.size(); n++ )
    {   // coordinate of every voxel
        long xc = clt.voxels[n].x;
        long yc = clt.voxels[n].y;
        long zc = clt.voxels[n].z;

        for ( long z = zc - 1; z <= zc + 1; z++ )
        {
            if ( z < 0 || z >= this->getZDim() )
                continue;
            long offsetz = z * this->getTotalUnitNumberPerPlane();//this->getYDim() * this->getXDim();
            for (long y = yc -1; y <= yc + 1; y++ )
            {
                if ( y < 0 || y >= this->getYDim() )
                    continue;
                long offsety = y * this->getXDim();
                for (long x = xc - 1; x <= xc + 1; x++)
                {
                    if ( x < 0 || x >= this->getXDim() )
                        continue;
                    long offset  = offsetz + offsety + x;
                    if ( ( visited[ offset ] == false ) && ( this->getRawData()[offset] > clt.threshold ) )
                    {   // unvisited voxel and the voxel value is greater than threshold
                        uNbClt.appendVoxel(x,y,z);
                        visited[ offset ] = true;
                    }
                } // for x
            } // for y
        } // for z
    } // for n
    return uNbClt;
}

// create new node according to Eq.(1)
V_NeuronSWC_unit MOSTImage::createNode(VoxelCluster &conCom, const V_NeuronSWC_unit &preNode, const double &preSize)
{
    V_NeuronSWC_unit newNode;
    V_NeuronSWC_unit center = conCom.getCenter();
    double sr, curSize = conCom.getSize();

    if ( curSize <= preSize )
        double sr = curSize / preSize;
    else
        double sr = preSize / curSize;

    // new node coordinate
    newNode.x = preNode.x + pow( 0.5, sr ) * ( center.x - preNode.x );
    newNode.y = preNode.y + pow( 0.5, sr ) * ( center.y - preNode.y );
    newNode.z = preNode.z + pow( 0.5, sr ) * ( center.z - preNode.z );

    // update the radius and parent ID
    newNode.r = this->getRayburstRadius(newNode, true, conCom.threshold);
    newNode.parent = conCom.parent_node_ID;
    newNode.type = 3;
    return newNode;
}

VoxelCluster MOSTImage::voxelScooping_V2 (const V_NeuronSWC_unit &pivot, const float &sd, const int &threshold )
{
    VoxelCluster clt;
    clt.clear();

    // get bounding box
    int xb = ( (pivot.x-sd) > 0 ) ? (pivot.x-sd) : 0;
    int xe = ( (pivot.x+sd) < this->getXDim() ) ? (pivot.x+sd) : this->getXDim();
    int yb = ( (pivot.y-sd) > 0 ) ? (pivot.y-sd) : 0;
    int ye = ( (pivot.y+sd) < this->getYDim() ) ? (pivot.y+sd) : this->getYDim();
    int zb = ( (pivot.z-sd) > 0 ) ? (pivot.z-sd) : 0;
    int ze = ( (pivot.z+sd) < this->getZDim() ) ? (pivot.z+sd) : this->getZDim();

    for (register long i = long(zb); i < long(ze); i++)
    {
        long offsetz = i * this->getTotalUnitNumberPerPlane();
        for ( register long j = long(yb); j < long(ye); j++ )
        {
            long offsety = j * this->getXDim();
            for (register long k = long(xb); k < long(xe); k++)
            {
                long offset = offsetz + offsety + k;
                if ( (visited[offset] == false) && ( this->getRawData()[offset] > threshold ) )
                {   // unvisited and the voxel value is greater than threshold
                    double d_sq = (k-pivot.x)*(k-pivot.x) + (j-pivot.y)*(j-pivot.y) + (i-pivot.z)*(i-pivot.z);
                    if ( d_sq < sd * sd )
                    {   // within scooping distance
                        clt.appendVoxel(k,j,i);
                        visited[offset] = true;
                    }
                }
            }//for k
        }//for j
    }//for i
    return clt;
}

VoxelCluster MOSTImage::voxelScooping(VoxelCluster &conClt, const V_NeuronSWC_unit &pivot, const float &sd, const int &threshold)
{
    VoxelCluster clt;

    V_NeuronSWC_unit seed;
    for (long n = 0; n < conClt.voxels.size(); n++ )
    {
        Voxel vtmp = conClt.voxels[n];
        // 26 connectivity growing
        for (register int i = vtmp.x-1; i <= vtmp.x+1; i++)
        {
            if ( ( i <= 0 ) || ( i >= this->getXDim() - 1 ) )
            {   // out of image boundary
                continue;
            }
            for ( register int j = vtmp.y-1; j <= vtmp.y+1; j++)
            {
                if ( ( j <= 0 ) || ( j >= this->getYDim() - 1 ) )
                {   // out of image boundary
                    continue;
                }
                for (register int k = vtmp.z-1; k <= vtmp.z+1; k++)
                {
                    if ( ( k <= 0 ) || ( k >= this->getZDim() - 1 ) )
                    {   // out of image boundary
                        continue;
                    }
                    if ( i==vtmp.x && j==vtmp.y && k==vtmp.z )
                    {   // it's the vtmp itself
                        continue;
                    }
                    else
                    {   // grow to next vtmp , grow recuesively
                        V_NeuronSWC_unit tmpSeed;
                        tmpSeed.x = i;
                        tmpSeed.y = j;
                        tmpSeed.z = k;
                        regionGrowInSphere_V2(pivot, sd, tmpSeed, clt, threshold );
                    }
                }//for k
            }//forj
        }//for i
    }
    return clt;
}

// use seed list instead of recursive mode
bool MOSTImage::regionGrowInSphere_V2(const V_NeuronSWC_unit &pivot, const float &radius, V_NeuronSWC_unit &seed, VoxelCluster &clt, const int &threshold)
{
    // initialize seed list
    QList < V_NeuronSWC_unit > seedList;
    seedList.append(seed);

    while ( ! seedList.isEmpty() )
    {
        seed = seedList.last();
        seedList.removeLast();
        // offset
        long offset = long(seed.z) * this->getTotalUnitNumberPerPlane() + long(seed.y) * this->getXDim() + long(seed.x);

        // check visited
        if ( true == visited[ offset ] )
            continue;

        // check voxel gray value
        if ( this->getRawData()[offset] < threshold )
            continue;

        // check distance
        double distance_sq =  (seed.x - pivot.x)*(seed.x - pivot.x) + (seed.y - pivot.y)*(seed.y - pivot.y) + (seed.z - pivot.z)*(seed.z - pivot.z) ;
        if ( distance_sq > radius * radius )
            continue;

        //append this voxel and mark visited
        clt.appendVoxel(int(seed.x),int (seed.y), int(seed.z));
        visited[ offset ] = true;

        // 26 connectivity growing
        for ( register int i = seed.x-1; i <= seed.x+1; i++ )
        {
            if ( ( i <= 0 ) || ( i >= this->getXDim() - 1 ) )
            {   // out of image boundary
                continue;
            }
            for ( register int j = seed.y-1; j <= seed.y+1; j++)
            {
                if ( ( j <= 0 ) || ( j >= this->getYDim() - 1 ) )
                {   // out of image boundary
                    continue;
                }
                for ( register int k = seed.z-1; k <= seed.z+1; k++)
                {
                    if ( ( k <= 0 ) || ( k >= this->getZDim() - 1 ) )
                    {   // out of image boundary
                        continue;
                    }
                    else if ((seed.x == i) && (seed.y == j) && (seed.z == k) )
                    {   // it's the seed itself
                        continue;
                    }
                    {   // append to seed list for further check
                        V_NeuronSWC_unit tmpSeed;
                        tmpSeed.x = i;
                        tmpSeed.y = j;
                        tmpSeed.z = k;
                        seedList.append(tmpSeed);
                    }
                }//for k
            }//for j
        }//for i
    }// while
    return true;
}

V_NeuronSWC MOSTImage::trace_single_seed ( const LocationSimple &seed )
{
//    // test qlist
//    QList < Voxel > voxList;
//    for (int i = 0; i < 57279; i++ )
//    {
//        Voxel vox;
//        vox.x = i;
//        vox.y = i;
//        vox.z = i;
//        vox.c = 0;
//        voxList.append(vox);
//    }

    swc.clear();
    // iteration 1 : init
    swcUnitTmp.set(seed.x, seed.y, seed.z);
    swcUnitTmp.y = seed.y;
    swcUnitTmp = this->recenter(swcUnitTmp, InitThreshold);
    swcUnitTmp.r = this->getRayburstRadius(swcUnitTmp, true, InitThreshold);

    // swc network, the swcUnit coordinate seems starts from 1
    swcUnitTmp.type = 3;
    swcUnitTmp.parent = -1;
    swcUnitTmp.n = 1;
    swcUnitTmp.seg_id = 0;
    swc.append(swcUnitTmp);

    VoxelCluster tmpClt;
    tmpClt.appendVoxel(swcUnitTmp.x,swcUnitTmp.y, swcUnitTmp.z);
    tmpClt.threshold = InitThreshold;
    tmpClt = this->voxelScooping( tmpClt, swcUnitTmp, 1 * swcUnitTmp.r, tmpClt.threshold );
//    VoxelCluster tmpClt = this->voxelScooping_V2( swcUnitTmp, 1.5 * swcUnitTmp.r);
    tmpClt.parent_node_ID = swc.nrows();//nodeList.count();

    QList < VoxelCluster > conCltList;
    QList < VoxelCluster > clusterList;
    clusterList.append( tmpClt );
    // start tracing !
    while( ! clusterList.isEmpty() )
    {
        tmpClt.clear();
        tmpClt = this->findUnvisitedNeighbor(clusterList.last());
        clusterList.removeLast();
        conCltList.clear();
        conCltList = tmpClt.split();

        // small vessel first, to reduce tracing gap.
        // Maybe caused by big vessel scooping covers small vessels
        for (long j = conCltList.count()-1; j >= 0 ; j--)
        {
            // get new node
            swcUnitTmp = this->createNode(conCltList[j], swcUnitTmp, tmpClt.getSize());
            swcUnitTmp = this->recenter(swcUnitTmp, conCltList[j].threshold);

            // skip the thin tubes
            if ( swcUnitTmp.r <= MinRadius )
                continue;

            // append node to swc network, the swc network coordinate start from 1
            swcUnitTmp.n = swc.nrows()+1;
            swc.append(swcUnitTmp);
            if(swc.row.size() == 48)
                int test =1;

            // voxel scooping, get scooping distance
            float scoopingDistance = conCltList[j].getScoopingDistance(swcUnitTmp);
            tmpClt.clear();
            tmpClt.threshold = this->getAdaptiveThreshold(swcUnitTmp, 10 * scoopingDistance);
            tmpClt = this->voxelScooping(conCltList[j], swcUnitTmp, scoopingDistance, tmpClt.threshold);
            tmpClt.parent_node_ID = swc.nrows();

            if( ! tmpClt.voxels.empty() )
                clusterList.append(tmpClt);
        }
    }

    // return swc
    swc.b_linegraph = false;
    swc = segmentPruning(swc);
    swc.check_data_consistency();
    return swc;
}

NeuronTree MOSTImage::trace_seed_list(const LandmarkList &seedList, QVector<bool> &visited_cur)
{
    visited = visited_cur;
    V_NeuronSWC swc;  // vessel tree
    V_NeuronSWC_list swcList;
    for (int n = 0; n < seedList.count(); n++)
    {
        // check boundary
        if ( (seedList.at(n).x >= this->getXDim()) || (seedList.at(n).y >= this->getYDim()) || (seedList.at(n).z >= this->getZDim()) )
            continue;

        long offset = long(seedList[n].z) * getTotalUnitNumberPerPlane() + long(seedList[n].y) * getXDim() + long(seedList[n].x) ;
        if ( (! isVisted(seedList[n])) && (getRawData()[offset] > InitThreshold ) )
        {   // unvisited seed
            swc = trace_single_seed ( seedList[n] ) ;
            swcList.append(swc);
        }
    }

    // converte the formate
    NeuronTree vt;
    vt = V_NeuronSWC_list__2__NeuronTree(swcList);

    // verify coordinate for visualization, it seems to be 1-based
    for (register long i = 0; i < vt.listNeuron.count(); i++ )
    {
        vt.listNeuron[i].x += 1;
        vt.listNeuron[i].y += 1;
        vt.listNeuron[i].z += 1;
    }
    return vt;
}
