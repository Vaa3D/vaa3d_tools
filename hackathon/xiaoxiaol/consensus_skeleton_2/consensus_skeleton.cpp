//consensus_skeleton.cpp
//generate consensus tree from multiple neurons

//(1) use a volume image to recode and accumulate the locations of nodes for all trees
//(2) keep one node for each small window, record its confidence value as a feature value (eswc) by the votes
//(3) final result is a minimum spanning tree from 2)


#include "consensus_skeleton.h"
//#include "kcentroid_cluster.h"
#include "mst_dij.h"
#include <QtGlobal>
#include <math.h>
#include <iostream>
#include <climits>
#include "basic_4dimage.h"

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


MyBoundingBox neuron_trees_bb(const vector<NeuronTree>  nt_list)
{
    MyBoundingBox bb ={0,0,0,0,0,0};

    bb.min_x = LONG_MAX;
    bb.min_y = LONG_MAX;
    bb.min_z = LONG_MAX;
    bb.max_x = 0;
    bb.max_y = 0;
    bb.max_z = 0;
    for (int j =0; j < nt_list.size(); j++){
        NeuronTree nt = nt_list[j];
        for (int i =0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC a = nt.listNeuron.at(i);
            bb.min_x = MIN(a.x,bb.min_x);
            bb.max_x = MAX(a.x,bb.max_x);
            bb.min_y = MIN(a.y,bb.min_y);
            bb.max_y = MAX(a.y,bb.max_y);
            bb.min_z = MIN(a.z,bb.min_z);
            bb.max_z = MAX(a.z,bb.max_z);
        }
    }
    return bb;
}

void non_max_suppresion( unsigned char * img1d, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z, Point3D offset,
                         vector<Point3D> &node_list,  vector<unsigned char> &vote_list,unsigned int win_size)
{
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



bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & merge_result, int method_code,V3DPluginCallback2 &callback)
{
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

    //count
    for (int j =0; j < nt_list.size(); j++){
        NeuronTree nt = nt_list[j];
        for (int i =0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC node = nt.listNeuron.at(i);
            V3DLONG id_x = (node.x-offset.x) +0.5; //round up
            V3DLONG id_y = (node.y-offset.y) +0.5;
            V3DLONG id_z = (node.z-offset.z) +0.5;
            V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;
            if (idx <tol_sz ){
                img1d[idx] ++ ;}
            else{
                cout <<"error idx" <<endl;
            }
        }
    }

    //for debug only
    Image4DSimple *image = new Image4DSimple();
    image->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
    callback.saveImage(image, "./vote_count_image.v3draw");

    //non max suppresion
     vector<Point3D>  node_list;
     vector<unsigned char>  vote_list;
     non_max_suppresion (img1d,sz_x,sz_y,sz_z,offset,node_list,vote_list,3);
     cout << "number of nodes:"<<node_list.size()<<endl;

     image->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);

     // minmum spanning tree , carrying the condifence value
     // adjMatrix

    printf("(2). generating confidence graph.\n");

    QHash<V3DLONG, pair<V3DLONG,V3DLONG> > nodeMap;//map  original nodes (i,j)   to consensus nodes  (node_id)
    for (int i=0;i<neuronNum;i++)
        for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
        {
            NeuronSWC s = nt_list[i].listNeuron.at(j);
            Point3D cur;
            cur.x = s.x;
            cur.y = s.y;
            cur.z = s.z;

            //find its nearest node
            V3DLONG node_id = 0;
            double min_dis = 0;
            for (V3DLONG ni = 0; ni<node_list.size(); ni++)
            {
                Point3D p = node_list[ni];
                double dis = PointDistance(p,cur);
                if (dis < min_dis){
                     min_dis = dis;
                     node_id = ni;
                }
            }

            pair<V3DLONG,V3DLONG> idx(i,j);
            nodeMap.insert(node_id, idx);
        }

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

    for (int i=0;i<neuronNum;i++)
    {
        for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
        {
            NeuronSWC cur = nt_list[i].listNeuron[j];
            if (cur.pn<0) continue;
            V3DLONG col,row;
            col = nodeMap.key(pair<V3DLONG,V3DLONG>(i,j));
            V3DLONG pid=nt_list[i].hashNeuron.value(cur.pn);
            row = nodeMap.key(pair<V3DLONG,V3DLONG>(i,pid));
            adjMatrix[col*num_nodes+row]++;
            adjMatrix[row*num_nodes+col]++;
        }
    }

    long rootid = 100;
    printf("(3). computing minimum-spanning tree.\n");
    if (method_code==0)
    {
        if (!mst_dij(adjMatrix, num_nodes, plist, rootid))
        {
            fprintf(stderr,"Error in minimum spanning tree!\n");
            return false;
        }
    }
//    else if (method_code==1)
//        mst_prim(adjMatrix, num_nodes,plist,0);

    merge_result.clear();
    for (V3DLONG i=0;i<num_nodes;i++)
    {
        NeuronSWC tmp;
        tmp.x = node_list[i].x;
        tmp.y = node_list[i].y;
        tmp.z = node_list[i].z;
        tmp.type = 2; //dendrite
        tmp.fea_val.push_back(vote_list[i]);
        tmp.r = 0.5; //only skeleton
        tmp.n = i+1;
        tmp.pn = plist[i]+1;
        if (tmp.pn<=0) tmp.pn = -1;
        merge_result.append(tmp);
    }

    if (adjMatrix) {delete[] adjMatrix; adjMatrix=0;}
    if (plist) {delete[] plist; plist=0;}

/*    if ( num_nodes <=0)
    {
        //initial cluster number is the median size of all input trees
        vector <V3DLONG> num_nodes_list;
        {
            for (int i=0;i<neuronNum;i++)
            {
                num_nodes_list.push_back( nt_list[i].listNeuron.size());
            }
        }
        num_nodes = median(num_nodes_list);
        cout << "number of nodes (in the output consensus skeleton) is :" << num_nodes <<" (the median of the population)" <<endl;
    }
*/
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
    fprintf(fp,"##n,type,x,y,z,radius,parent,vote\n");
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn,curr.fea_val[0]);
    }
    fclose(fp);
    return true;
}
