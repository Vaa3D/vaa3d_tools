//consensus_skeleton.cpp
//generate consensus tree from multiple neurons

//(1) use a volume image to recode and accumulate the locations of nodes for all trees
//(2) keep one node for each small window, record its confidence value as a feature value (eswc) by the votes
//(3) final result is a minimum spanning tree from 2)


#include "consensus_skeleton.h"
//#include "kcentroid_cluster.h"
#include "mst_dij.h"
#include "mst_prim.h"
#include <QtGlobal>
#include <math.h>
#include <iostream>
#include <climits>
#include "basic_4dimage.h"
#include "algorithm"

using namespace std;


#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif


struct NeuronSize{
    float x;
    float y;
    float z;
};



unsigned int v_min(vector<unsigned int> x)
{
    sort(x.begin(), x.end());
    return  x[0];
}

unsigned int v_max(vector<unsigned int> x)
{
    sort(x.begin(), x.end());
    return  x[x.size()-1];
}


V3DLONG median(vector<V3DLONG> x)
{
    sort(x.begin(), x.end());
    return  x[x.size()/2];
}


void remove_outliers(vector<NeuronTree> & nt_list)
{  //validate each tree (remove empty tree and extreame big trees, trees that contain straight lines)

   //remove statistically outlisers

   cout <<"no trees are removed"<<endl;

}

struct MyBoundingBox{
    float min_x;
    float min_y;
    float min_z;
    float max_x;
    float max_y;
    float max_z;
};


MyBoundingBox neuron_trees_bb(vector<NeuronTree> nt_list)
{

    MyBoundingBox bb ={0,0,0,0,0,0};

    bb.min_x = LONG_MAX;
    bb.min_y = LONG_MAX;
    bb.min_z = LONG_MAX;
    bb.max_x = 0;
    bb.max_y = 0;
    bb.max_z = 0;
    for (int j = 0; j < nt_list.size(); j++){
        NeuronTree nt = nt_list[j];
        for (int i = 0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC a = nt.listNeuron.at(i);
            bb.min_x = MIN(a.x - a.r,bb.min_x );
            bb.max_x = MAX(a.x + a.r,bb.max_x );
            bb.min_y = MIN(a.y - a.r,bb.min_y );
            bb.max_y = MAX(a.y + a.r,bb.max_y );
            bb.min_z = MIN(a.z - a.r,bb.min_z );
            bb.max_z = MAX(a.z + a.r,bb.max_z );
        }
    }
    return bb;
}


void non_max_suppresion( unsigned char * img1d, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z, Point3D offset,
                         vector<Point3D> &node_list,  vector<unsigned int> &vote_list,unsigned int win_size)
{ // extract the local maximum voted skelenton node locations : node_list
  // the corresponding votes are collected in: vote_list
    V3DLONG num_nodes =0;
    for (V3DLONG id_x = 0 + win_size/2; id_x <  sz_x- win_size/2; id_x++)
        for (V3DLONG id_y = 0 + win_size/2; id_y <  sz_y- win_size/2; id_y++)
            for (V3DLONG id_z = 0 + win_size/2; id_z <  sz_z- win_size/2; id_z++)
            {
                //nn
                unsigned char max_val = 0 ;
                V3DLONG max_idx = 0 ;
                for ( V3DLONG xx = id_x - win_size/2;xx< id_x + win_size/2;xx++)
                    for ( V3DLONG yy = id_y - win_size/2;yy< id_y + win_size/2;yy++)
                        for ( V3DLONG zz = id_z - win_size/2;zz< id_z + win_size/2;zz++)
                        {
                            V3DLONG idx = zz * (sz_x*sz_y) + yy * sz_x + xx;
                            if (img1d[idx] > max_val){
                                max_val = img1d[idx] ;
                                max_idx = idx;
                            }
                        }
                if ( max_val > 0 )
                {// found non-zero max
                    for ( V3DLONG xx = id_x - win_size/2;xx< id_x + win_size/2;xx++)
                        for ( V3DLONG yy = id_y - win_size/2;yy< id_y + win_size/2;yy++)
                            for ( V3DLONG zz = id_z - win_size/2;zz< id_z + win_size/2;zz++)
                            {
                                V3DLONG idx = zz * (sz_x*sz_y) + yy * sz_x + xx;
                                img1d[idx] = 0;
                            }

                    img1d[max_idx] = max_val;
                }
            }

    for (V3DLONG id_x = 0 + win_size/2; id_x <  sz_x- win_size/2; id_x++)
        for (V3DLONG id_y = 0 + win_size/2; id_y <  sz_y- win_size/2; id_y++)
            for (V3DLONG id_z = 0 + win_size/2; id_z <  sz_z- win_size/2; id_z++)
            {
                V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;

                if (img1d[idx] >0)
                {
                    num_nodes++;
                    Point3D p;
                    p.x = id_x+offset.x;
                    p.y = id_y+offset.y;
                    p.z = id_z+offset.z;
                    node_list.push_back(p);
                    vote_list.push_back(img1d[idx]);
                }
            }
return;
}




QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
    QHash<V3DLONG, V3DLONG> neuron_id_table;
    for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
        neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
    return neuron_id_table;
}

void AddToMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,
                    int imageCount, V3DPluginCallback2 & callback)
{
    NeuronSWC *p_cur = 0;
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = sx*sy;
    V3DLONG tol_sz = pagesz*sz;

    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        V3DLONG i,j,k;
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
        xs = p_cur->x;
        ys = p_cur->y;
        zs = p_cur->z;
        rs = p_cur->r;

        double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

        ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
        ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
        if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

        bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
        bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
        if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

        ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
        ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
        if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

        //mark all voxels close to the swc node(s)
        for (k = ballz0; k <= ballz1; k++){
            for (j = bally0; j <= bally1; j++){
                for (i = ballx0; i <= ballx1; i++){
                    V3DLONG ind = (k)*pagesz + (j)*sx + i;
                    ind = MIN(ind,tol_sz);
                    ind = MAX(ind, 0);
                    if (pImMask[ind]>imageCount) continue;
                    double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
                    double dt = sqrt(norms10);
                    if(dt <=rs || dt<=1)
                    {
                        pImMask[ind] +=1;
                    }
                }
            }
        }

        //find previous node
        if (p_cur->pn < 0) continue;//then it is root node already
        //get the parent info
        const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
        xe = pp.x;
        ye = pp.y;
        ze = pp.z;
        re = pp.r;

        //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
        if (xe==xs && ye==ys && ze==zs)
        {
            v3d_msg(QString("Detect overlapping coordinates of node\n"), 0);
            continue;
        }

        double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
        double dx = (xe - xs);
        double dy = (ye - ys);
        double dz = (ze - zs);
        double x = xs;
        double y = ys;
        double z = zs;

        int steps = lroundf(l);
        steps = (steps < fabs(dx))? fabs(dx):steps;
        steps = (steps < fabs(dy))? fabs(dy):steps;
        steps = (steps < fabs(dz))? fabs(dz):steps;
        if (steps<1) steps =1;

        double xIncrement = double(dx) / (steps*2);
        double yIncrement = double(dy) / (steps*2);
        double zIncrement = double(dz) / (steps*2);

        V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
        idex1 = MIN(idex1,tol_sz);
        idex1 = MAX(idex1, 0);
        if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
         pImMask[idex1] += 1;

        for (int i = 0; i <= steps; i++)
        {
            x += xIncrement;
            y += yIncrement;
            z += zIncrement;

            x = ( x > sx )? sx : x;
            y = ( y > sy )? sy : y;
            z = ( z > sz )? sz : z;

            V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
            idex = MIN(idex,tol_sz);
            idex = MAX(idex, 0);
            if (pImMask[idex]> imageCount) continue;
            if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
            pImMask[idex] += 1;

        }

        //finding the envelope of the current line segment

        double rbox = (rs>re) ? rs : re;
        double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
        double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
        double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
        double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
        double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
        double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

        //compute cylinder and flag mask

        for (k=z_down; k<=z_top; k++)
        {
            for (j=y_down; j<=y_top; j++)
            {
                for (i=x_down; i<=x_top; i++)
                {
                    double rr = 0;
                    double countxsi = (xs-i);
                    double countysj = (ys-j);
                    double countzsk = (zs-k);
                    double countxes = (xe-xs);
                    double countyes = (ye-ys);
                    double countzes = (ze-zs);
                    double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
                    double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
                    double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
                    double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
                    double t1 = -dots1021/norms21;
                    if(t1<0) dist = sqrt(norms10);
                    else if(t1>1)
                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
                    //compute rr
                    if (rs==re) rr =rs;
                    else
                    {
                        // compute point of intersection
                        double v1 = xe - xs;
                        double v2 = ye - ys;
                        double v3 = ze - zs;
                        double vpt = v1*v1 + v2*v2 +v3*v3;
                        double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
                        double xc = xs + v1*t;
                        double yc = ys + v2*t;
                        double zc = zs + v3*t;
                        double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
                        double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
                        rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
                    }
                    V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;
                    ind1 = MIN(ind1,tol_sz);
                    ind1 = MAX(ind1, 0);
                    if (pImMask[ind1]>imageCount) continue;
                    if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
                    if (dist <= rr || dist<=1)
                    {
                        pImMask[ind1] += 1;
                    }
                }
            }
        }

    }


}

bool vote_map(vector<NeuronTree> & nt_list,  QString outfileName,V3DPluginCallback2 & callback){

    //initialize the image volume to record/accumulate the  location votes from neurons
    MyBoundingBox bbUnion = neuron_trees_bb(nt_list);

    V3DLONG sz_x,sz_y,sz_z;
    //for swcs generated from vaa3d, all meta offset are disgarded, so the swc coordinates are in model coordinates
    sz_x = bbUnion.max_x;
    sz_y = bbUnion.max_y;
    sz_z = bbUnion.max_z;


    V3DLONG  tol_sz = sz_x * sz_y * sz_z;
    cout << "image size = " << tol_sz<<": " <<sz_x<<"x "<<sz_y<<" x"<<sz_z<< endl;

    unsigned char* pImMask = 0;
    pImMask = new unsigned char [tol_sz];
    memset(pImMask,0,tol_sz*sizeof(unsigned char));

    for (int j = 0; j < nt_list.size(); j++){
        NeuronTree nt = nt_list[j];
        AddToMaskImage(nt, pImMask, sz_x, sz_y, sz_z,j,callback);
    }

    Image4DSimple *image = new Image4DSimple();
    image->setData(pImMask, sz_x, sz_y, sz_z, 1, V3D_UINT8);
    callback.saveImage(image, &(outfileName.toStdString()[0]));

    return true;
}


bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & merge_result, int method_code,V3DPluginCallback2 &callback)
{
    double CLUSTERING_RANGE = 10;//threshold to ignore mapping  (too far away) for generting nodeMap below
    //potentially, there are invalid neuron trees (massive node points, no node points, looping)
    remove_outliers(nt_list);
    int neuronNum = nt_list.size();

    //initialize the image volume to record/accumulate the  location votes from neurons
    MyBoundingBox bbUnion = neuron_trees_bb(nt_list);

    Point3D offset = {bbUnion.min_x ,bbUnion.min_y ,bbUnion.min_z };
    float closeness = 1.0;
    V3DLONG  sz_x = ceil((bbUnion.max_x - bbUnion.min_x ) / closeness) +1; //+0.5 to round up from float to V3DLONG
    V3DLONG  sz_y = ceil((bbUnion.max_y - bbUnion.min_y ) / closeness) +1;
    V3DLONG  sz_z = ceil((bbUnion.max_z - bbUnion.min_z ) / closeness) +1;
    V3DLONG  tol_sz = sz_x * sz_y * sz_z;
    cout << "image size = " << tol_sz<<": " <<sz_x<<"x "<<sz_y<<" x"<<sz_z<< endl;

    unsigned char * img1d = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) img1d[i] = 0;


    unsigned char * VOTED = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) VOTED[i] = 0;

    //count
    for (int j =0; j < nt_list.size(); j++){
        NeuronTree nt = nt_list[j];
        //keep a record which idx have been voted by previous nodes in the same tree
        for(V3DLONG ii = 0; ii< tol_sz; ii++) VOTED[ii] = 0;

        for (int i =0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC node = nt.listNeuron.at(i);
            V3DLONG id_x = (node.x-offset.x) +0.5; //round up
            V3DLONG id_y = (node.y-offset.y) +0.5;
            V3DLONG id_z = (node.z-offset.z) +0.5;
            V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;

            if (idx <tol_sz &&  VOTED[idx] == 0 ){
                img1d[idx] ++ ;
                VOTED[idx] = 1;
            }
           /*
             else{
                cout <<"error idx" <<endl;
            }
            */
        }
    }

    //for debug only
    /*
    Image4DSimple *image = new Image4DSimple();
    image->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
    callback.saveImage(image, "./vote_count_image.v3draw");
    */

    //non-maximum suppresion
     vector<Point3D>  node_list;
     vector<unsigned int>  vote_list;
     non_max_suppresion (img1d,sz_x,sz_y,sz_z,offset,node_list,vote_list,3);
     cout << "After non_max supression:"<< endl;
     cout << "number of nodes:"<< node_list.size() << endl;
     cout << "maximum votes:" << v_min(vote_list) << endl;
     cout << "minimum votes:" << v_max(vote_list) << endl;

     //for debug only
     /*
     Image4DSimple *image2 = new Image4DSimple();
     image2->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
     callback.saveImage(image2, "./nms_image.v3draw");
     */

     // for debug: save node_list to check locations
     /*QList<NeuronSWC> locationTree;
     for (int i=0;i<node_list.size();i++)
     {
         NeuronSWC tmp;
         tmp.x = node_list[i].x;
         tmp.y = node_list[i].y;
         tmp.z = node_list[i].z;

         tmp.type = 2; //edge votes
         tmp.pn = -1;  //parent id, form the edge
         tmp.r = double(vote_list[i])/neuronNum*3; //*3 for visulaization
         tmp.n = i+1;

         locationTree.append(tmp);
     }
    export_listNeuron_2swc(locationTree, "./testlocation.swc");
    */

    printf("(2). compute adjacency matrix (vote for edges).\n");


    double * adjMatrix;
    V3DLONG * plist;
    V3DLONG num_nodes = node_list.size();
    try{
        adjMatrix = new double[num_nodes*num_nodes];
        plist = new V3DLONG[num_nodes];
        for (V3DLONG i=0;i<num_nodes*num_nodes;i++) adjMatrix[i] = 0;
    }
    catch (...)
    {
        fprintf(stderr,"fail to allocate memory.\n");
        if (adjMatrix) {delete[] adjMatrix; adjMatrix=0;}
        if (plist) {delete[] plist; plist=0;}
        return false;
    }


    int * EDGE_VOTED = new int[num_nodes*num_nodes];


    for (int i=0;i<neuronNum;i++)
    {
        QHash<V3DLONG, V3DLONG > nodeMap; //maps j node of tree i to consensus node(node_id)
        for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
        {
            NeuronSWC s = nt_list[i].listNeuron.at(j);
            Point3D cur;
            cur.x = s.x;
            cur.y = s.y;
            cur.z = s.z;

            //find its nearest node
            V3DLONG node_id = -1;// this node does not exist
            double min_dis = CLUSTERING_RANGE; //threshold to ignore mapping  (too far away)
            for (V3DLONG ni = 0; ni <node_list.size(); ni++)
            {
                Point3D p = node_list[ni];
                double dis = PointDistance(p,cur);

                if (dis < min_dis){
                    min_dis = dis;
                    node_id = ni;
                }
            }
            if (node_id > -1){
                nodeMap.insert( j, node_id);
            }
        }
        //maps.push_back(nodeMap);

        for (V3DLONG ii=0;ii<num_nodes*num_nodes;ii++) EDGE_VOTED[ii] = 0;

        for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
        {
            NeuronSWC cur = nt_list[i].listNeuron[j];
            // if (cur.pn<0) continue;
            V3DLONG n_id,pn_id;
            n_id = nodeMap[j];
            if (n_id > 0){
                V3DLONG pidx = cur.pn-1;//nt_list[i].hashNeuron.value(cur.pn);  // find the index in nueon_list

                pn_id = nodeMap[pidx];
                if (pn_id > 0  && EDGE_VOTED[n_id*num_nodes + pn_id] ==0 ){
                    adjMatrix[n_id*num_nodes + pn_id] += 1;
                    adjMatrix[pn_id*num_nodes + n_id] += 1;
                    //cout<<adjMatrix[n_id*num_nodes + pn_id] <<endl;
                    EDGE_VOTED[n_id*num_nodes + pn_id] = 1;
                    EDGE_VOTED[pn_id*num_nodes + n_id] = 1;

                }
            }
        }

    }


    if (method_code == 0 ){
        long rootnode =100;
        printf("(3). computing max-spanning tree.\n");

        //if (!mst_dij(adjMatrix, num_nodes, plist, rootnode))
        if (!mst_prim(adjMatrix, num_nodes, plist, rootnode))

        {
            fprintf(stderr,"Error in minimum spanning tree!\n");
            return false;
        }

        printf("(3). genearate consensus graph swc file by assign parents (form edges).\n");

       // code the edge votes into type for visualization
       //         graph: duplicate swc nodes are allowed to accomandate mutiple parents for the child node, no root id,
        merge_result.clear();
        for (V3DLONG i = 0;i <num_nodes;i ++)
        {
            V3DLONG p = plist[i];
            //cout <<p<<endl;
            unsigned int edgeVote = adjMatrix[i*num_nodes + p];

            NeuronSWC tmp;
            tmp.x = node_list[i].x;
            tmp.y = node_list[i].y;
            tmp.z = node_list[i].z;

            tmp.type = edgeVote; //edge votes
            tmp.pn = p + 1;  //parent id, form the edge
            tmp.r = double(vote_list[i])/double(neuronNum);
            tmp.n = i+1;

            merge_result.append(tmp);

        }
    }

//### output vertices ( node lcoations)
//    V3DLONG count = 0;
//    for (V3DLONG i=0;i<num_nodes;i++)
//    {
//            NeuronSWC tmp;
//            tmp.x = node_list[i].x;
//            tmp.y = node_list[i].y;
//            tmp.z = node_list[i].z;
//           // tmp.fea_val.push_back(vote_list[i]);  //location votes are coded into radius

//            tmp.type = 0; //vertices (soma)
//            tmp.pn = -1;  //parent id, no edge
//            tmp.r = double(vote_list[i])/double(neuronNum); //location votes are coded into radius
//            tmp.n = count +1; //id start from 1
//            merge_result.append(tmp);
//            count++;
//    }

//    //output edges, go through half of the symmetric matrix, not directed graph
//    for (V3DLONG row = 0;row <num_nodes;row ++)
//    {
//        for (V3DLONG col = row+1;col < num_nodes;col++){
//            unsigned int edgeVote = adjMatrix[row*num_nodes + col];
//            if (edgeVote > 1)
//            {
//                NeuronSWC tmp;
//                tmp.x = node_list[row].x;
//                tmp.y = node_list[row].y;
//                tmp.z = node_list[row].z;

//                tmp.type = edgeVote; //edge votes
//                tmp.pn = col + 1;  //parent id  , form the edge
//                tmp.r = double(vote_list[row])/double(neuronNum);
//                tmp.n = count+1;

//                merge_result.append(tmp);
//                count++;
//            }
//        }
//    }

    // alternative
  if  (method_code == 1){
        merge_result.clear();
        V3DLONG count = 0;
        for (V3DLONG i=0;i<num_nodes;i++)
        {
                NeuronSWC tmp;
                tmp.x = node_list[i].x;
                tmp.y = node_list[i].y;
                tmp.z = node_list[i].z;
               // tmp.fea_val.push_back(vote_list[i]);  //location votes are coded into radius

                tmp.type = 0; //vertices (soma)
                tmp.pn = -1;  //parent id, no edge
                tmp.r = double(vote_list[i])/double(neuronNum); //location votes are coded into radius
                tmp.n = count +1; //id start from 1
                merge_result.append(tmp);
                count++;
        }

        //output edges, go through half of the symmetric matrix, not directed graph
        for (V3DLONG row = 0;row <num_nodes;row ++)
        {
            for (V3DLONG col = row+1;col < num_nodes;col++){
                unsigned int edgeVote = adjMatrix[row*num_nodes + col];
                if (edgeVote > 0)
                {
                    if (merge_result[row].pn == -1)
                    {//exsiting isolated vertex, modify parent id
                        merge_result[row].type = edgeVote; //edge votes
                        merge_result[row].pn = col + 1;  //parent id  , form the edge
                        merge_result[row].r = double(vote_list[row])/double(neuronNum);
                    }
                    else{
                      //add new edge  , via duplication nodes with different parent id and edge votes
                        NeuronSWC tmp;
                        tmp.x = node_list[row].x;
                        tmp.y = node_list[row].y;
                        tmp.z = node_list[row].z;

                        tmp.type = edgeVote; //edge votes
                        tmp.pn = col + 1;  //parent id  , form the edge
                        tmp.r = double(vote_list[row])/double(neuronNum);
                        tmp.n = count+1;

                        merge_result.append(tmp);
                        count++;
                    }

                }
            }
        }


}
    if (adjMatrix) {delete[] adjMatrix; adjMatrix = 0;}
    if (plist) {delete[] plist; plist=0;}


    return true;
}

bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename)
{
    FILE * fp;
    fp = fopen(filename,"w");
    if (fp==NULL)
    {
        fprintf(stderr,"ERROR: %s: failed to open file to write!\n",filename);
        return false;
    }
    fprintf(fp,"##n,type,x,y,z,radius,parent,seg_id, level, edge_vote\n");   //,vote\n");
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d 0 0 %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn, curr.type);//,curr.fea_val[0]);
    }
    fclose(fp);
    return true;
}
