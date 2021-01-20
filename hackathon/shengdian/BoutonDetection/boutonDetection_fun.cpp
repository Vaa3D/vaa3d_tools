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
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath, QList<CellAPO> apolist, string outpath, int cropx, int cropy, int cropz)
{
    cout<<"Welcome into terafly block crop"<<endl;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    cout<<"Input crop size x="<<cropx<<";y="<<cropy<<";z="<<cropz<<endl;
    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
        end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
        end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
        end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2];
        cout<<"crop size x="<<start_x<<";y="<<start_y<<";z="<<start_z<<endl;
        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = cropx;
        in_sz[1] = cropy;
        in_sz[2] = cropz;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x)*(end_y-start_y)*(end_z-start_z);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
        if(im_cropped==NULL){
            continue;
        }
        //erosion
//        erosionImg(im_cropped,in_sz,3);
        QString tmpstr = "";
        tmpstr.append("_x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        tmpstr.append("_blocksize");
        tmpstr.append("_x_").append(QString("%1").arg(cropx));
        tmpstr.append("_y_").append(QString("%1").arg(cropy));
        tmpstr.append("_z_").append(QString("%1").arg(cropz));
        QString default_name = "Img"+tmpstr+".v3draw";
        QString save_path = QString::fromStdString(outpath);
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}

void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int block_size)
{
    cout<<"Welcome into bouton detection: crop part"<<endl;
    cout<<"get bouton apo file and crop img out"<<endl;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - block_size; if(start_x<0) start_x = 0;
        end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - block_size;if(start_y<0) start_y = 0;
        end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - block_size;if(start_z<0) start_z = 0;
        end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = block_size*2;
        in_sz[1] = block_size*2;
        in_sz[2] = block_size*2;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x)*(end_y-start_y)*(end_z-start_z);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
        if(im_cropped==NULL){
            cout<<"Crop fail"<<endl;
            continue;
        }
        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(s.x));
        tmpstr.append("_y").append(QString("%1").arg(s.y));
        tmpstr.append("_z").append(QString("%1").arg(s.z));
        QString default_name = "Bouton"+tmpstr+".tif";
//        QString default_name_apo="Bouton"+tmpstr+".apo";
        QString save_path = QString::fromStdString(outpath);
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}

void erosionImg(unsigned char *&inimg1d, long in_sz[], int kernelSize)
{
    /*Img processing: erosion
     *kernel size: default=3
     *revise the value of each pixels to it's local minimal
    */
    cout<<"Img processing: erosion"<<endl;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    long startx,starty,startz;
    long endx,endy,endz;
    cout<<"Input img size x="<<in_sz[0]<<";y="<<in_sz[1]<<";z="<<in_sz[2]<<endl;
    unsigned char * im_transfer = 0;
    V3DLONG pagesz;
    pagesz = sz01*sz0;
    try {im_transfer = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
//    im_transfer=inimg1d;
    //back up
    for(long ix=0;ix<in_sz[0];ix++)
    {
        for(long iy=0;iy<in_sz[1];iy++)
        {
            for(long iz=0;iz<in_sz[2];iz++)
            {
                im_transfer[iz * sz01 + iy * sz0 + ix]=inimg1d[iz * sz01 + iy * sz0 + ix];
            }
        }
    }
    cout<<"copy img pointer"<<endl;
    for(long ix=0;ix<in_sz[0];ix++)
    {
        for(long iy=0;iy<in_sz[1];iy++)
        {
            for(long iz=0;iz<in_sz[2];iz++)
            {
//                im_transfer[iz * sz01 + iy * sz0 + ix]=0;
                //get the local area
                startx=(ix-kernelSize>=0)?(ix-kernelSize):0;
                endx=(ix+kernelSize<in_sz[0])?(ix+kernelSize):in_sz[0];
                starty=(iy-kernelSize>=0)?(iy-kernelSize):0;
                endy=(iy+kernelSize<in_sz[1])?(iy+kernelSize):in_sz[1];
                startz=(iz-kernelSize>=0)?(iz-kernelSize):0;
                endz=(iz+kernelSize<in_sz[2])?(iz+kernelSize):in_sz[2];
                long tmpIntensity=im_transfer[iz * sz01 + iy * sz0 + ix];
                for(long kx=startx;kx<endx;kx++)
                {
                    for(long ky=starty;ky<endy;ky++)
                    {
                        for(long kz=startz;kz<endz;kz++)
                        {
                            long thistmpIntensity=im_transfer[kz * sz01 + ky * sz0 + kx];
                            tmpIntensity=(thistmpIntensity<tmpIntensity)?thistmpIntensity:tmpIntensity;

                        }
                    }
                }
                //erosion
                inimg1d[iz * sz01 + iy * sz0 + ix]=tmpIntensity;
            }
        }
    }
    if(im_transfer) {delete []im_transfer; im_transfer=0;}
}

void maskImg(V3DPluginCallback2 &callback, unsigned char *&inimg1d, QString outpath, long in_sz[], NeuronTree &nt, int maskRadius)
{
    /*for all the pixels in the dst block*/
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = listNeuron.size();
    if(siz<1) return;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    int startx,starty,startz;
    int endx,endy,endz;
    cout<<"Input img size x="<<in_sz[0]<<";y="<<in_sz[1]<<";z="<<in_sz[2]<<endl;
    unsigned char * im_transfer = 0;
    V3DLONG pagesz;
    pagesz = sz01*sz0;
    try {im_transfer = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
    for(int ix=0;ix<in_sz[0];ix++)
    {
        for(int iy=0;iy<in_sz[1];iy++)
        {
            for(int iz=0;iz<in_sz[2];iz++)
            {
                im_transfer[iz * sz01 + iy * sz0 + ix]=0;
            }
        }
    }

    for(int is=0;is<siz;is++)
    {
        int ix,iy,iz;
        NeuronSWC thiss = listNeuron[is];
        ix=int(thiss.x);
        iy=int(thiss.y);
        iz=int(thiss.z);
        startx=(ix-maskRadius>=0)?(ix-maskRadius):0;
        endx=(ix+maskRadius<in_sz[0])?(ix+maskRadius):in_sz[0];
        starty=(iy-maskRadius>=0)?(iy-maskRadius):0;
        endy=(iy+maskRadius<in_sz[1])?(iy+maskRadius):in_sz[1];
        startz=(iz-maskRadius>=0)?(iz-maskRadius):0;
        endz=(iz+maskRadius<in_sz[2])?(iz+maskRadius):in_sz[2];
        // for the surrounding area
        for(long iix=startx;iix<(endx);iix++)
        {
            for(long iiy=starty;iiy<(endy);iiy++)
            {
                for(long iiz=startz;iiz<(endz);iiz++)
                {
                    im_transfer[iiz * sz01 + iiy * sz0 + iix]=inimg1d[iiz * sz01 + iiy * sz0 + iix];

                }
            }
        }
    }
    //erosion
    //erosionImg(im_transfer,in_sz,3);
    //save img
    simple_saveimage_wrapper(callback, outpath.toStdString().c_str(),(unsigned char *)im_transfer,in_sz,1);
    //release pointer
     if(im_transfer) {delete []im_transfer; im_transfer=0;}
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
/*Remove the duplicated boutons at branch point*/
QList <CellAPO> removeBoutons(QList <CellAPO> inapo,V3DLONG removed_dist_thre)
{
    QList <CellAPO> outapo;outapo.clear();
    if(!inapo.size())
        return outapo;
    V3DLONG aposize=inapo.size();
    for (V3DLONG i=0;i<aposize-1;i++)
    {
        CellAPO apo1=inapo.at(i);
        bool isok=true;
        for (V3DLONG j=i+1;j<aposize;j++)
        {
            CellAPO apo2=inapo.at(j);
            V3DLONG b_dist=(apo1.x-apo2.x)*(apo1.x-apo2.x)+(apo1.y-apo2.y)*(apo1.y-apo2.y)+(apo1.z-apo2.z)*(apo1.z-apo2.z);
            if(b_dist<removed_dist_thre*removed_dist_thre)
            { isok=false;break;}
        }
        outapo.append(apo1);
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
/*Func: get radius of nodes in neuron tree
 *
*/
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

void getBoutonInImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt,int useNeighborArea)
{
    cout<<"Welcome into bouton detection: intensity part"<<endl;
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
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    double imgave,imgstd;
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
    mean_and_std(inimg1d,total_size,imgave,imgstd);
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.level==1)
        {
            int thisx,thisy,thisz;
            thisx=s.x;
            thisy=s.y;
            thisz=s.z;
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

void getSWCIntensityInTerafly(V3DPluginCallback2 &callback, string imgPath, QString inswc_file)
{
    cout<<"Welcome into Intensity statistics"<<endl;
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();

    float intensityList[siz];
    float meanList[siz];
    float stdintensityList[siz];
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=1;
        intensityList[i]=meanList[i]=stdintensityList[i]=0;
        hashNeuron.insert(listNeuron[i].n,i);
    }
    V3DLONG *in_zz = 0;
        //read img
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    //for wirting out
    QString inswc_file_out = inswc_file + "_intensityStatistics.csv";
    ofstream fp(inswc_file_out.toStdString().c_str());
    if(!fp.is_open()){
        cout<<"Can't create file for out"<<endl;
        exit(1);
    }
    fp<<"Intensity,Mean,Std"<<endl;
    //start from here
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and radius=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.radius==1)
        {
            //get a block
            long start_x,start_y,start_z,end_x,end_y,end_z;
            long block_size=64;
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
            in_sz[3]=in_zz[3];
            V3DLONG *sz;
            sz=in_sz;
            long sz0 = sz[0];
            long sz01 = sz[0] * sz[1];
            unsigned char * inimg1d = 0;
            inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
            //            //version one
            int thisx,thisy,thisz;
            thisx=s.x-start_x;
            thisy=s.y-start_y;
            thisz=s.z-start_z;

            listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
//            cout<<"This node intensity is "<<listNeuron[i].radius<<endl;
            double imgave,imgstd;
            V3DLONG total_size=8*block_size;
            mean_and_std(inimg1d,total_size,imgave,imgstd);
//            cout<<"here"<<endl;
            intensityList[i]=listNeuron[i].level;
            meanList[i]=imgave;
            stdintensityList[i]=imgstd;
            cout<<intensityList[i]<<","<<imgave<<","<<imgstd<<endl;
            fp<<intensityList[i]<<","<<imgave<<","<<imgstd<<endl;

            if(inimg1d) {delete []inimg1d; inimg1d=0;}
        }
    }
    fp.close();
    //out swc file
    float intensityMean,imgMean,imgstdMean;
    intensityMean=0;
    imgMean=0;
    imgstdMean=0;
    for(V3DLONG i=0;i<siz;i++)
    {
        intensityMean+=intensityList[i];
        imgMean+=meanList[i];
        imgstdMean+=stdintensityList[i];
        listNeuron[i].radius=10*listNeuron[i].level/255;
    }
    intensityMean/=siz;
    imgMean/=siz;
    imgstdMean/=siz;
    QString tmpstr = "";
    tmpstr.append("_NodeIntensity_").append(QString("%1").arg(intensityMean));
    tmpstr.append("_ImgIntensityMean_").append(QString("%1").arg(imgMean));
    tmpstr.append("_ImgIntensityStd_").append(QString("%1").arg(imgstdMean));
    QString outswc_name =inswc_file+tmpstr+".eswc";
    writeESWC_file(outswc_name,nt);
    cout<<"Done:"<<endl;
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
