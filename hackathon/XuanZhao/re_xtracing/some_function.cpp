#include "some_function.h"

#define NODE_TO_XYZ(j,x,y,z) {x=(j)%(sz0);y=((j)/(sz0))%sz1;z=((j)/(sz01))%sz2;}

bool trans_segs(long *plist, vector<unit_seg> &unit_segs, int sz0, int sz1, int sz2)
{
    long tol_sz=sz0*sz1*sz2;
    long sz01=sz0*sz1;

    /*for(long i=0;i<tol_sz;++i)
    {
        if(plist[i]>=tol_sz)
            cout<<"yuejie: "<<plist[i]<<endl;
    }*/



    //bool* state=0;
    cout<<"in trans_segs"<<endl;

    vector<vector<long>> children;
    children=vector<vector<long>>(tol_sz,vector<long>());
    for(long i=0;i<tol_sz;++i)
    {
        long par=plist[i];
        if(par<0||par==i) continue;
        children[par].push_back(i);
    }

    cout<<"chirlden fin"<<endl;



    double seg_id=0;

    for(long i=0;i<tol_sz;++i)
    {
        long par=plist[i];
        if(par==-1)
        {
            unit_seg seg;
            seg.seg_id=seg_id;
            unit_p ori_p;
            vector<unit_p> p_queue;
            trans_coord(i,ori_p,sz0,sz1,sz2);
            ori_p.n=i;
            ori_p.nchild=children[i].size();
            ori_p.parent=-1;
            ori_p.segid=seg_id;
            p_queue.push_back(ori_p);
            while(!p_queue.empty())
            {
                unit_p tmp_p=p_queue.front();
                p_queue.erase(p_queue.begin());
                seg.seg_tree.push_back(tmp_p);
                if(children[tmp_p.n].size()==0)
                    continue;
                for(int i=0;i<children[tmp_p.n].size();++i)
                {
                    unit_p tmp_pc;
                    trans_coord(children[tmp_p.n][i],tmp_pc,sz0,sz1,sz2);
                    tmp_pc.n=children[tmp_p.n][i];
                    tmp_pc.nchild=children[children[tmp_p.n][i]].size();
                    tmp_pc.parent=tmp_p.n;
                    tmp_pc.segid=tmp_p.segid;
                    p_queue.push_back(tmp_pc);
                }
            }
            unit_segs.push_back(seg);
            seg.seg_tree.clear();
            seg_id++;
        }
    }

    cout<<"fin"<<endl;

    for(long i=0;i<children.size();++i)
    {
        children[i].clear();
    }
    children.clear();
    return true;

}


bool re_xtrace(V3DPluginCallback2 &callback, PARA_T &p)
{
    unsigned char* indata1d=p.p4dImage->getRawDataAtChannel(0);
    V3DLONG in_sz[4]={p.xc1,p.yc1,p.zc1,1};
    int datatype=p.p4dImage->getDatatype();
    double thres=p.visible_thres;
    float* phi=0;
    long* plist=0;
    int cnn_type=3;
    if(p.is_gsdt)
    {
        cout<<"in is_gsdt"<<endl;
        fastmarching_dt(indata1d,phi,in_sz[0],in_sz[1],in_sz[2],cnn_type,thres);

        cout<<"end fastmarching_dt"<<endl;

        fm_find_segs(phi,plist,in_sz[0],in_sz[1],in_sz[2],cnn_type,thres);

        cout<<"end fm_find_segs"<<endl;
    }
    else
    {
        cout<<"in not"<<endl;
        fm_find_segs(indata1d,plist,in_sz[0],in_sz[1],in_sz[2],cnn_type,thres);
    }
    vector<unit_seg> segs;

    trans_segs(plist,segs,in_sz[0],in_sz[1],in_sz[2]);
    cout<<"end trans_segs"<<endl;
    calculate_segs_radius(indata1d,in_sz,segs,thres);
    cout<<"end calculate_segs_radius"<<endl;
    NeuronTree nt;
    for(long i=0;i<segs.size();++i)
    {
        for(long j=0;j<segs[i].seg_tree.size();++j)
        {
            NeuronSWC tmp;
            tmp.n=segs[i].seg_tree[j].n;
            tmp.x=segs[i].seg_tree[j].x;
            tmp.y=segs[i].seg_tree[j].y;
            tmp.z=segs[i].seg_tree[j].z;
            tmp.parent=segs[i].seg_tree[j].parent;
            tmp.r=segs[i].seg_tree[j].r;
            nt.listNeuron.push_back(tmp);
        }
    }
    writeESWC_file("D://test.eswc",nt);
    for(long i=0;i<segs.size();++i)
    {
        for(long j=0;j<segs[i].seg_tree.size();++j)
        {
            segs[i].seg_tree.clear();
        }
    }
    segs.clear();
    nt.listNeuron.clear();
    return true;
}




