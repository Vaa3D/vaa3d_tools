#include "boutonDetection_fun.h"
#include <QHash>
#include "volimg_proc.h"
#include <fstream>
void getBoutonInTerafly(V3DPluginCallback2 &callback, string imgPath, NeuronTree& nt,int allnode)
{
    cout<<"Welcome into bouton detection: intensity part"<<endl;
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
                int start_x,start_y,start_z,end_x,end_y,end_z;
                int block_size=64;
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
                inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
    //            //version one
                int thisx,thisy,thisz;
                thisx=s.x-start_x;
                thisy=s.y-start_y;
                thisz=s.z-start_z;

                listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                //refine the intensity to local maximal
                float childIntensity=listNeuron[i].level;
                //get their parent node intensity

                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    NeuronSWC sp=listNeuron[pid];
                    //consider the distance of child-parent node
                    float distanceThre=5;
                    float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                    if(child_parent_distance<distanceThre*distanceThre)
                    {

                        NeuronSWC sm;
                        sm.x=(sp.x-s.x)/2+s.x;
                        sm.y=(sp.y-s.y)/2+s.y;
                        sm.z=(sp.z-s.z)/2+s.z;
                        int thisxm,thisym,thiszm;
                        thisxm=sm.x-start_x;
                        thisym=sm.y-start_y;
                        thiszm=sm.z-start_z;
                        int thisxp,thisyp,thiszp;
                        thisxp=sp.x-start_x;
                        thisyp=sp.y-start_y;
                        thiszp=sp.z-start_z;
                        float parentIntensity=inimg1d[int(thiszp) * sz01 + int(thisyp) * sz0 + int(thisxp)];
                        float middleIntensity=inimg1d[int(thiszm) * sz01 + int(thisym) * sz0 + int(thisxm)];
                        //this may need another threshold
                        int updateThre=10;
                        if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                        {
                            cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<endl;
                            cout<<"this node id:"<<s.n;
                            //                    update child node Intensity
                            listNeuron[i].level=middleIntensity;
                        }
                    }
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
                int block_size=32;
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
                inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
    //            //version one
                int thisx,thisy,thisz;
                thisx=s.x-start_x;
                thisy=s.y-start_y;
                thisz=s.z-start_z;

                listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
//                cout<<"level:"<<listNeuron[i].level<<endl;
                //refine the intensity to local maximal
                float childIntensity=listNeuron[i].level;
                //get their parent node intensity

                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    NeuronSWC sp=listNeuron[pid];
                    //consider the distance of child-parent node
                    float distanceThre=5;
                    float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                    if(child_parent_distance<distanceThre*distanceThre)
                    {

                        NeuronSWC sm;
                        sm.x=(sp.x-s.x)/2+s.x;
                        sm.y=(sp.y-s.y)/2+s.y;
                        sm.z=(sp.z-s.z)/2+s.z;
                        int thisxm,thisym,thiszm;
                        thisxm=sm.x-start_x;
                        thisym=sm.y-start_y;
                        thiszm=sm.z-start_z;
                        int thisxp,thisyp,thiszp;
                        thisxp=sp.x-start_x;
                        thisyp=sp.y-start_y;
                        thiszp=sp.z-start_z;
                        float parentIntensity=inimg1d[int(thiszp) * sz01 + int(thisyp) * sz0 + int(thisxp)];
                        float middleIntensity=inimg1d[int(thiszm) * sz01 + int(thisym) * sz0 + int(thisxm)];
                        //this may need another threshold
                        int updateThre=10;
                        if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                        {
                            cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<endl;
                            cout<<"this node id:"<<s.n;
                            //                    update child node Intensity
                            listNeuron[i].level=middleIntensity;
                        }
                    }
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
QList <CellAPO> getBouton(NeuronTree nt, int threshold,int allnode)
{
    cout<<"Welcome into bouton detection: filter part"<<endl;

    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();

    QHash<V3DLONG,int> id_threshold;
    id_threshold.clear();
    id_threshold=getIntensityStd(nt,128);//key is line id and value is std
    QHash <int, int>  hashNeuron,hashchild;
    hashNeuron.clear();
    hashchild.clear();
    long boutonnodelist[siz];

    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        boutonnodelist[i]=0;
        hashNeuron.insert(s.n,i);
        hashchild.insert(i,0);
    }

    float Ip1,Ip2,Ip3,Ip4;
    float intensity_change=0;
//    int thre_low=4*threshold/5;
    /*Two level threshold seperate the region into three intervals:
     *p1 maybe is a P-site or maybe not. if p1 is a P-site, it should be known by its child.
     *So consider the situation below, p1 is not a P-site, which means it has a low intensity:
     *p1,p2,p3,p4(p1->p2->p3->p4)
     *R1(~,Ip1),R2(Ip1,threshold),R3(threshold,~)
     *0. p2 in R1; PASS this point
     *1. p2 in R2: look at P3
     *1.1. p3 in R1; use thre_low,
     *     if  Ip2-Ip3 >thre_low, this is P-site-low.
     *     if  Ip2-Ip3 >threshold, this is P-site-high.
     *1.2. p3 in R2;
     *      look at P4 (maybe p2-p3 line direction have the local maximal P-site)
     *              p4 in R1; P-site should be in p2 or p3. if we store this point, maybe we can further improve performance
     *              p4 in R2; pass this point
     *              p4 in R3; p4 =P-site-high
     *1.3. p3 in R3; p3 =P-site-high
     *2. p2 in R3; p2=P-site-high
    */
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];

        if(id_threshold.contains(i))
        {
            threshold=id_threshold.value(i);
        }
        else
        {
            cout<<"can not find the key of "<<i<<endl;
            threshold=20;
        }
        if(allnode==0 && s.type!=2)
            continue;
        if(s.parent>0)
        {
            long p2_id=hashNeuron.value(s.parent);
            hashchild[p2_id]=hashchild[p2_id]+1;
            if(s.level>1)
            {
                Ip1=s.level;
                NeuronSWC p2=listNeuron[p2_id];
                Ip2=p2.level;
                intensity_change=Ip2-Ip1;
                if(intensity_change>=threshold)
                {
                    cout<<"(internal p2, p2>p1)bouton: index="<<p2.n<<endl;
                    cout<<"threshold intensity is "<<threshold<<endl;
                    boutonnodelist[p2_id]=Ip2;
                }
                else if(intensity_change>0&&intensity_change<threshold)
                {
                    //1.1
                    if(p2.parent<0)
                        continue;
                    long p3_id=hashNeuron.value(p2.parent);
                    hashchild[p3_id]=hashchild[p3_id]+1;
                    NeuronSWC p3=listNeuron[p3_id];
                    Ip3=p3.level;
                    int p3change=Ip3-Ip1;
                    if(p3change>=threshold)
                    {
                        cout<<"(internal p3, 0<p2-p1<thre && p3-p1 > thre)bouton: index="<<p3.n<<endl;
                        cout<<"threshold intensity is "<<threshold<<endl;
                        boutonnodelist[p3_id]=Ip3;
                    }
                    else if(p3change<0)
                    {
                        if(Ip2-Ip3>=threshold)
                        {
                            cout<<"(internal p3, 0<p2-p1<thre && p2-p3 > thre && p3<p1 )bouton: index="<<p2.n<<endl;
                            cout<<"threshold intensity is "<<threshold<<endl;
                            boutonnodelist[p2_id]=Ip2;
                        }
                        else
                            continue;
                    }
                    else
                    {
                        //0<p2-p1<thre && 0<p3-p1<thre
                        if(p3.parent<0)
                        {
                            continue;
                        }
                        long p4_id=hashNeuron.value(p3.parent);
                        hashchild[p4_id]=hashchild[p4_id]+1;
                        NeuronSWC p4=listNeuron[p4_id];
                        Ip4=p4.level;
                        if(Ip4-Ip1>=threshold)
                        {
                            cout<<"(internal p4, p4-p1>thre )bouton: index="<<p4.n<<endl;
                            cout<<"threshold intensity is "<<threshold<<endl;
                            boutonnodelist[p4_id]=Ip4;
                        }
                        else
                        {
                            //0<p2-p1<thre && 0<p3-p1<thre && 0<p4-p1<thre
                            continue;
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        }
        else
            hashchild[hashNeuron.value(s.n)]=hashchild[hashNeuron.value(s.n)]+1;
    }
    //find all the tips
    QHash<int, int>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        if(hcit.value()==0)
        {

            int childid=hcit.key();
            NeuronSWC s = listNeuron[childid];
            if(allnode==0)
            {
                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    Ip1=s.level;
                    NeuronSWC s_pp=listNeuron[pid];
                    if(id_threshold.contains(pid))
                        threshold=id_threshold.value(pid);
                    else
                        threshold=20;
                    Ip2=s_pp.level;
                    intensity_change=Ip1-Ip2;
                    if(intensity_change>threshold)
                    {
                        cout<<"tip bouton: index="<<s.n<<endl;
                        boutonnodelist[s.n-1]=Ip1;
                    }
                }
            }
            else
            {
                if(s.parent>0&&s.type==2)
                {
                    long pid=hashNeuron.value(s.parent);
                    Ip1=s.level;
                    NeuronSWC s_pp=listNeuron[pid];

                    Ip2=s_pp.level;
                    intensity_change=Ip1-Ip2;
                    if(id_threshold.contains(pid))
                        threshold=id_threshold.value(pid);
                    else
                        threshold=20;
//                    threshold=id_threshold.value(pid);
                    if(intensity_change>threshold)
                    {
                        cout<<"tip bouton: index="<<s.n<<endl;
                        boutonnodelist[s.n-1]=Ip1;
                    }
                }
            }

        }
    }
    //for all the P-sites nodes
    QList <CellAPO> apolist;apolist.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        if(boutonnodelist[i]==0)
            continue;
        NeuronSWC s = listNeuron[i];
        //parent node and grandparent node should not be bouton

        if(s.parent>0)
        {
            long pid=hashNeuron.value(s.parent);
            NeuronSWC s_p=listNeuron[pid];

            if(boutonnodelist[pid]>0)
                continue;
            if(s_p.parent>0)
            {
                long ppid=hashNeuron.value(s_p.parent);
//                NeuronSWC s_pp=listNeuron[ppid];
                if(boutonnodelist[ppid]>0)
                    continue;
            }
        }
        threshold=id_threshold.value(i);
        CellAPO apo;
        apo.x=s.x+1;
        apo.y=s.y+1;
        apo.z=s.z+1;
        apo.intensity=s.radius;
        apo.volsize=s.level;
        apo.color.r=0;
        apo.color.g=0;
        apo.color.b=255;
        apolist.push_back(apo);
    }
    return apolist;
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
        pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
        if(im_cropped==NULL){
            continue;
        }
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
        pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
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
        QString default_name_apo="Bouton"+tmpstr+".apo";
        QString save_path = QString::fromStdString(outpath);
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        QString save_path_apo=save_path+"/"+default_name_apo;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
        //save apo file
        QList <CellAPO> apoboutonlist;apoboutonlist.clear();
        CellAPO tmp;
        tmp.x=block_size;
        tmp.y=block_size;
        tmp.z=block_size;
        tmp.volsize=s.intensity;
        tmp.color.r=0;
        tmp.color.g=0;
        tmp.color.b=255;
        apoboutonlist.push_back(tmp);
        writeAPO_file(save_path_apo,apoboutonlist);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}
void splitSWC(V3DPluginCallback2 &callback, string imgPath,string inswc_file, string outpath, int cropx, int cropy, int cropz)
{
    cout<<"Crop swc and terafly block: split swc"<<endl;
    //read swc
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    QList <CellAPO> outapo;
    outapo.clear();
    V3DLONG siz = nt.listNeuron.size();
    size_t minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX;
    size_t maxX = 0, maxY = 0, maxZ = 0;
    for(int i=0; i<siz; i++){
        NeuronSWC s = nt.listNeuron[i];
        if(minX>s.x) minX = s.x;
        if(maxX<s.x) maxX = s.x;
        if(minY>s.y) minY = s.y;
        if(maxY<s.y) maxY = s.y;
        if(minZ>s.z) minZ = s.z;
        if(maxZ<s.z) maxZ = s.z;
    }
    int xblockNum=1,yblockNum=1,zblockNum=1;
    if(cropx<1&&cropy<1&&cropz<1)
        cropx=cropy=cropz=256;
    xblockNum=int((maxX-minX)/cropx)+1;
    yblockNum=int((maxY-minY)/cropy)+1;
    zblockNum=int((maxZ-minZ)/cropz)+1;
    cout<<"Blockx min="<<minX<<",Blockx max="<<maxX<<endl;
    cout<<"Blocky min="<<minY<<",Blocky max="<<maxY<<endl;
    cout<<"Blockz min="<<minZ<<",Blockz max="<<maxZ<<endl;
    cout<<"split block x="<<xblockNum<<endl;
    cout<<"split block y="<<yblockNum<<endl;
    cout<<"split block z="<<zblockNum<<endl;
    //for every blocks
    float startx,starty,startz;
    float endx,endy,endz;
    startx=endx=minX;
    starty=endy=minY;
    startz=endz=minZ;
    for(long ix=0;ix<xblockNum;ix++)
    {
        if(ix==0) startx=minX; else startx=endx;
        endx=((startx+cropx)>maxX)?maxX:(startx+cropx);
        for(long iy=0;iy<yblockNum;iy++)
        {
            starty=(iy==0)?minY:endy;
            endy=((starty+cropy)>maxY)?maxY:(starty+cropy);
            for(long iz=0;iz<zblockNum;iz++)
            {
                startz=(iz==0)?minZ:endz;
                endz=((startz+cropz)>maxZ)?maxZ:(startz+cropz);
                //crop swc
//                cout<<"----"<<endl;
//                cout<<"x="<<startx<<","<<endx<<endl;
//                cout<<"y="<<starty<<","<<endy<<endl;
//                cout<<"z="<<startz<<","<<endz<<endl;
                NeuronTree nt_corped=NeuronTree();
                long count = 0;
                for(long i=0; i<siz; i++)
                {
                    NeuronSWC thiss = listNeuron[i];
                    if(thiss.x>=startx&&thiss.x<endx
                            &&thiss.y>=starty&&thiss.y<endy
                            &&thiss.z>=startz&&thiss.z<endz)
                    {
                        thiss.x-=startx;
                        thiss.y-=starty;
                        thiss.z-=startz;
                        nt_corped.listNeuron.push_back(thiss);
                        nt_corped.hashNeuron.insert(thiss.n,count);
                        count++;
                    }
                }
                //save to file
                if(nt_corped.listNeuron.size()>0&&count>0)
                {
                    cout<<"neuron tree size is "<<nt_corped.listNeuron.size()<<endl;
                    QString tmpstr = "";
                    tmpstr.append("_x_").append(QString("%1").arg((startx+endx)/2));
                    tmpstr.append("_y_").append(QString("%1").arg((starty+endy)/2));
                    tmpstr.append("_z_").append(QString("%1").arg((startz+endz)/2));
                    tmpstr.append("_blocksize");
                    tmpstr.append("_x_").append(QString("%1").arg((endx-startx)));
                    tmpstr.append("_y_").append(QString("%1").arg((endy-starty)));
                    tmpstr.append("_z_").append(QString("%1").arg((endz-startz)));
                    QString default_name = "Croped"+tmpstr+".swc";
                    QString save_path = QString::fromStdString(outpath)+"/CropedSWC";
                    QDir path(save_path);
                    if(!path.exists())
                    {
                        path.mkpath(save_path);
                    }
                    QString save_croped_path =save_path+"/"+default_name;
//                    cout<<"save img path:"<<save_croped_path.toStdString()<<endl;
                    writeESWC_file(save_croped_path,nt_corped);
                    //out the center of this block to apo
                    CellAPO tmp;
                    tmp.x=float((startx+endx)/2);
                    tmp.y=float((starty+endy)/2);
                    tmp.z=float((startz+endz)/2);
                    tmp.volsize=ix+iy+iz+1;
                    tmp.color.r=0;
                    tmp.color.g=0;
                    tmp.color.b=255;
                    outapo.push_back(tmp);
                    string outImgpath=outpath+"/CropedImg";
                    getTeraflyBlock(callback,imgPath,outapo,outImgpath,endx-startx,endy-starty,endz-startz);
                    outapo.clear();
                }
            }
        }
    }
//    return outapo;
}

/*Given a NeuronTree and a swc node in this NeuronTree, return std intensity of surrounding area (thre_size)*/
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size)
{
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash<V3DLONG,int> id_threshold;//key is line id and value is std
    id_threshold.clear();
    QList<long> surIntensity;
//    long processedNode[siz];
    if(nt.listNeuron.size()==0)
    {
        cout<<"Can't read neuronTree and input error"<<endl;
        return id_threshold;
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC thisnode=listNeuron[i];
        //get a surrounding block nodelist
        long long blockIntensityAll=thisnode.level;
        V3DLONG blockNodeNum=1;
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
        float blockIntensityAvearage=blockIntensityAll/blockNodeNum;

        //standard
        float varIntensity=0;
        for (V3DLONG j=0;j<blockNodeNum;j++)
        {
            varIntensity+=(surIntensity.at(j)-blockIntensityAvearage)*(surIntensity.at(j)-blockIntensityAvearage);
        }
        varIntensity/=blockNodeNum;
        float stdIntensity=sqrt(varIntensity);
        if(stdIntensity<0)
        {
            cout<<"varIntensity="<<varIntensity<<endl;
            cout<<"Avearage="<<blockIntensityAvearage<<";index="<<i<<endl;
            cout<<"blockNode Num="<<blockNodeNum<<endl;
        }
        if(blockIntensityAvearage>120)
        {
            id_threshold.insert(i,stdIntensity*3);
            cout<<"Avearage="<<blockIntensityAvearage<<";index="<<i<<endl;
        }
        else
            id_threshold.insert(i,stdIntensity);
//        id_threshold.insert(i,stdIntensity);
    }
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
    cout<<"thresh="<<bkg_thresh<<endl;

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
     if(inimg1d) {delete []inimg1d; inimg1d=0;}
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

    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.type==2 && s.level==1)
        {
            int thisx,thisy,thisz;
            thisx=s.x;
            thisy=s.y;
            thisz=s.z;
            if(useNeighborArea!=0)
            {
                listNeuron[i].level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                NeuronSWC out=nodeIntensity(inimg1d,listNeuron[i].x,listNeuron[i].y,listNeuron[i].z,in_sz,1);
                if(listNeuron[i].level<out.level)
                {
                    cout<<"update intensity:old="<<listNeuron[i].level<<";New="<<out.level<<endl;
                    cout<<"Distance changes:x="<<(listNeuron[i].x-out.x)<<";y="<<(listNeuron[i].y-out.y)<<";z="<<(listNeuron[i].z-out.z)<<endl;
                    listNeuron[i].level=out.level;
                    listNeuron[i].x=out.x;
                    listNeuron[i].y=out.y;
                    listNeuron[i].z=out.z;
                }
            }
            else
            {
                float childIntensity=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                listNeuron[i].level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                //get their parent node intensity

                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    NeuronSWC sp=listNeuron[pid];
                    //consider the distance of child-parent node
                    float distanceThre=5;
                    float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                    if(child_parent_distance<distanceThre*distanceThre)
                    {

                        NeuronSWC sm;
                        sm.x=(sp.x-s.x)/2+s.x;
                        sm.y=(sp.y-s.y)/2+s.y;
                        sm.z=(sp.z-s.z)/2+s.z;
                        float parentIntensity=inimg1d[int(sp.z) * sz01 + int(sp.y) * sz0 + int(sp.x)];
                        float middleIntensity=inimg1d[int(sm.z) * sz01 + int(sm.y) * sz0 + int(sm.x)];
                        //this may need another threshold
                        int updateThre=10;
                        if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                        {
                            cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<";this node id:"<<s.n<<endl;

                            //                    update child node Intensity
                            listNeuron[i].level=middleIntensity;
                        }
                    }
                }
            }
//            cout<<"level:"<<listNeuron[i].level<<endl;
        }
    }
    //release pointer
     if(inimg1d) {delete []inimg1d; inimg1d=0;}
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
            inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
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

NeuronSWC nodeIntensity(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size)
{
    //return the intensity of the input node
//    int outRadius=0;
    NeuronSWC out;
    out.level=0;
    float thisx,thisy,thisz;
    out.x=thisx=nodex;
    out.y=thisy=nodey;
    out.z=thisz=nodez;

    V3DLONG sz01 = sz[0] * sz[1];
    for(float iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz=iz+0.2)
    {

        for( float iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size-1)&&(iy<sz[1]);iy=iy+0.2)
        {
            for(float ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size-1)&&(ix<sz[0]);ix=ix+0.2)
            {
                long pos = iz*sz01 + iy * sz[0] + ix;
                int thisRadius=inimg1d[pos];
                if(thisRadius>out.level)
                {
                    out.level=thisRadius;
                    out.x=ix;
                    out.y=iy;
                    out.z=iz;
                }
            }
        }
    }
    return out;
}
