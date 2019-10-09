#ifndef REVIEW_H
#define REVIEW_H
#include<v3d_interface.h>
#include<vector>
#include<math.h>
using namespace std;
#define Block_Size 10000
#define IN 100000000000

struct select_point{
    V3DLONG n;
    bool select;
    select_point():n(0),select(false){}
    select_point(V3DLONG n,bool select){
        this->n=n;
        this->select=select;
    }
};

struct Point_xyz
{
    int x,y,z;
    Point_xyz(int x,int y,int z){
        this->x=x;
        this->y=y;
        this->z=z;
    }

};

struct segment{
    NeuronSWC start;
    NeuronSWC end;
    segment* parent_seg;
    vector<segment*> child_seg;
    vector<NeuronSWC> points;
    double avg_intensity;
    int seq_index;
    int segs_index;
    segment(){
        avg_intensity=0;
        seq_index=0;
        segs_index=0;
        parent_seg=0;
    }
    ~segment(){

    }
    bool operator ==(const segment &seg)const{//重载运算符==，segment比较的函数
        if(start==seg.start&&end==seg.end){
            return true;
        }
    }
};

struct SWCTreeSeg{
    vector<segment> segments;
    NeuronTree nt;
    NeuronSWC root;
    vector<NeuronSWC> candidate_point;
    //vector<NeuronSWC> subNeuron;
    vector<vector<V3DLONG> > children;
    V3DLONG block_sz0, block_sz1, block_sz2;
    unsigned char *p1data;
    bool initialize(NeuronTree t);//将一个重建好的swc分成若干段，人工重建过程中可能存在断点的地方，先只按分叉点来分段
    bool block_seg(NeuronTree orig,vector<NeuronSWC> &candidate_point,vector<V3DLONG> &subNeuron,size_t x0,size_t y0,size_t z0,int resolution);
    bool seg_intensity(V3DLONG block_sz0,V3DLONG block_sz1,V3DLONG block_sz2,unsigned char* &p1data);
//    SWCTreeSeg(){
//        block_sz1=block_sz2=block_sz0=0;
//        p1data=0;
//    }
};


bool write_swc(NeuronTree orig,NeuronTree &nt,QList<CellAPO> &markers,segment seg,V3DLONG &index);
bool cut_block(QString input_path,V3DPluginCallback2 &callback,NeuronTree &nt,V3DLONG block_sz0,V3DLONG block_sz1,V3DLONG block_sz2,size_t &x0,size_t &y0,size_t &z0, NeuronSWC center_point,vector<V3DLONG> &subNeuron,int resolution,unsigned char* &p1data);//以center_point为中心切块
bool move_block(QString braindir,V3DPluginCallback2 &callback,NeuronTree orig,SWCTreeSeg &segs ,V3DLONG block_sz0,V3DLONG block_sz1,V3DLONG block_sz2,vector<NeuronSWC> &candidate_point);
bool sequence_rule(SWCTreeSeg &segs,NeuronTree orig_tree,vector<vector<V3DLONG> > children);

#endif // REVIEW_H

