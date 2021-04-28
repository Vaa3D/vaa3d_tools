#include "boutonDetection_fun.h"
#include <QHash>
#include "volimg_proc.h"
#include <fstream>

void getBoutonInTerafly(V3DPluginCallback2 &callback, string imgPath, NeuronTree& nt,int allnode,int ne_area)
{
    cout<<"Welcome into bouton detection (Terafly Version): intensity part"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].level=1;
        hashNeuron.insert(listNeuron[i].n,i);
    }
    //only keep axon part
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(allnode>0)
        {
            if(s.level==1)
            {
                //get a block
                long start_x,start_y,start_z,end_x,end_y,end_z;
                int block_size=16;
                start_x = s.x - block_size; if(start_x<0) start_x = 0;
                end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
                start_y =s.y - block_size;if(start_y<0) start_y = 0;
                end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
                start_z = s.z - block_size;if(start_z<0) start_z = 0;
                end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

                V3DLONG *in_sz = new V3DLONG[4];
                in_sz[0] = end_x - start_x;
                in_sz[1] = end_y - start_y;
                in_sz[2] = end_z - start_z;
                V3DLONG *sz;
                sz=in_sz;
                long sz0 = sz[0];
                long sz01 = sz[0] * sz[1];
                unsigned char * inimg1d = 0;
                inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
    //            //version one
                V3DLONG thisx,thisy,thisz;
                thisx=s.x-start_x;
                thisy=s.y-start_y;
                thisz=s.z-start_z;

                listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                //refine the node to local maximal(surrounding area)
                NeuronSWC out=nodeRefine(inimg1d,thisx,thisy,thisz,in_sz,ne_area);
                double imgave,imgstd;
                V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
                mean_and_std(inimg1d,total_size,imgave,imgstd);
                if(listNeuron[i].level+imgstd<out.level)
                {
                    listNeuron[i].level=out.level;
                    listNeuron[i].x=start_x+out.x;
                    listNeuron[i].y=start_y+out.y;
                    listNeuron[i].z=start_z+out.z;
                }

                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }
        else
        {
            if(s.type==2 && s.level==1)
            {
                //get a block
                int start_x,start_y,start_z,end_x,end_y,end_z;
                int block_size=16;
                start_x = s.x - block_size; if(start_x<0) start_x = 0;
                end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
                start_y =s.y - block_size;if(start_y<0) start_y = 0;
                end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
                start_z = s.z - block_size;if(start_z<0) start_z = 0;
                end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

                V3DLONG *in_sz = new V3DLONG[4];
                in_sz[0] = end_x - start_x;
                in_sz[1] = end_y - start_y;
                in_sz[2] = end_z - start_z;
                V3DLONG *sz;
                sz=in_sz;
                long sz0 = sz[0];
                long sz01 = sz[0] * sz[1];
                unsigned char * inimg1d = 0;
                inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
    //            //version one
                V3DLONG thisx,thisy,thisz;
                thisx=s.x-start_x;
                thisy=s.y-start_y;
                thisz=s.z-start_z;

                listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
//                cout<<"level:"<<listNeuron[i].level<<endl;

                //refine the node to local maximal(surrounding area)
                NeuronSWC out=nodeRefine(inimg1d,thisx,thisy,thisz,in_sz,ne_area);
                double imgave,imgstd;
                V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
                mean_and_std(inimg1d,total_size,imgave,imgstd);
                if(listNeuron[i].level+imgstd<out.level)
                {
                    listNeuron[i].level=out.level;
                    listNeuron[i].x=start_x+out.x;
                    listNeuron[i].y=start_y+out.y;
                    listNeuron[i].z=start_z+out.z;
                }
                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }

    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}

}
/*
 *renderingType=0, get intensity from level
 *renderingType=1, get intensity from type
*/
QList <CellAPO> getBouton(NeuronTree nt, int in_thre,int allnode)
{
    cout<<"Welcome into bouton detection: filter part"<<endl;
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash <V3DLONG, V3DLONG>  hashNeuron,hashchild;
    hashNeuron.clear();
    hashchild.clear();
    V3DLONG boutonnodelist[siz];
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        boutonnodelist[i]=0;
        hashNeuron.insert(s.n,i);
        hashchild.insert(i,0);
    }
    QHash<V3DLONG,int> gau_thre;gau_thre.clear();
    gau_thre=getIntensityStd(nt,in_thre);
    /*Two level threshold seperate the region into three intervals:
     *p1 maybe is a P-site or maybe not. if p1 is a P-site, it should be known by its child.
     *So consider the situation below, p1 is not a P-site, which means it has a low intensity:
     *p1,p2,p3,p4(p1->p2->p3->p4)
     *R1(~,Ip1),R2(Ip1,threshold),R3(threshold,~)
    */
    cout<<"-------Internal bouton detection---------------"<<endl;
    V3DLONG bouton_dist_thre=3;
    cout<<"--default bouton intensity threshold is more than "<<in_thre<< " --"<<endl;
    cout<<"------default bouton distance is more than "<<bouton_dist_thre<< " pixels-----"<<endl;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        int threshold=in_thre;
        if(allnode==0 && s.type!=2)
            continue;
        if(s.parent>0&&s.level>1)
        {
            if(!hashNeuron.contains(s.parent))
                continue;
            //get threshold for each node
            if(gau_thre.contains(i))
                threshold=gau_thre.value(i);
            long p2_id=hashNeuron.value(s.parent);
            hashchild[p2_id]=hashchild[p2_id]+1;
            long Ip1=s.level;
            NeuronSWC p2=listNeuron[p2_id];
            long Ip2=p2.level;
            long intensity_change=Ip2-Ip1;
            //1.1

            if(intensity_change>=threshold)
            {
                boutonnodelist[p2_id]=Ip2;
                //look at p3
                if(p2.parent<0||!hashNeuron.contains(p2.parent))
                    continue;
                long p3_id=hashNeuron.value(p2.parent);
                hashchild[p3_id]=hashchild[p3_id]+1;
                NeuronSWC p3=listNeuron[p3_id];
                long Ip3=p3.level;
                int p3change=Ip3-Ip1;
                V3DLONG sp_spp_dist=(p3.x-p2.x)*(p3.x-p2.x)+(p3.y-p2.y)*(p3.y-p2.y)+(p3.z-p2.z)*(p3.z-p2.z);
                if(sp_spp_dist>bouton_dist_thre*bouton_dist_thre&&p3change>=threshold)
                    boutonnodelist[p3_id]=Ip3;
            }
            else if(intensity_change>0&&intensity_change<threshold)
            {
                //2.1
                if(p2.parent<0||!hashNeuron.contains(p2.parent))
                    continue;
                long p3_id=hashNeuron.value(p2.parent);
                hashchild[p3_id]=hashchild[p3_id]+1;
                NeuronSWC p3=listNeuron[p3_id];
                long Ip3=p3.level;
                int p3change=Ip3-Ip1;
                V3DLONG sp_spp_dist=(p3.x-p2.x)*(p3.x-p2.x)+(p3.y-p2.y)*(p3.y-p2.y)+(p3.z-p2.z)*(p3.z-p2.z);
                if(sp_spp_dist<bouton_dist_thre*bouton_dist_thre&&p3change>=threshold)
                    boutonnodelist[p3_id]=Ip3;
            }
            else if(Ip1-Ip2>=threshold)
                boutonnodelist[hashNeuron.value(s.n)]=Ip1;
            else
                continue; //0
        }
        else
            hashchild[hashNeuron.value(s.n)]=hashchild[hashNeuron.value(s.n)]+1;
    }
    cout<<"-------Tip bouton detection---------------"<<endl;
    /*the parent node of tip node maybe a bouton*/
    QHash<V3DLONG, V3DLONG>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        if(hcit.value()==0)
        {
            V3DLONG childid=hcit.key();
            NeuronSWC s = listNeuron[childid];
            V3DLONG Ip1=s.level;
            if(!allnode&&s.type!=2)
                continue;
            if(s.parent&&hashNeuron.contains(s.parent))
            {
                /*the distance between child-node to parent-node should have a threshold.*/
                long pid=hashNeuron.value(s.parent);
                NeuronSWC s_pp=listNeuron[pid];
                V3DLONG cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
                while(cp_dist<bouton_dist_thre*bouton_dist_thre)
                {
                    if(Ip1<s_pp.level)
                    {
                        Ip1=s_pp.level;
                        childid=pid;
                    }
                    if(!hashNeuron.contains(s_pp.parent))
                        break;
                    pid=hashNeuron.value(s_pp.parent);
                    s_pp=listNeuron[pid];
                    cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
                }
                V3DLONG Ip2=s_pp.level;
                V3DLONG intensity_change=Ip1-Ip2;
                int threshold=in_thre;
                if(gau_thre.contains(childid))
                    threshold=gau_thre.value(childid);
                if(intensity_change>=threshold)
                {
                    boutonnodelist[childid]=Ip1;
                }
                if(Ip2-Ip1>=threshold)
                {
                    boutonnodelist[pid]=Ip2;
                }
            }
        }
    }

    //for all the P-sites nodes

    QList <CellAPO> apolist;apolist.clear();
    Bouton_Color_List boutonColorMap;
    V3DLONG boutonColorMapSize=boutonColorMap.listcolor.size();
    V3DLONG boutonCount=0;
    //if you want to render one swc file into one color
    Bouton_Color_Basic this_color;
    this_color=(siz<boutonColorMapSize)?boutonColorMap.listcolor[siz]:boutonColorMap.listcolor[V3DLONG(siz%boutonColorMapSize)];
    cout<<"-------Bouton purnning---------------"<<endl;
    for (V3DLONG i=0;i<siz;i++)
    {
        if(boutonnodelist[i]==0)
            continue;
        NeuronSWC s = listNeuron[i];
        /*the distance between child-bouton-node to parent-bouton-node should have a threshold.*/
        if(s.parent>0&&hashNeuron.contains(s.parent))
        {
            long pid=hashNeuron.value(s.parent);
            NeuronSWC s_pp=listNeuron[pid];
            V3DLONG cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
            while(cp_dist<bouton_dist_thre*bouton_dist_thre)
            {
                if(boutonnodelist[pid]>0)
                    boutonnodelist[pid]=0;
                if(s_pp.parent==0||!hashNeuron.contains(s_pp.parent))
                    break;
                pid=hashNeuron.value(s_pp.parent);
                s_pp=listNeuron[pid];
                cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
            }
        }
    }
    cout<<"-------Bouton out to Apo---------------"<<endl;
    for (V3DLONG i=0;i<siz;i++)
    {
        if(boutonnodelist[i]==0)
            continue;
        NeuronSWC s = listNeuron[i];
        //if you want to render different colors to one swc file, unshield it
//        Bouton_Color_Basic this_color;
//        this_color=(i<boutonColorMapSize)?boutonColorMap.listcolor[i]:boutonColorMap.listcolor[V3DLONG(i%boutonColorMapSize)];
        int threshold=in_thre;
        if(gau_thre.contains(i))
            threshold=gau_thre.value(i);
        CellAPO apo;
        apo.x=s.x+1;
        apo.y=s.y+1;
        apo.z=s.z+1;
        apo.intensity=threshold;
        apo.volsize=s.level;
        apo.color.r=this_color.color_ele_r;
        apo.color.g=this_color.color_ele_g;
        apo.color.b=this_color.color_ele_b;
        boutonCount++;
        apolist.push_back(apo);
    }
    cout<<"Detected Bouton: "<<boutonCount<<endl;
    return apolist;
}

/*
 *renderingType=0, get intensity from level
 *renderingType=1, get intensity from type
*/
NeuronTree getBouton_toSWC(NeuronTree nt, int in_thre, int allnode,float dis_thre)
{

    cout<<"Welcome into bouton detection: filter part"<<endl;
    cout<<"This version will output bouton to swc file"<<endl;
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash <V3DLONG, V3DLONG>  hashNeuron,hashchild;
    hashNeuron.clear();
    hashchild.clear();
    V3DLONG boutonnodelist[siz];
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        boutonnodelist[i]=0;
        hashNeuron.insert(s.n,i);
        hashchild.insert(i,0);
    }
    QHash<V3DLONG,int> gau_thre;gau_thre.clear();
    gau_thre=getIntensityStd(nt,64);
    /*Two level threshold seperate the region into three intervals:
     *p1 maybe is a P-site or maybe not. if p1 is a P-site, it should be known by its child.
     *So consider the situation below, p1 is not a P-site, which means it has a low intensity:
     *p1,p2,p3,p4(p1->p2->p3->p4)
     *R1(~,Ip1),R2(Ip1,threshold),R3(threshold,~)
    */
    cout<<"-------Internal bouton detection---------------"<<endl;
    V3DLONG bouton_dist_thre=5;
    cout<<"--default bouton intensity threshold is more than "<<in_thre<< " --"<<endl;
    cout<<"------default bouton distance is more than "<<bouton_dist_thre<< " pixels-----"<<endl;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        int threshold=in_thre;
        if(allnode==0 && s.type!=2)
            continue;
        if(s.parent>0&&s.level>1)
        {
            if(!hashNeuron.contains(s.parent))
                continue;
            //get threshold for each node
            if(gau_thre.contains(i))
                threshold=gau_thre.value(i);
            long p2_id=hashNeuron.value(s.parent);
            hashchild[p2_id]=hashchild[p2_id]+1;
            long Ip1=s.level;
            NeuronSWC p2=listNeuron[p2_id];
            long Ip2=p2.level;
            long intensity_change=Ip2-Ip1;
            //1.1

            if(intensity_change>=threshold)
            {
                boutonnodelist[p2_id]=Ip2;
                //look at p3
                if(p2.parent<0||!hashNeuron.contains(p2.parent))
                    continue;
                long p3_id=hashNeuron.value(p2.parent);
                hashchild[p3_id]=hashchild[p3_id]+1;
                NeuronSWC p3=listNeuron[p3_id];
                long Ip3=p3.level;
                int p3change=Ip3-Ip1;
                V3DLONG sp_spp_dist=(p3.x-p2.x)*(p3.x-p2.x)+(p3.y-p2.y)*(p3.y-p2.y)+(p3.z-p2.z)*(p3.z-p2.z);
                if(sp_spp_dist>bouton_dist_thre*bouton_dist_thre&&p3change>=threshold)
                    boutonnodelist[p3_id]=Ip3;
            }
            else if(intensity_change>0&&intensity_change<threshold)
            {
                //2.1
                if(p2.parent<0||!hashNeuron.contains(p2.parent))
                    continue;
                long p3_id=hashNeuron.value(p2.parent);
                hashchild[p3_id]=hashchild[p3_id]+1;
                NeuronSWC p3=listNeuron[p3_id];
                long Ip3=p3.level;
                int p3change=Ip3-Ip1;
                V3DLONG sp_spp_dist=(p3.x-p2.x)*(p3.x-p2.x)+(p3.y-p2.y)*(p3.y-p2.y)+(p3.z-p2.z)*(p3.z-p2.z);
                if(sp_spp_dist<bouton_dist_thre*bouton_dist_thre&&p3change>=threshold)
                    boutonnodelist[p3_id]=Ip3;
            }
            else if(Ip1-Ip2>=threshold)
                boutonnodelist[hashNeuron.value(s.n)]=Ip1;
            else
                continue; //0
        }
        else
            hashchild[hashNeuron.value(s.n)]=hashchild[hashNeuron.value(s.n)]+1;
    }
    cout<<"-------Tip bouton detection---------------"<<endl;
    /*the parent node of tip node maybe a bouton*/
    QHash<V3DLONG, V3DLONG>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        if(hcit.value()==0)
        {
            V3DLONG childid=hcit.key();
            NeuronSWC s = listNeuron[childid];
            V3DLONG Ip1=s.level;
            if(!allnode&&s.type!=2)
                continue;
            if(s.parent&&hashNeuron.contains(s.parent))
            {
                /*the distance between child-node to parent-node should have a threshold.*/
                long pid=hashNeuron.value(s.parent);
                NeuronSWC s_pp=listNeuron[pid];
                V3DLONG cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
                while(cp_dist<bouton_dist_thre*bouton_dist_thre)
                {
                    if(Ip1<s_pp.level)
                    {
                        Ip1=s_pp.level;
                        childid=pid;
                    }
                    if(!hashNeuron.contains(s_pp.parent))
                        break;
                    pid=hashNeuron.value(s_pp.parent);
                    s_pp=listNeuron[pid];
                    cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
                }
                V3DLONG Ip2=s_pp.level;
                V3DLONG intensity_change=Ip1-Ip2;
                int threshold=in_thre;
                if(gau_thre.contains(childid))
                    threshold=gau_thre.value(childid);
                if(intensity_change>=threshold)
                {
                    boutonnodelist[childid]=Ip1;
                }
                if(Ip2-Ip1>=threshold)
                {
                    boutonnodelist[pid]=Ip2;
                }
            }
        }
    }

    //for all the P-sites nodes

    QList <CellAPO> apolist;apolist.clear();
    Bouton_Color_List boutonColorMap;
    V3DLONG boutonColorMapSize=boutonColorMap.listcolor.size();
    //if you want to render one swc file into one color
    Bouton_Color_Basic this_color;
    this_color=(siz<boutonColorMapSize)?boutonColorMap.listcolor[siz]:boutonColorMap.listcolor[V3DLONG(siz%boutonColorMapSize)];
    cout<<"-------Bouton purnning---------------"<<endl;
    for (V3DLONG i=0;i<siz;i++)
    {
        if(boutonnodelist[i]==0)
            continue;
        NeuronSWC s = listNeuron[i];
        /*the distance between child-bouton-node to parent-bouton-node should have a threshold.*/
        if(s.parent>0&&hashNeuron.contains(s.parent))
        {
            long pid=hashNeuron.value(s.parent);
            NeuronSWC s_pp=listNeuron[pid];
            V3DLONG cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
            while(cp_dist<bouton_dist_thre*bouton_dist_thre)
            {
                if(boutonnodelist[pid]>0)
                    boutonnodelist[pid]=0;
                if(s_pp.parent==0||!hashNeuron.contains(s_pp.parent))
                    break;
                pid=hashNeuron.value(s_pp.parent);
                s_pp=listNeuron[pid];
                cp_dist=(s_pp.x-s.x)*(s_pp.x-s.x)+(s_pp.y-s.y)*(s_pp.y-s.y)+(s_pp.z-s.z)*(s_pp.z-s.z);
            }
        }
    }
    //for all the P-sites nodes
    cout<<"-------Bouton out to SWC---------------"<<endl;
    //if a node
    NeuronTree bouton_out,bouton_final;
    float MAX_Bouton_dist=200;
    for (V3DLONG i=0;i<siz;i++)
    {
        if(!boutonnodelist[i]) continue;
        NeuronSWC s = listNeuron[i];
        long pIndex=hashNeuron.value(s.parent);
        //update pn_id to a parent-bouton node
        V3DLONG isParentBouton=0;
        V3DLONG pid=s.parent;
        NeuronSWC childNode=s;
        s.parent=-1;
        float bouton_cp_dis=0.0;
        while(pid)
        {
            if(!hashNeuron.contains(pid))
            {
                cout<<"can't find parent node: "<<pid<<",set as -1"<<endl;
                break;
            }
            pIndex=hashNeuron.value(pid);
            NeuronSWC parentNode=listNeuron[pIndex];
            isParentBouton=boutonnodelist[pIndex];
            float disx,disy,disz;
            disx=abs(childNode.x-parentNode.x);
            disy=abs(childNode.y-parentNode.y);
            disz=abs(childNode.z-parentNode.z);
            bouton_cp_dis+=sqrt(disx*disx+disy*disy+disz*disz);
            if(bouton_cp_dis>MAX_Bouton_dist) break;
            if(isParentBouton)
            {
                //this is a bouton node
                s.parent=parentNode.n;
                break;
            }
            childNode=parentNode;
            pid=parentNode.parent;
        }
        if(bouton_cp_dis>MAX_Bouton_dist) s.parent==-1; s.radius=0;
        s.radius=(s.parent==-1||isParentBouton==0)?0:bouton_cp_dis;
        s.parent=(isParentBouton==0)?(-1):s.parent;
        bouton_out.listNeuron.append(s);
        bouton_out.hashNeuron.insert(s.n,bouton_out.listNeuron.size()-1);
    }
    // revise the false parent node into -1
    cout<<"Revise part"<<endl;
    for(V3DLONG i=0;i<bouton_out.listNeuron.size();i++)
    {
        NeuronSWC s = bouton_out.listNeuron[i];
        if(s.parent==-1)
            continue;
        if(!bouton_out.hashNeuron.contains(s.parent))
        {
            cout<<"can't find parent node: "<<s.parent<<",set as -1"<<endl;
            bouton_out.listNeuron[i].parent=-1;
        }
    }
    cout<<"Detected Bouton: V0="<<bouton_out.listNeuron.size()<<endl;
    cout<<"remove distance less than 1"<<endl;
    QHash<long,long> skipnodes;
    skipnodes.clear();
    for(V3DLONG i=0;i<bouton_out.listNeuron.size();i++)
    {
        NeuronSWC s = bouton_out.listNeuron[i];
        if(s.radius<=dis_thre)
        {
            long pIndex=bouton_out.hashNeuron.value(s.parent);
            NeuronSWC sp = bouton_out.listNeuron[pIndex];
            //set it's new parent
            skipnodes.insert(sp.n,skipnodes.size()-1);
            bouton_out.listNeuron[i].parent=sp.parent;
            bouton_out.listNeuron[i].radius=s.radius+sp.radius;
        }
    }
    cout<<"SKIP false bouton : "<<skipnodes.size()<<endl;
    for(V3DLONG i=0;i<bouton_out.listNeuron.size();i++)
    {
         NeuronSWC s = bouton_out.listNeuron[i];
         if(!skipnodes.contains(s.n))
         {
             bouton_final.listNeuron.append(s);
             bouton_final.hashNeuron.insert(s.n,bouton_final.listNeuron.size());
         }
    }
    cout<<"Detected Bouton: "<<bouton_final.listNeuron.size()<<endl;
    return bouton_final;
}
void getBoutonBlock_inImg(V3DPluginCallback2 &callback, unsigned char *& inimg1d,V3DLONG in_zz[],QList <CellAPO> apolist,string outpath,int block_size)
{
    /*crop 3d image-block
     * get mip img
    */
    if(!apolist.size()) {return;}
    QString save_path = QString::fromStdString(outpath);
    QDir path(save_path);
    if(!path.exists())
        path.mkpath(save_path);
    for(V3DLONG i=0;i<apolist.size();i++)
    {
        CellAPO s = apolist[i];
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - block_size; if(start_x<0) {start_x = 0;}
        end_x = s.x + block_size; if(end_x >= in_zz[0]) {end_x = in_zz[0]-1;}
        start_y =s.y - block_size;if(start_y<0) {start_y = 0;}
        end_y = s.y + block_size;if(end_y >= in_zz[1]) {end_y = in_zz[1]-1;}
        start_z = s.z - block_size;if(start_z<0) {start_z = 0;}
        end_z = s.z + block_size;if(end_z >= in_zz[2]) {end_z = in_zz[2]-1;}

//        V3DLONG *in_sz = new V3DLONG[4];
        V3DLONG in_sz [4];
        in_sz[0] =end_x-start_x+1;
        in_sz[1] = end_y-start_y+1;
        in_sz[2] = end_z-start_z+1;
        in_sz[3]=in_zz[3];
        long sz01 = in_sz[0] * in_sz[1];
        long sz0 = in_sz[0];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz= in_sz[0] * in_sz[1]*in_sz[2]*in_sz[3];
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
        for(V3DLONG iz=0;iz<in_sz[2];iz++)
        {
            for(V3DLONG iy=0;iy<in_sz[1];iy++)
            {
                for(V3DLONG ix=0;ix<in_sz[0];ix++)
                {
                    im_cropped[iz * sz01 + iy * sz0 + ix]=
                            inimg1d[(start_z+iz) * in_zz[0]*in_zz[1]
                            +(start_y+ iy) *in_zz[0] + (start_x+ix)];
                }
            }
        }
        QString tmpstr = "";
        tmpstr.append("_n_").append(QString("%1").arg(s.n));
        tmpstr.append("_x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "Bouton"+tmpstr+".tif";
//        QString default_name_apo="Bouton"+tmpstr+".apo";
        QString save_path_img =save_path+"/"+default_name;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
}
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int block_size,uint mip_flag)
{
    cout<<"Welcome into bouton detection: crop part"<<endl;
    cout<<"get bouton apo file and crop img out"<<endl;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}
    QString save_path_3d = QString::fromStdString(outpath)+"/3d";
    QDir path(save_path_3d);    if(!path.exists()) { path.mkpath(save_path_3d);}
    QString save_path_mip="";
    if(mip_flag){
         save_path_mip= QString::fromStdString(outpath)+"/mip";
        QDir path_mip(save_path_mip);    if(!path_mip.exists()) { path_mip.mkpath(save_path_mip);}
    }

    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - block_size; if(start_x<0) start_x = 0;
        end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - block_size;if(start_y<0) start_y = 0;
        end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - block_size;if(start_z<0) start_z = 0;
        end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] =end_x-start_x+1;
        in_sz[1] = end_y-start_y+1;
        in_sz[2] = end_z-start_z+1;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz =in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
        if(im_cropped==NULL){ cout<<"Crop fail"<<endl;continue; }
        //3d image block
        QString tmpstr = "";
        tmpstr.append("_n_").append(QString("%1").arg(s.n));
        tmpstr.append("_x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "Bouton"+tmpstr+".tif";
        QString save_path_img =save_path_3d+"/"+default_name;
//        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
        //2d mip
        if(mip_flag)
        {
            QString mip_save_path_img =save_path_mip+"/"+"Bouton"+tmpstr+".tif";
            getBoutonMIP(callback,im_cropped,in_sz, mip_save_path_img);
        }
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
}
void getBoutonMIP(V3DPluginCallback2 &callback, unsigned char *& inimg1d, V3DLONG in_sz[], QString outpath)
{
    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG mip_sz[4];
    mip_sz[0] = N;
    mip_sz[1] = M;
    mip_sz[2] = 1;
    mip_sz[3] = 1;

    V3DLONG pagesz_mip = mip_sz[0]*mip_sz[1]*mip_sz[2];
    unsigned char *image_mip=0;
    try {image_mip = new unsigned char [pagesz_mip];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return ;}

    for(V3DLONG ix = 0; ix < N; ix++)
    {
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                if(inimg1d[N*M*iz + N*iy + ix] >= max_mip)
                {
                    image_mip[iy*N + ix] = inimg1d[N*M*iz + N*iy + ix];
                    max_mip = inimg1d[N*M*iz + N*iy + ix];
                }
            }
        }
    }
    simple_saveimage_wrapper(callback,outpath.toStdString().c_str(), (unsigned char *)image_mip, mip_sz, 1);
    if(image_mip) {delete []image_mip; image_mip = 0;}
}

NeuronTree removeDupNodes(NeuronTree nt,V3DLONG removed_dist_thre)
{
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = listNeuron.size();
    NeuronTree removed_out; removed_out.listNeuron.clear();removed_out.hashNeuron.clear();
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        hashNeuron.insert(s.n,i);
    }
    for(V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        if(s.parent&&hashNeuron.contains(s.parent))
        {
            NeuronSWC sp=listNeuron[hashNeuron.value(s.parent)];
            V3DLONG cp_dist=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
            if(cp_dist<=removed_dist_thre)
            {
                //remove this parent node, and set the grandparent node as parent node.

                if(sp.parent&&hashNeuron.contains(sp.parent))
                {
                    listNeuron[i].parent=sp.parent;
                    listNeuron[hashNeuron.value(s.parent)].n=-1;
                    listNeuron[hashNeuron.value(s.parent)].parent=-1;

                }
            }
        }
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];

        if(s.n<0)
        {
//            cout<<i<<endl;
            continue;
        }
        removed_out.listNeuron.append(s);
        removed_out.hashNeuron.insert(s.n,removed_out.listNeuron.size()-1);
    }
    cout<<"Original swc size: "<<siz<<endl;
    cout<<"Removed swc size: "<<removed_out.listNeuron.size()<<endl;
    return removed_out;
}
QList <CellAPO> rmNearMarkers(QList <CellAPO> inapo,V3DLONG removed_dist_thre)
{
    QList <CellAPO> outapo;outapo.clear();
    if(!inapo.size())
        return outapo;
    V3DLONG aposize=inapo.size();
    for (V3DLONG i=0;i<aposize;i++)
    {
        CellAPO apo1=inapo.at(i);
        bool isok=true;
        for (V3DLONG j=0;j<aposize;j++)
        {
            if(i==j) continue;
            CellAPO apo2=inapo.at(j);
            V3DLONG b_dist=(apo1.x-apo2.x)*(apo1.x-apo2.x)
                    +(apo1.y-apo2.y)*(apo1.y-apo2.y)
                    +(apo1.z-apo2.z)*(apo1.z-apo2.z);
            if(b_dist<removed_dist_thre*removed_dist_thre)
            {
                if(apo1.volsize>apo2.volsize)
                    continue;
                isok=false;
                break;
            }
        }
        if(isok) {outapo.append(apo1);}
    }
    cout<<"Original apo size: "<<aposize<<endl;
    cout<<"Removed apo size: "<<outapo.size()<<endl;
    return outapo;
}
/*Given a NeuronTree and a swc node in this NeuronTree, return std intensity of surrounding area (thre_size)*/
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size)
{
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash<V3DLONG,int> id_threshold,id_threshold_out;//key is line id and value is std
    id_threshold.clear();id_threshold_out.clear();
    QList<long> surIntensity;
    if(nt.listNeuron.size()==0)
    {
        cout<<"Can't read neuronTree and input error"<<endl;
        return id_threshold;
    }
    long std_mean_thre=0;
    long mean_mean_thre=0;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC thisnode=listNeuron[i];
        //get a surrounding block nodelist
        long blockIntensityAll=thisnode.level;
        V3DLONG blockNodeNum=0;
        surIntensity.clear();
        for (V3DLONG j=0;j<siz;j++)
        {
            NeuronSWC surnode=listNeuron[j];
            if(abs(surnode.x-thisnode.x)<thre_size&&
                    abs(surnode.y-thisnode.y)<thre_size&&
                    abs(surnode.z-thisnode.z)<thre_size)
            {
                blockIntensityAll+=surnode.level;
                surIntensity.push_back(surnode.level);
                blockNodeNum++;
            }
        }
        if(!blockNodeNum)
            return id_threshold;
        int blockIntensityAvearage=blockIntensityAll/blockNodeNum;

        //standard
        long varIntensity=0;
        for (V3DLONG j=0;j<blockNodeNum;j++)
        {
            varIntensity=varIntensity+(surIntensity.at(j)-blockIntensityAvearage)*(surIntensity.at(j)-blockIntensityAvearage);
        }
        varIntensity/=blockNodeNum;
        int stdIntensity=sqrt(varIntensity);
        if(stdIntensity<0)
        {
            cout<<"varIntensity="<<varIntensity<<endl;
            cout<<"Avearage="<<blockIntensityAvearage<<";index="<<i<<endl;
            cout<<"blockNode Num="<<blockNodeNum<<endl;
            return id_threshold;
        }
        stdIntensity=(stdIntensity>100)?100:stdIntensity;
        stdIntensity=(stdIntensity<10)?10:stdIntensity;
        if(blockIntensityAvearage>120)
            id_threshold.insert(i,blockIntensityAvearage);
        else
            id_threshold.insert(i,stdIntensity);
        std_mean_thre+=stdIntensity;
        mean_mean_thre+=blockIntensityAvearage;
    }
//    std_mean_thre/=siz; mean_mean_thre/=siz;
//    cout<<"std mean: "<<(std_mean_thre)<<endl;
//    cout<<"mean mean: "<<mean_mean_thre<<endl;
//    if(mean_mean_thre>120)
//        for (V3DLONG i=0;i<id_threshold.size();i++)
//            id_threshold_out.insert(i,mean_mean_thre);
//    if(id_threshold_out.size())
//        return id_threshold_out;
    return id_threshold;
}
void getNodeRadius(unsigned char *&inimg1d, long in_sz[], NeuronTree& nt)
{
    cout<<"Get node radius"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=0;
    }

    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    double max_r = in_sz[0]/2;
    if (max_r > in_sz[1]/2) max_r = in_sz[1]/2;
    if (max_r > (in_sz[2])/2) max_r = (in_sz[2])/2;

    //get the background threshold
    double imgave,imgstd;
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
    mean_and_std(inimg1d,total_size,imgave,imgstd);
//    double td= (imgstd<10) ? 10:imgstd;
    double bkg_thresh= imgave+imgstd;
    cout<<"bkg thresh="<<bkg_thresh<<endl;

    for(V3DLONG it=0;it<siz;it++)
    {
        NeuronSWC s = listNeuron[it];
        int thisx,thisy,thisz;
        thisx=s.x;
        thisy=s.y;
        thisz=s.z;
        if(s.radius==0)
        {
            double total_num, background_num;
            double ir;
            for (ir=1; ir<=max_r; ir++)
            {
                total_num = background_num = 0;
                double dz, dy, dx;
                double zlower = -ir, zupper = +ir;
                for (dz= zlower; dz <= zupper; ++dz)
                    for (dy= -ir; dy <= +ir; ++dy)
                        for (dx= -ir; dx <= +ir; ++dx)
                        {
                            total_num++;
                            double r = sqrt(dx*dx + dy*dy + dz*dz);
                            if (r>ir-1 && r<=ir)
                            {
                                V3DLONG i = thisx+dx;   if (i<0 || i>=in_sz[0]) goto end2;
                                V3DLONG j = thisy+dy;   if (j<0 || j>=in_sz[1]) goto end2;
                                V3DLONG k = thisz+dz;   if (k<0 || k>=in_sz[2]) goto end2;

                                if (inimg1d[k * sz01 + j * sz0 + i] <= bkg_thresh)
                                {
                                    background_num++;

                                    if ((background_num/total_num) > 0.001) goto end2;
                                }
                            }
                        }
            }
end2:
            listNeuron[it].radius= ir;
        }
    }
    //release pointer
//     if(inimg1d) {delete []inimg1d; inimg1d=0;}
}
NeuronTree linearInterpolation(NeuronTree nt,int Min_Interpolation_Pixels)
{
    cout<<"linear interpolation of neuron tree"<<endl;
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    //step1: sort the index
    QHash <V3DLONG, V3DLONG>  index_n; index_n.clear();
    for(V3DLONG i=0;i<siz;i++)
        index_n.insert(listNeuron[i].n,i);
    NeuronTree nt_Index_sorted;nt_Index_sorted.listNeuron.clear();nt_Index_sorted.hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        s.n=i+1;
        if(s.parent&&index_n.contains(s.parent))
            s.parent=index_n.value(s.parent)+1;
        else
            s.parent=-1;
        nt_Index_sorted.listNeuron.append(s);
        nt_Index_sorted.hashNeuron.insert(s.n,nt_Index_sorted.listNeuron.size()-1);
    }
    listNeuron.clear();
    cout<<"finished the sorting of index"<<endl;
    listNeuron=nt_Index_sorted.listNeuron;
    QHash <int, int>  hashNeuron;hashNeuron.clear();
    hashNeuron=nt_Index_sorted.hashNeuron;

    //step2
    QList <NeuronSWC> nt_out_listNeuron;nt_out_listNeuron.clear();
    V3DLONG new_node_count=0;
    for (V3DLONG i=0;i<listNeuron.size();i++)
    {
        NeuronSWC s = listNeuron[i];
        if(s.parent&&hashNeuron.contains(s.parent))
        {
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=listNeuron[pid];
            double cp_dist=sqrt(long((sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z)));
            double Min_Interpolation_Pixels_dist=sqrt(long(Min_Interpolation_Pixels*Min_Interpolation_Pixels*Min_Interpolation_Pixels));
            int interpolate_times=int(cp_dist/Min_Interpolation_Pixels_dist);

            if(interpolate_times==1)
            {
                NeuronSWC s_interpolated=s;
                s_interpolated.n=siz+1+new_node_count;
                //one node at the center of p and sp
                s_interpolated.x=(sp.x+s.x)/2;
                s_interpolated.y=(sp.y+s.y)/2;
                s_interpolated.z=(sp.z+s.z)/2;
                s.parent=s_interpolated.n;
                new_node_count++;
                nt_out_listNeuron.append(s);
                nt_out_listNeuron.append(s_interpolated);
            }
            else if(interpolate_times>1)
            {
                //interpolate list of nodes
                double x_Interpolation_dis=(sp.x-s.x)/interpolate_times;
                double y_Interpolation_dis=(sp.y-s.y)/interpolate_times;
                double z_Interpolation_dis=(sp.z-s.z)/interpolate_times;

                NeuronSWC s_interpolated_start=s;
                for(int ti=1;ti<=interpolate_times;ti++)
                {
                    NeuronSWC s_interpolated=s_interpolated_start;
                    s_interpolated.n=siz+1+new_node_count;
                    s_interpolated.x=s_interpolated_start.x+x_Interpolation_dis;
                    s_interpolated.y=s_interpolated_start.y+y_Interpolation_dis;
                    s_interpolated.z=s_interpolated_start.z+z_Interpolation_dis;
                    if(ti==interpolate_times)
                        s_interpolated_start.parent=s_interpolated.parent;
                    else
                        s_interpolated_start.parent=s_interpolated.n;
                    nt_out_listNeuron.append(s_interpolated_start);
                    s_interpolated_start=s_interpolated;
                    new_node_count++;
                }
            }
            else
                nt_out_listNeuron.append(s);
        }
        else
            nt_out_listNeuron.append(s);
    }
    cout<<"finished the interpolation"<<endl;
    cout<<"from size: "<<siz<<" to "<<nt_out_listNeuron.size()<<endl;
    //step3: re_sort index of nt_out
    index_n.clear();
    NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
    for(V3DLONG i=0;i<nt_out_listNeuron.size();i++)
    {
        NeuronSWC s = nt_out_listNeuron[i];
        index_n.insert(s.n,i);
    }
    for(V3DLONG i=0;i<nt_out_listNeuron.size();i++)
    {
        NeuronSWC s = nt_out_listNeuron[i];
        s.n=i+1;        
        if(s.parent&&index_n.contains(s.parent))
            s.parent=index_n.value(s.parent)+1;
        else
            s.parent=-1;
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    return nt_out;
}
void getSWCIntensityInTeraflyImg(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int useNeighborArea)
{
    cout<<"Terafly dataset: get intensity of each node in a neuron tree"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = listNeuron.size();
    for (V3DLONG i=0;i<siz;i++)
         listNeuron[i].level=1;
    V3DLONG *in_zz = 0;    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        //get a block
        long start_x,start_y,start_z,end_x,end_y,end_z;
        int block_size=8;
        start_x = s.x - block_size; if(start_x<0) start_x = 0;
        end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - block_size;if(start_y<0) start_y = 0;
        end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - block_size;if(start_z<0) start_z = 0;
        end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = end_x - start_x+1;
        in_sz[1] = end_y - start_y+1;
        in_sz[2] = end_z - start_z+1;
        in_sz[3]=in_zz[3];
        V3DLONG sz01 = in_sz[0] * in_sz[1];
        V3DLONG sz0 = in_sz[0];
        unsigned char * inimg1d = 0;
        V3DLONG pagesz= in_sz[0] * in_sz[1]*in_sz[2]*in_sz[3];
        try {inimg1d = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
        inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
        if(inimg1d==NULL){ cout<<"Crop fail"<<endl;continue; }

        V3DLONG thisx,thisy,thisz;
        thisx=s.x-start_x;
        thisy=s.y-start_y;
        thisz=s.z-start_z;
        listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
        //refine the node to local maximal(surrounding area)
        double imgave,imgstd;
        mean_and_std(inimg1d,pagesz,imgave,imgstd);
        if(useNeighborArea!=0)
        {
            NeuronSWC out=nodeRefine(inimg1d,thisx,thisy,thisz,in_sz,useNeighborArea);
            if(listNeuron[i].level+imgstd<out.level)
            {
                listNeuron[i].level=out.level;
                listNeuron[i].x=start_x+out.x;
                listNeuron[i].y=start_y+out.y;
                listNeuron[i].z=start_z+out.z;
            }
        }
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}
}
void getSWCIntensityInImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt,int useNeighborArea)
{
    cout<<"Get intensity of each node in a neuron tree"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].level=1;
        hashNeuron.insert(listNeuron[i].n,i);
    }

    cout<<"Img size,x="<<in_sz[0]<<",y="<<in_sz[1]<<",z="<<in_sz[2]<<endl;
    V3DLONG sz01 = in_sz[0] * in_sz[1];
    V3DLONG sz0 = in_sz[0];
    double imgave,imgstd;
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
    mean_and_std(inimg1d,total_size,imgave,imgstd);
    cout<<"mean: "<<imgave<<", and std: "<<imgstd<<endl;
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.level==1)
        {
            V3DLONG thisx,thisy,thisz,dstpos;
            thisx=s.x;thisy=s.y;thisz=s.z;
            dstpos=long(thisz * sz01 + thisy * sz0 + thisx);
            if (dstpos>=total_size)
                    continue;
            listNeuron[i].level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
            if(useNeighborArea!=0)
            {
                NeuronSWC out=nodeRefine(inimg1d,listNeuron[i].x,listNeuron[i].y,listNeuron[i].z,in_sz,useNeighborArea);
                if(listNeuron[i].level+imgave+imgstd<out.level)
                {
                    listNeuron[i].level=out.level;
                    listNeuron[i].x=out.x;
                    listNeuron[i].y=out.y;
                    listNeuron[i].z=out.z;
                }
            }
        }
    }
}
std::vector<double> get_sorted_level_of_seg(V_NeuronSWC inseg)
{
    /*start from tip to root
    */
    vector<double> seg_levels; seg_levels.clear();
    QHash<V3DLONG,V3DLONG> nlist;nlist.clear();
    QHash<V3DLONG,V3DLONG> pnlist;pnlist.clear();
    for(V3DLONG r=0;r<inseg.row.size();r++)
    {
        nlist.insert(long(inseg.row.at(r).n),r);
        pnlist.insert(long(inseg.row.at(r).parent),r);
    }
    V3DLONG tip_id=0;
    for(V3DLONG r=0;r<inseg.row.size();r++)
    {
        if(!pnlist.contains(nlist[r]))
        {tip_id=r;break;}
    }
    //from tip_id to root
    V3DLONG cur_id=tip_id;
//    cout<<"level list: "<<endl;
    while(nlist.contains(long(inseg.row.at(cur_id).n)))
    {
        seg_levels.push_back((inseg.row.at(cur_id).level));
        if(nlist.contains(long(inseg.row.at(cur_id).parent)))
            cur_id=nlist.value(long(inseg.row.at(cur_id).parent));
        else
            break;
    }
    return seg_levels;
}
std::vector<int> smoothedZScore(std::vector<float> input,float threshold,float influence,int lag)
{
    if (input.size() <= lag + 2)
    {
        std::vector<int> emptyVec;
        return emptyVec;
    }

    //Initialise variables
    std::vector<int> signals_flag(input.size(), 0.0);
    std::vector<float> filteredY(input.size(), 0.0);
    std::vector<float> avgFilter(input.size(), 0.0);
    std::vector<float> stdFilter(input.size(), 0.0);
    std::vector<float> subVecStart(input.begin(), input.begin() + lag);
//    avgFilter[lag] = mean(subVecStart);
    float Vecsum = std::accumulate(subVecStart.begin(),subVecStart.end(), 0.0);
    uint slice_size = std::distance(subVecStart.begin(),subVecStart.end());
    float Vecmean=Vecsum / slice_size;avgFilter[lag]= Vecmean;
//    stdFilter[lag] = stdDev(subVecStart);
    float accum  = 0.0;
    for(V3DLONG i=0;i<subVecStart.size();i++)
        accum+=(subVecStart[i]-Vecmean)*(subVecStart[i]-Vecmean);
    stdFilter[lag]   = sqrt(accum/(subVecStart.size()-1));

    for (size_t i = lag + 1; i < input.size(); i++)
    {
        if (std::abs(input[i] - avgFilter[i - 1]) > threshold * stdFilter[i - 1])
        {
            if (input[i] > avgFilter[i - 1])
            {
                signals_flag[i] = 1; //# Positive signal
            }
            else
            {
                signals_flag[i] = -1; //# Negative signal
            }
            //Make influence lower
            filteredY[i] = influence* input[i] + (1 - influence) * filteredY[i - 1];
        }
        else
        {
            signals_flag[i] = 0; //# No signal
            filteredY[i] = input[i];
        }
        //Adjust the filters
        std::vector<float> subVec(filteredY.begin() + i - lag, filteredY.begin() + i);
        float subVecsum = std::accumulate(subVec.begin(),subVec.end(), 0.0);
        uint subVecslice_size = std::distance(subVec.begin(),subVec.end());
        float subVecmean=subVecsum / subVecslice_size;avgFilter[i]= subVecmean;
        float accum  = 0.0;
        for(V3DLONG ii=0;ii<subVec.size();ii++)
            accum+=(subVec[ii]-Vecmean)*(subVec[ii]-Vecmean);
         stdFilter[i]  = sqrt(accum/(subVec.size()-1));
    }
    cout<<"flag";
    for(V3DLONG i=0;i<input.size();i++)
        cout<<","<<signals_flag[i];
    cout<<endl;
    return signals_flag;
}
std::vector<int> peaks_in_seg(std::vector<double> input,float delta)
{
    /*The first argument is the vector to examine,
     * and the second is the peak threshold:
                * We require a difference of at least 0.5 between a peak and its surrounding in order to declare it as a peak.
      * Same goes with valleys.
      * Note, peak detection are used. this function is also usable for detecting valleys.
    */
    std::vector<int> signals_flag_highpeaks(input.size(), 0.0);
    std::vector<int> signals_flag_lowpeaks(input.size(), 0.0);
    int max_intensity=MinIntensity; int min_intensity=MaxIntensity;
    bool lookformax=true;V3DLONG maxpos=0;V3DLONG minpos=0;
    for(V3DLONG i=0;i<input.size();i++)
    {
        int this_intensity=input[i];
        if(this_intensity>max_intensity) {max_intensity=this_intensity;maxpos=i;}
        if(this_intensity<min_intensity)  {min_intensity=this_intensity;minpos=i;}
        if(lookformax)
        {
            if(this_intensity<max_intensity-delta)
            {signals_flag_highpeaks[maxpos]=1;min_intensity=this_intensity;lookformax=false;}
        }
        else
        {
            if(this_intensity>min_intensity+delta)
            {signals_flag_lowpeaks[minpos]=1;max_intensity=this_intensity;lookformax=true;}
        }
    }
//    cout<<"flag";
//    for(V3DLONG i=0;i<input.size();i++)
//        cout<<","<<signals_flag_highpeaks[i];
//    cout<<endl;
    return signals_flag_highpeaks;
}
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size)
{
    //return the intensity of the input node
//    int outRadius=0;
//    cout<<"---Node refine to the local maximal intensity----"<<endl;
    NeuronSWC out;
    out.level=0;
    V3DLONG thisx,thisy,thisz;
    out.x=thisx=nodex;
    out.y=thisy=nodey;
    out.z=thisz=nodez;

    V3DLONG sz01 = sz[0] * sz[1];
//    cout<<"----------node new------------"<<endl;
    for(V3DLONG iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz++)
    {
        for( V3DLONG iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size+1)&&(iy<sz[1]);iy++)
        {
            for(V3DLONG ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size+1)&&(ix<sz[0]);ix++)
            {

                V3DLONG pos = iz*sz01 + iy * sz[0] + ix;
                V3DLONG thisIntensity=inimg1d[pos];
                if(thisIntensity>out.level)
                {
//                    cout<<"update to intensity:"<<thisIntensity<<endl;
                    out.level=thisIntensity;
                    out.x=ix;
                    out.y=iy;
                    out.z=iz;
                }
            }
        }
    }
    return out;
}
