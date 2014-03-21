/*
   Copyright 2010 Jingpeng Wu

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "mostimage.h"
#include "mostVesselTracer.h"
#include <QFileDialog>

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

float MOSTImage::getRayburstRadius(  V_NeuronSWC_unit &center, const bool &force, const int &rayburstThreshold )
{
    if (!force && center.r != 10)
        return center.r;
    unsigned char* img3d = this->getRawData();
    V3DLONG dim0 = this->getXDim();
    V3DLONG dim1 = this->getYDim();
    V3DLONG dim2 = this->getZDim();
    float x = center.x;
    float y = center.y;
    float z = center.z;
    float zthickness = 1.0;

    double max_r = dim0/2;
    if (max_r > dim1/2) max_r = dim1/2;
            if (max_r > (dim2*zthickness)/2) max_r = (dim2*zthickness)/2;

            double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir++)
    {
            total_num = background_num = 0;

            double dz, dy, dx;
            double zlower = -ir/zthickness, zupper = +ir/zthickness;
            for (dz= zlower; dz <= zupper; ++dz)
                    for (dy= -ir; dy <= +ir; ++dy)
                            for (dx= -ir; dx <= +ir; ++dx)
                            {
                                    total_num++;

                                    double r = sqrt(dx*dx + dy*dy + dz*dz);
                                    if (r>ir-1 && r<=ir)
                                    {
                                            V3DLONG i = x+dx;	if (i<0 || i>=dim0) goto end;
                                            V3DLONG j = y+dy;	if (j<0 || j>=dim1) goto end;
                                            V3DLONG k = z+dz;	if (k<0 || k>=dim2) goto end;
                                            long offset =k*dim0*dim1+j*dim0+i;

                                            if (img3d[offset] <= rayburstThreshold)
                                            {
                                                    background_num++;

                                                    if ((background_num/total_num) > 0.022)	goto end; //change 0.01 to 0.001 on 100104
                                            }
                                    }
                            }
    }
end:
    center.r = ir;
    return center.r;
}

/*
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
*/
int MOSTImage::getAdaptiveThreshold(const V_NeuronSWC_unit &pivot, const float &radius)
{
    // use constant global threshold ?
    //return InitThreshold;

    // get bounding box
    int x1 = int(pivot.x - radius);
    int y1 = int(pivot.y - radius);
    int z1 = int(pivot.z - radius);

    int x2 = int(pivot.x + radius);
    int y2 = int(pivot.y + radius);
    int z2 = int(pivot.z + radius);

    int min_value = 255;
    int max_value = 0;
    int temp_value;
    int old_value = 0;
    int temp1 = 0;
    int temp2=0;
    int sum =0;
    int count =0;
    long xx1=0;
    long xx2=0;
    long x1i=0;
    long x2i=0;

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
                min_value = min_value < this->getRawData()[offset] ? min_value : this->getRawData()[offset];
                max_value = max_value > this->getRawData()[offset] ? max_value : this->getRawData()[offset];
            }
        }
    }
   // temp_value =otsu2(hist);

    for(int i=min_value;i<=max_value;i++)
    {
        sum += hist[i]*i;
        count += hist[i];
    }
    temp_value = sum/count +5;

    return temp_value;



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

    long zend = this->getZDim();
    long yend = this->getYDim();
    long xend = this->getXDim();

    uNbClt.clear();

    // the same parent node
    uNbClt.parent_node_ID = clt.parent_node_ID;

    for ( long n = 0; n < clt.voxels.size(); n++ )
    {   // coordinate of every voxel
        long xc = clt.voxels[n].x;
        long yc = clt.voxels[n].y;
        long zc = clt.voxels[n].z;

        for ( long z = zc - 2; z <= zc + 2; z++ )
        {
            if ( z < 0 || z >= zend )
                continue;
            long offsetz = z * this->getTotalUnitNumberPerPlane();//this->getYDim() * this->getXDim();
            for (long y = yc -2; y <= yc + 2; y++ )
            {
                if ( y < 0 || y >= yend )
                    continue;
                long offsety = y * this->getXDim();
                for (long x = xc - 2; x <= xc + 2; x++)
                {
                    if ( x < 0 || x >= xend )
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
    uNbClt.threshold=clt.threshold;
    return uNbClt;
}

// create new node according to Eq.(1)

V_NeuronSWC_unit MOSTImage::createNode_V2(VoxelCluster &conCom)
{
    V_NeuronSWC_unit newNode = conCom.getCenter();
    //newNode = this->recenter(newNode, conCom.threshold);
    newNode.r = this->getRayburstRadius(newNode, true, conCom.threshold);
    newNode.parent = conCom.parent_node_ID;
    long parentID= newNode.parent;
    float parent_r = swc.row.at(parentID-1).r;
    if((parent_r-newNode.r<=2&& parent_r-newNode.r>=-2)||(newNode.r-parent_r<=2&&newNode.r-parent_r>=-2))
    {
        newNode.r = (newNode.r+parent_r)/2;
    }
    newNode.type = 3;

    return newNode;

}
V_NeuronSWC_unit MOSTImage::createNode_br(VoxelCluster &conCom)
{
    V_NeuronSWC_unit newNode = conCom.getCenter();
    //newNode = this->recenter(newNode, conCom.threshold);
    newNode.r = this->getRayburstRadius(newNode, true, conCom.threshold);
    vector<V3DLONG> node_ID =swc.getIndexofParent_nodeid(conCom.parent_node_ID);
    long node = node_ID.back();

    newNode.parent = node;
    newNode.type = 3;

    return newNode;

}
V_NeuronSWC_unit MOSTImage::createNode(VoxelCluster &conCom, const V_NeuronSWC_unit &preNode, const double &preSize)
{
    V_NeuronSWC_unit newNode;
    V_NeuronSWC_unit center = conCom.getCenter();
    double sr, curSize = conCom.getSize();

    if ( curSize <= preSize )
         sr = curSize / preSize;
    else
         sr = preSize / curSize;

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
    clt.threshold=threshold;
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

V_NeuronSWC MOSTImage::trace_single_seed ( const LocationSimple &seed,const int InitThreshold , int slipsize)
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
    float timescoop= 0;
    float timefindun =0;
    float timesplit = 0;
    float timerecent =0;
    float timecreat = 0;

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

    //tmpClt.appendVoxel(swcUnitTmp.x,swcUnitTmp.y, swcUnitTmp.z);
    tmpClt.threshold = InitThreshold;

  //  tmpClt.threshold = this->getAdaptiveThreshold(swcUnitTmp,3*swcUnitTmp.r);

    QTime qtime;
     qtime.start();
    tmpClt = this->voxelScooping_V2( swcUnitTmp,  swcUnitTmp.r, tmpClt.threshold );
    qDebug("  ---$$$$----cost time = %g sec", qtime.elapsed()*0.001);
//    VoxelCluster tmpClt = this->voxelScooping_V2( swcUnitTmp, 1.5 * swcUnitTmp.r);
    tmpClt.parent_node_ID = swc.nrows();//nodeList.count();

    QList < VoxelCluster > conCltList;
    QList < VoxelCluster > clusterList;

    QList < VoxelCluster > preCltList;
    QList < V_NeuronSWC_unit > preNodeList;
    QList < V_NeuronSWC_unit > tempNodeList;
    V_NeuronSWC_unit preNode;
    VoxelCluster preClt;
    clusterList.append( tmpClt );
    int is_seed = 0;
    // start tracing !
    while( ! clusterList.isEmpty() )
    {
        tmpClt.clear();
        QTime qtime;
        qtime.start();
        tmpClt = this->findUnvisitedNeighbor(clusterList.last());
        timefindun += qtime.elapsed()*0.001;
        clusterList.removeLast();
        if(is_seed == 0)
        {
            preCltList = tmpClt.split(slipsize);
            for(int i=1 ;i<preCltList.count();i++)
                preCltList.removeLast();
            preNodeList.append(swcUnitTmp);
            is_seed++;
        }
        conCltList.clear();
        QTime qtimesplit;
        qtimesplit.start();
        conCltList = tmpClt.split(slipsize);
      //  if(conCltList.size()==2&&tmpClt.parent_node_ID!=1)
      //  {
     //       swc.row.pop_back();
     //   }

            int test =2;

        timesplit +=qtimesplit.elapsed()*0.001;


        // small vessel first, to reduce tracing gap.
        // Maybe caused by big vessel scooping covers small vessels


        for (long j = conCltList.count()-1; j >= 0 ; j--)
        {
            // get new node
            QTime timecreater;
            timecreater.start();
           // swcUnitTmp = this->createNode(conCltList[j], preNodeList.last(), preCltList.last().getSize());
            swcUnitTmp = this->createNode_V2(conCltList[j]);

          /*  if(conCltList.size()==2&&tmpClt.parent_node_ID!=1)
            {
                swcUnitTmp = this->createNode_br(conCltList[j]);

            }
            else*/
               //

            timecreat+= timecreater.elapsed()*0.001;


//            QTime time_ren;
 //           time_ren.start();
           // swcUnitTmp = this->recenter(swcUnitTmp, conCltList[j].threshold);
  //          timerecent += time_ren.elapsed()*0.001;


            // skip the thin tubes
            if ( swcUnitTmp.r <= MinRadius )
                continue;

            // append node to swc network, the swc network coordinate start from 1
            swcUnitTmp.n = swc.nrows()+1;
            swc.append(swcUnitTmp);
            if(swc.row.size() == 48)
                int test =1;

           //  voxel scooping, get scooping distance
         /*   float scoopingDistance = 1.1*conCltList[j].getScoopingDistance(swcUnitTmp);
            tmpClt.clear();
            tmpClt.threshold = this->getAdaptiveThreshold(swcUnitTmp, 3 * scoopingDistance);
            tmpClt = this->voxelScooping_V2( swcUnitTmp, scoopingDistance, tmpClt.threshold);
            tmpClt.parent_node_ID = swc.nrows();
            clusterList.append(tmpClt);*/

           conCltList[j].parent_node_ID = swc.nrows();
           conCltList[j].threshold = InitThreshold;

         //  conCltList[j].threshold = this->getAdaptiveThreshold(swcUnitTmp, swcUnitTmp.r *3);
           clusterList.append(conCltList[j]);


            tempNodeList.append(swcUnitTmp);
        }
      //  if(conCltList.count()==0)
    //    {
    //        preCltList.removeLast();
   //         preNodeList.removeLast();
   //     }
        if(! preCltList.isEmpty())
            preCltList.removeLast();
        if(!preNodeList.isEmpty())
        preNodeList.removeLast();
        for (long j = conCltList.count()-1; j >= 0 ; j--)
             preCltList.append(conCltList[j]);
        for(long i = 0;i<tempNodeList.count();i++)
            preNodeList.append(tempNodeList[i]);
        for(long i = tempNodeList.count()-1;i>=0;i--)
            tempNodeList.removeLast();


    }

    // return swc
   // swc.b_linegraph = false;
    //swc = segmentPruning(swc);
   // swc.check_data_consistency();
    qDebug("cost$scooping$$time = %g sec",timescoop);
    qDebug("cost$findun$$time = %g sec",timefindun);
    qDebug("cost$split$$time = %g sec",timesplit);
    qDebug("cost$recenter$$time = %g sec",timerecent);
    qDebug("cost&creat&&time = %g sec",timecreat);
    return swc;
}

NeuronTree MOSTImage::trace_seed_list(const LandmarkList &seedList, std::vector<bool> &visited_cur,const int InitThreshold,const double res_x_all,const double res_y_all,const double res_z_all,QString swcfile,int slipsize,int pruning_flag)
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
            QTime qtime;
             qtime.start();
            swc = trace_single_seed ( seedList[n] ,InitThreshold , slipsize) ;

            for(int i = 0;i<swc.row.size();i++)
            {
                swc.row.at(i).x *=res_x_all;
                swc.row.at(i).y *=res_y_all;
                swc.row.at(i).z *=res_z_all;
                swc.row.at(i).r *=res_x_all;

            }
            qDebug("  -------single seed cost time = %g sec", qtime.elapsed()*0.001);
            swcList.append(swc);
        }
    }

    qDebug("stop!!!!!!!!!!!!!!!");




    //  QString curFile = QFileDialog::getSaveFileName(this, "保存图片",QDir::currentPath(), "Images (*.png *.bmp *.jpg)");

    qDebug("write00000000000000000000   OK!!!!!!!!!!!!!!!");
   // V_NeuronSWC swclll =  segmentPruning(swcList);



      // QString curFile = "/home/most/jieguo/auto_v1_.swc";
     //   QString curFile = QFileDialog::getSaveFileName( 0, "Save as SWC", "auto.swc", "*.swc" );
//
               V_NeuronSWC merged_neuron = merge_V_NeuronSWC_list(swcList);
               if(pruning_flag)
               {
                    merged_neuron = segmentPruning(merged_neuron);
               }

               FILE * fp = fopen(swcfile.toAscii(), "wt");
               for (int i=0;i<merged_neuron.row.size(); i++)
               {
                    fprintf(fp, "%ld %ld %5.3f %5.3f %5.3f %5.3f %ld\n",
                                           V3DLONG(merged_neuron.row[i].data[0]), V3DLONG(merged_neuron.row[i].data[1]), merged_neuron.row[i].data[2], merged_neuron.row[i].data[3], merged_neuron.row[i].data[4], merged_neuron.row[i].data[5], V3DLONG(merged_neuron.row[i].data[6]));
               }
               fclose(fp);

                qDebug("write111    OK!!!!!!!!!!!!!!!");

    // converte the formate

    NeuronTree vt;

    vt = V_NeuronSWC_list__2__NeuronTree(swcList);

    // verify coordinate for visualization, it seems to be 1-based
   /* for (register long i = 0; i < vt.listNeuron.count(); i++ )
    {
        vt.listNeuron[i].x += 1;
        vt.listNeuron[i].y += 1;
        vt.listNeuron[i].z += 1;
    }*/
    return vt;
}

void MOSTImage::auto_detect_seedz(LandmarkList &seedList,V3DLONG slice,const int InitThreshold,const int seed_size_all)
{
   {
    V3DLONG zz=slice;
    V3DLONG xx=this->getXDim();
    V3DLONG yy=this->getYDim();
    VoxelCluster all_seed_region;
    QList < VoxelCluster > conCltList;
    LocationSimple seed;
    for(int i =0;i<xx;i++)
    {
        for(int j=0;j<yy;j++)
        {
            long offset =  zz * getTotalUnitNumberPerPlane() +  j * getXDim() + i;
            if ( this->getRawData()[offset] > InitThreshold)
                all_seed_region.appendVoxel(i,j,zz);
         }
    }
    conCltList = all_seed_region.split2Dz(xx,yy,zz,seed_size_all);

    for(int i=0;i<conCltList.size();i++)
    {
        int cx=0;
        int cy=0;
        for ( int j = 0; j< conCltList.at(i).voxels.size();j++)
        {
            cx += conCltList.at(i).voxels[j].x;
            cy += conCltList.at(i).voxels[j].y;
         }
        //average
        seed.x= cx / conCltList.at(i).voxels.size();
        seed.y= cy / conCltList.at(i).voxels.size();
        seed.z= zz;
        seedList.append(seed);
      }

   }
  /* {
    V3DLONG zz=this->getZDim()-1;
    V3DLONG xx=this->getXDim();
    V3DLONG yy=this->getYDim();
    VoxelCluster all_seed_region;
    QList < VoxelCluster > conCltList;
    LocationSimple seed;
    for(int i =0;i<xx;i++)
    {
        for(int j=0;j<yy;j++)
        {
            long offset =  zz * getTotalUnitNumberPerPlane() +  j * getXDim() + i;
            if ( this->getRawData()[offset] > InitThreshold)
                all_seed_region.appendVoxel(i,j,zz);
         }
    }
    conCltList = all_seed_region.split2Dz(xx,yy,zz);

    for(int i=0;i<conCltList.size();i++)
    {
        int cx=0;
        int cy=0;
        for ( int j = 0; j< conCltList.at(i).voxels.size();j++)
        {
            cx += conCltList.at(i).voxels[j].x;
            cy += conCltList.at(i).voxels[j].y;
         }
        //average
        seed.x= cx / conCltList.at(i).voxels.size();
        seed.y= cy / conCltList.at(i).voxels.size();
        seed.z= zz;
        seedList.append(seed);
      }
    }*/
}

void MOSTImage::auto_detect_seedx(LandmarkList &seedList,V3DLONG slice,const int InitThreshold,const int seed_size_all)
{
   {
    V3DLONG zz=this->getZDim();
    V3DLONG xx=slice;
    V3DLONG yy=this->getYDim();
    VoxelCluster all_seed_region;
    QList < VoxelCluster > conCltList;
    LocationSimple seed;
    for(int i =0;i<yy;i++)
    {
        for(int j=0;j<zz;j++)
        {
            long offset =  j * getTotalUnitNumberPerPlane() +  i * getXDim() + xx;
            if ( this->getRawData()[offset] > InitThreshold)
                all_seed_region.appendVoxel(xx,i,j);
         }
    }
    conCltList = all_seed_region.split2Dx(xx,yy,zz,seed_size_all);

    for(int i=0;i<conCltList.size();i++)
    {
        int cy=0;
        int cz=0;
        for ( int j = 0; j< conCltList.at(i).voxels.size();j++)
        {
            cz += conCltList.at(i).voxels[j].z;
            cy += conCltList.at(i).voxels[j].y;
         }
        //average
        seed.z= cz / conCltList.at(i).voxels.size();
        seed.y= cy / conCltList.at(i).voxels.size();
        seed.x= xx;
        seedList.append(seed);
      }

   }
}

void MOSTImage::auto_detect_seedy(LandmarkList &seedList,V3DLONG slice,const int InitThreshold,const int seed_size_all)
{
   {
    V3DLONG zz=this->getZDim();
    V3DLONG xx=this->getXDim();
    V3DLONG yy=slice;
    VoxelCluster all_seed_region;
    QList < VoxelCluster > conCltList;
    LocationSimple seed;
    for(int i =0;i<xx;i++)
    {
        for(int j=0;j<zz;j++)
        {
            long offset =  j * getTotalUnitNumberPerPlane() +  yy * getXDim() + i;
            if ( this->getRawData()[offset] > InitThreshold)
                all_seed_region.appendVoxel(i,yy,j);
         }
    }
    conCltList = all_seed_region.split2Dy(xx,yy,zz,seed_size_all);

    for(int i=0;i<conCltList.size();i++)
    {
        int cx=0;
        int cz=0;
        for ( int j = 0; j< conCltList.at(i).voxels.size();j++)
        {
            cz += conCltList.at(i).voxels[j].z;
            cx += conCltList.at(i).voxels[j].x;
         }
        //average
        seed.z= cz / conCltList.at(i).voxels.size();
        seed.x= cx / conCltList.at(i).voxels.size();
        seed.y= yy;
        seedList.append(seed);
      }

   }
}
