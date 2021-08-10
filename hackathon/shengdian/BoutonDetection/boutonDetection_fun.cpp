#include "boutonDetection_fun.h"
void refinement_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool mean_shift,bool in_terafly){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    string inswc_file,inimg_file;
    if(infiles.size()>=2) {inimg_file = infiles[0];inswc_file = infiles[1];}
    else
        printHelp();
    //read para list
    int refine_radius=(inparas.size()>=1)?atoi(inparas[0]):8;
    double bkg_thre_theta=(inparas.size()>=2)?atoi(inparas[1]):0.7;
    int interpolation_pixels=(inparas.size()>=3)?atoi(inparas[2]):3;
    int half_crop_size=(inparas.size()>=4)?atoi(inparas[3]):128;


    //read input swc to neuron-tree
   NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
   if(!nt.listNeuron.size()) return;

   // shift or refinement function
   if(in_terafly)
       refinement_terafly_fun(callback,inimg_file,nt,mean_shift,refine_radius,half_crop_size,bkg_thre_theta);
   else
       refinement_Image_fun(callback,inimg_file,nt,mean_shift,refine_radius,bkg_thre_theta);

   NeuronTree nt_out=linearInterpolation(nt,interpolation_pixels);

    //out
    string refined_swc=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_refined.eswc");
    writeESWC_file(QString::fromStdString(refined_swc),nt_out);
}
void refinement_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,bool mean_shift,int refine_radius,long half_block_size,double bkg_bias){
    cout<<"Refinement uses mean-shift, under terafly datasets"<<endl;
    NeuronTree nt_raw; nt_raw.deepCopy(nt);
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = listNeuron.size();
    vector<V3DLONG> scanned(siz,0);
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}
    int min_dist_to_block_edge=32;
    for(V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC ss = listNeuron[i];
        if(scanned.at(i)==0)
        {
            //get a block
            long start_x,start_y,start_z,end_x,end_y,end_z;
            start_x = ss.x - half_block_size; if(start_x<0) start_x = 0;
            end_x = ss.x + half_block_size; if(end_x > in_zz[0]) end_x = in_zz[0]-1;
            start_y =ss.y - half_block_size;if(start_y<0) start_y = 0;
            end_y = ss.y + half_block_size;if(end_y > in_zz[1]) end_y = in_zz[1]-1;
            start_z = ss.z - half_block_size;if(start_z<0) start_z = 0;
            end_z = ss.z + half_block_size;if(end_z > in_zz[2]) end_z = in_zz[2]-1;

            V3DLONG in_sz[4];
            in_sz[0] = end_x - start_x+1;
            in_sz[1] = end_y - start_y+1;
            in_sz[2] = end_z - start_z+1;
            in_sz[3]=in_zz[3];
            V3DLONG sz01 = in_sz[0] * in_sz[1];
            V3DLONG sz0 = in_sz[0];

            unsigned char * inimg1d = 0;
            V3DLONG pagesz= in_sz[0] * in_sz[1]*in_sz[2]*in_sz[3];
            try {inimg1d = new unsigned char [pagesz];}
            catch(...)  {cout<<"cannot allocate memory for processing."<<endl; return;}
            inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            if(inimg1d==NULL){ cout<<"Crop fail"<<endl;continue; }

            double imgave,imgstd;
            mean_and_std(inimg1d,pagesz,imgave,imgstd);
            imgstd=MAX(imgstd,10);
            double bkg_thresh= imgave+imgstd+bkg_bias;
            bkg_thresh=MAX(bkg_thresh,20);

            //for all the node inside this block
            for(V3DLONG j=0;j<siz;j++){
                NeuronSWC sj = listNeuron[j];
                if(scanned.at(j)==0&&
                        (sj.x-start_x)>=min_dist_to_block_edge&&(end_x-sj.x)>=min_dist_to_block_edge&&
                         (sj.y-start_y)>=min_dist_to_block_edge&&(end_y-sj.y)>=min_dist_to_block_edge&&
                         (sj.z-start_z)>=min_dist_to_block_edge&&(end_z-sj.z)>=min_dist_to_block_edge)
                {
                    V3DLONG thisx,thisy,thisz;                    thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                    NeuronSWC out;
                    if(mean_shift)
                        out=calc_mean_shift_center(inimg1d,sj,in_sz,bkg_thresh,refine_radius);
                    else
                        out=nodeRefine(inimg1d,sj,in_sz,refine_radius);
                    scanned[j]=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                    listNeuron[j].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                    if(scanned[j]+imgstd<out.level)
                    {
                        listNeuron[j].level=bkg_thresh;
                        listNeuron[j].x=float(start_x)+out.x;
                        listNeuron[j].y=float(start_y)+out.y;
                        listNeuron[j].z=float(start_z)+out.z;
                    }
                }
            }
            if(inimg1d) {delete []inimg1d; inimg1d=0;}
        }
    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}
}
void refinement_Image_fun(V3DPluginCallback2 &callback,string inimg_file, NeuronTree& nt,bool mean_shift,int refine_radius,double bkg_bias){
    cout<<"Refinement uses mean-shift, under image block"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();

    QHash <V3DLONG, V3DLONG>  hashNeuron;    hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(listNeuron[i].n,i);

    //read image file
    unsigned char * inimg1d = 0;V3DLONG in_sz[4];int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return;

    V3DLONG sz01 = in_sz[0] * in_sz[1];
    V3DLONG sz0 = in_sz[0];
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];

    double imgave,imgstd;
    mean_and_std(inimg1d,total_size,imgave,imgstd);
    imgstd=MAX(imgstd,10);
    double bkg_thresh= imgave+imgstd+bkg_bias;
    bkg_thresh=MAX(bkg_thresh,20);

    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node
        NeuronSWC s = listNeuron[i];
        V3DLONG thisx,thisy,thisz;        thisx=s.x;thisy=s.y;thisz=s.z;
        s.level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
        listNeuron[i].level=bkg_thresh;

        NeuronSWC out;
        if(mean_shift)
            out=calc_mean_shift_center(inimg1d,listNeuron.at(i),in_sz,bkg_thresh,refine_radius);
        else
            out=nodeRefine(inimg1d,listNeuron.at(i),in_sz,refine_radius);
        if(s.level+imgstd<out.level)
        {
            s.level=out.level;
            listNeuron[i].x=out.x;
            listNeuron[i].y=out.y;
            listNeuron[i].z=out.z;
            listNeuron[i].level=bkg_thresh;
        }
    }
    if(inimg1d) {delete []inimg1d; inimg1d=0;}
}
NeuronSWC calc_mean_shift_center(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double bkg_thre,int windowradius)
{
    //qDebug()<<"methodcode:"<<methodcode;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    V3DLONG pos;
//    vector<V3DLONG> coord;

    float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
    float center_dis=1;
    vector<float> center_float(3,0);

    x=snode.x;y=snode.y;z=snode.z;
    //qDebug()<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<ind;

    //find out the channel with the maximum intensity for the marker
    v_prev=inimg1d[long(z)*z_offset+long(y)*y_offset+long(x)];
    int testCount=0;
    int testCount1=0;

    while (center_dis>=0.5 && testCount<50)
    {
        total_x=total_y=total_z=sum_v=0;
        testCount++;
        testCount=testCount1=0;

        for(V3DLONG dx=MAX(x+0.5-windowradius,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius); dx++){
            for(V3DLONG dy=MAX(y+0.5-windowradius,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius); dy++){
                for(V3DLONG dz=MAX(z+0.5-windowradius,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius); dz++){
                    pos=dz*z_offset+dy*y_offset+dx;
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=inimg1d[pos];
                    if(v_color<bkg_thre)
                        v_color=0;
                    total_x=v_color*(float)dx+total_x;
                    total_y=v_color*(float)dy+total_y;
                    total_z=v_color*(float)dz+total_z;
                    sum_v=sum_v+v_color;

                    testCount++;
                    if(v_color>100)
                        testCount1++;
                 }
             }
         }

        center_float[0]=total_x/sum_v;
        center_float[1]=total_y/sum_v;
        center_float[2]=total_z/sum_v;

        if (total_x<1e-5||total_y<1e-5||total_z<1e-5) //a very dark marker.
        {

            v3d_msg("Sphere surrounding the marker is zero. Mean-shift cannot happen. Marker location will not move",0);
            center_float[0]=x;
            center_float[1]=y;
            center_float[2]=z;
            return snode;
        }

        float tmp_1=(center_float[0]-x)*(center_float[0]-x)+(center_float[1]-y)*(center_float[1]-y)
                    +(center_float[2]-z)*(center_float[2]-z);
        center_dis=sqrt(tmp_1);
        x=center_float[0]; y=center_float[1]; z=center_float[2];
    }

    NeuronSWC out_center=snode;
    out_center.x=center_float[0]; out_center.y=center_float[1]; out_center.z=center_float[2];
    out_center.level=inimg1d[long(out_center.z)*z_offset+long(out_center.y)*y_offset+long(out_center.x)];
//    if(out_center.level>v_prev)
//        cout<<"shift intensity improve= "<<out_center.level-v_prev<<endl;
    return out_center;
}
NeuronSWC nodeRefine(unsigned char * & inimg1d,NeuronSWC s,V3DLONG * sz,int neighbor_size)
{
    /*shift to local center*/
//    cout<<"---Node refine to the local maximal intensity----"<<endl;
    NeuronSWC out=s;
    V3DLONG thisx,thisy,thisz;
    thisx=s.x;    thisy=s.y;    thisz=s.z;

    V3DLONG sz01 = sz[0] * sz[1];
    out.level=inimg1d[(thisz*sz01 + thisy * sz[0] + thisx)];

    V3DLONG maxIntensity=inimg1d[(thisz*sz01 + thisy * sz[0] + thisx)];
    for(V3DLONG iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz++)
        for( V3DLONG iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size+1)&&(iy<sz[1]);iy++)
            for(V3DLONG ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size+1)&&(ix<sz[0]);ix++)
                if(inimg1d[(iz*sz01 + iy * sz[0] + ix)]>maxIntensity)
                    maxIntensity=inimg1d[(iz*sz01 + iy * sz[0] + ix)];
    if(maxIntensity<=out.level)
        return out;
    uint candicates_num=0;
    double x_candicates,y_candicates,z_candicates;
    x_candicates=y_candicates=z_candicates=0;
    for(V3DLONG iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz++)
    {
        for( V3DLONG iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size+1)&&(iy<sz[1]);iy++)
        {
            for(V3DLONG ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size+1)&&(ix<sz[0]);ix++)
            {
                V3DLONG pos = iz*sz01 + iy * sz[0] + ix;
                V3DLONG thisIntensity=inimg1d[pos];
                if(thisIntensity==maxIntensity)
                {
                    x_candicates+=ix;
                    y_candicates+=iy;
                    z_candicates+=iz;
                    candicates_num+=1;
                }
            }
        }
    }
    out.x=float(x_candicates/candicates_num);
    out.y=float(y_candicates/candicates_num);
    out.z=float(z_candicates/candicates_num);
//    V3DLONG thisx,thisy,thisz;
    thisx=long(out.x);    thisy=long(out.y); thisz=long(out.z);
    out.level=inimg1d[(thisz*sz01 + thisy * sz[0] + thisx)];
    return out;
}
double getAngleofNodeVector(NeuronSWC n0,NeuronSWC n1,NeuronSWC n2){
    double theta=0.0;
    double v1,v2;
    double err=0.000000001;
    v1=sqrt((n1.x-n0.x)*(n1.x-n0.x)+(n1.y-n0.y)*(n1.y-n0.y)+(n1.z-n0.z)*(n1.z-n0.z));
    v2=sqrt((n2.x-n0.x)*(n2.x-n0.x)+(n2.y-n0.y)*(n2.y-n0.y)+(n2.z-n0.z)*(n2.z-n0.z));
    if(v1>err&&v2>err){
        theta=acos(abs((n1.x-n0.x)*(n2.x-n0.x)+(n1.y-n0.y)*(n2.y-n0.y)+(n1.z-n0.z)*(n2.z-n0.z))/(v1*v2));
    }
    return (theta*180/PI);
}
NeuronSWC lineRefine(unsigned char * & inimg1d,V3DLONG * sz,NeuronSWC snode,NeuronSWC spnode, int sqhere_radius,int searching_line_radius){
    /*V2: shift to local intensity maximum
     * searching area: 求解域是一个球体的基础上，屏蔽沿着Line方向的两个锥体
     * 主要目的是想 让node沿着Line法向空间作refinement，而尽量减少沿着Line方向的refinement，只有这样才能增大refinement搜索区域，而不至于让所有node都集中到一个点
    */
//    cout<<"---Node refine to the local maximal intensity----"<<endl;
    double imgave,imgstd;
    V3DLONG total_size=sz[0]*sz[1]*sz[2];
    mean_and_std(inimg1d,total_size,imgave,imgstd);

    NeuronSWC out=snode;
    V3DLONG thisx,thisy,thisz;
    thisx=snode.x;    thisy=snode.y;    thisz=snode.z;

    V3DLONG sz01 = sz[0] * sz[1];
    out.level=inimg1d[(thisz*sz01 + thisy * sz[0] + thisx)];

    V3DLONG maxIntensity=inimg1d[(thisz*sz01 + thisy * sz[0] + thisx)];
    for(V3DLONG iz=((thisz-sqhere_radius)>=0)?(thisz-sqhere_radius):0;(iz<thisz+sqhere_radius+1)&&(iz<sz[2]);iz++){
        for( V3DLONG iy=((thisy-sqhere_radius)>=0)?(thisy-sqhere_radius):0;(iy<thisy+sqhere_radius+1)&&(iy<sz[1]);iy++){
            for(V3DLONG ix=((thisx-sqhere_radius)>=0)?(thisx-sqhere_radius):0;(ix<thisx+sqhere_radius+1)&&(ix<sz[0]);ix++){
                //get the angle of (snode,spnode) and (snode, inode)
                NeuronSWC inode;
                inode.x=ix; inode.y=iy;inode.z=iz;
                double searching_angle=getAngleofNodeVector(snode,spnode,inode);
                //get dist(inode,snode)
                double searching_dist=sqrt((snode.x-ix)*(snode.x-ix)+(snode.y-iy)*(snode.y-iy)
                                           +(snode.z-iz)*(snode.z-iz));
                if(searching_angle<MinRefineAngle&&searching_dist>searching_line_radius)
                    continue;
//                cout<<"angle="<<searching_angle<<endl;
                if(inimg1d[(iz*sz01 + iy * sz[0] + ix)]>maxIntensity)
                    maxIntensity=inimg1d[(iz*sz01 + iy * sz[0] + ix)];
            }
        }
    }
//    cout<<"raw level="<<out.level<<endl;
//    cout<<"max level="<<maxIntensity<<endl;
    if(maxIntensity<=out.level)
        return out;
    uint candicates_num=0;
    double x_candicates,y_candicates,z_candicates;
    x_candicates=y_candicates=z_candicates=0;
    for(V3DLONG iz=((thisz-sqhere_radius)>=0)?(thisz-sqhere_radius):0;(iz<thisz+sqhere_radius+1)&&(iz<sz[2]);iz++)
    {
        for( V3DLONG iy=((thisy-sqhere_radius)>=0)?(thisy-sqhere_radius):0;(iy<thisy+sqhere_radius+1)&&(iy<sz[1]);iy++)
        {
            for(V3DLONG ix=((thisx-sqhere_radius)>=0)?(thisx-sqhere_radius):0;(ix<thisx+sqhere_radius+1)&&(ix<sz[0]);ix++)
            {
                //get the angle of (snode,spnode) and (snode, inode)
                NeuronSWC inode;
                inode.x=ix; inode.y=iy;inode.z=iz;
                double searching_angle=getAngleofNodeVector(snode,spnode,inode);
                //get dist(inode,snode)
                double searching_dist=sqrt((snode.x-ix)*(snode.x-ix)+(snode.y-iy)*(snode.y-iy)
                                           +(snode.z-iz)*(snode.z-iz));
                if(searching_angle<MinRefineAngle&&searching_dist>searching_line_radius)
                    continue;

                V3DLONG pos = iz*sz01 + iy * sz[0] + ix;
                V3DLONG thisIntensity=inimg1d[pos];
                if(thisIntensity>=(0.8*maxIntensity)&&thisIntensity<=maxIntensity&&thisIntensity>out.level)
                {
                    x_candicates+=ix;
                    y_candicates+=iy;
                    z_candicates+=iz;
                    candicates_num+=1;
                }
            }
        }
    }

    out.x=float(x_candicates/candicates_num);
    out.y=float(y_candicates/candicates_num);
    out.z=float(z_candicates/candicates_num);
//    V3DLONG thisx,thisy,thisz;
    thisx=long(out.x);    thisy=long(out.y); thisz=long(out.z);
    out.level=inimg1d[(thisz*sz01 + thisy * sz[0] + thisx)];
//    cout<<"candicates:"<<candicates_num<<endl;
//    cout<<"refine level dist="<<out.level-snode.level<<endl;
    return out;
}

//bouton detection
void boutonDetection_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool in_terafly){
    /*peak detection algorithm on bouton detection,2021-04-20 ~ 2021-04-22
     * updated at 2021-07-31
    *Usage:
     * input file: <in_terafly_formatted_brain_highest_path>, <in_swc_file_path or in_eswc_file_path>
     * output file: <out_bouton_block>,<out_intensity_eswc_file_path>,<out_bouton_apo_file_path>,<out_bouton_eswc_file_path>
     * input para:
            * <Min_Interpolation_Pixels>,<Shift_Pixels>,
            * <min_bouton_dist>,<all_node or axonal part>,
            * <bouton_crop_size>,<mip>
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    string inimg_file,inswc_file;
    if(infiles.size()>=2) {
        inimg_file = infiles[0];
        inswc_file = infiles[1];
    }
    else {
        printHelp();
        return;
    }

    //read para list
    int Min_Interpolation_Pixels=(inparas.size()>=1)?atoi(inparas[0]):4;
    int Shift_Pixels=(inparas.size()>=2)?atoi(inparas[1]):2;
    int min_bouton_dist=(inparas.size()>=3)?atoi(inparas[2]):4;
    int allnode=(inparas.size()>=4)?atoi(inparas[3]):1;
    long crop_size=(inparas.size()>=5)?atoi(inparas[4]):64;
    int bkg_thre_bias=(inparas.size()>=6)?atoi(inparas[5]):20;

    //read input swc to neuron-tree
   NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
   if(!nt.listNeuron.size()) return;
    // 0. axonal part of the neuron-tree
   NeuronTree nt_p;
   if(!allnode)
   {
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            if(nt.listNeuron[i].type==2){
                nt_p.listNeuron.append(nt.listNeuron[i]);
                nt_p.hashNeuron.insert(nt.listNeuron[i].n,nt_p.listNeuron.size()-1);}
        }
   }
   else
       nt_p.deepCopy(nt);
   //1. interpolation
   NeuronTree nt_interpolated;       nt_interpolated=linearInterpolation(nt_p,Min_Interpolation_Pixels);

   if(in_terafly)
       boutonDetection_terafly_fun(callback,inimg_file,nt_interpolated,Shift_Pixels,crop_size,bkg_thre_bias);
   else
       boutonDetection_image_fun(callback,inimg_file,nt_interpolated,Shift_Pixels,bkg_thre_bias);
    cout<<"end of getting intensity and radius profile"<<endl;

    //3. neuron tree to segment list
    QList <CellAPO> apolist_init_boutons;        apolist_init_boutons=getBouton_1D_filter(nt_interpolated,1.3,0.05,3);

    //5. out
    QList <CellAPO> apo_boutons;apo_boutons.clear();
    apo_boutons=rmNearMarkers(apolist_init_boutons,min_bouton_dist);

    //bouton out to swc
    NeuronTree nt_bouton;        nt_bouton.copy(nt_interpolated);
    for(V3DLONG i=0;i<nt_bouton.listNeuron.size();i++)
    {
        for(V3DLONG b=0;b<apo_boutons.size();b++)
        {
            if(float(nt_bouton.listNeuron[i].x)==apo_boutons[b].x
                    &&float(nt_bouton.listNeuron[i].y)==apo_boutons[b].y
                    &&float(nt_bouton.listNeuron[i].z)==apo_boutons[b].z)
            {
                nt_bouton.listNeuron[i].type=BoutonType;
                nt_bouton.listNeuron[i].r=apo_boutons[b].volsize;
                nt_bouton.listNeuron[i].level=apo_boutons[b].intensity;
                nt_bouton.listNeuron[i].timestamp=apo_boutons[b].sdev;
                nt_bouton.listNeuron[i].tfresindex=apo_boutons[b].mass;
                break;
            }
        }
    }
    //crop 3D bouton block and mip image
    if(outfiles.size()==1)
    {
        //get out path
        string out_path=outfiles[0];
        int bouton_half_crop_size=(inparas.size()>=5)?atoi(inparas[4]):8;
        int get_mip=(inparas.size()>=6)?atoi(inparas[5]):0;
        getBoutonBlock(callback,inimg_file,apo_boutons,out_path,bouton_half_crop_size,get_mip);
        getBoutonBlockSWC(nt_bouton,out_path,bouton_half_crop_size);
    }
    else{
        //save to file: intensity_file, bouton_apo_file, bouton_eswc_file
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_intensity.eswc");
        string out_bouton_apo_file=(outfiles.size()>=2)?outfiles[1]:(inswc_file + "_bouton.apo");
        string out_bouton_swc_file=(outfiles.size()>=3)?outfiles[2]:(inswc_file + "_bouton.eswc");
//        string out_init_bouton_apo_file=(outfiles.size()>=4)?outfiles[3]:(inswc_file + "_initial_bouton.eswc");
        writeESWC_file(QString::fromStdString(out_swc_file),nt_interpolated);
        writeESWC_file(QString::fromStdString(out_bouton_swc_file),nt_bouton);
        writeAPO_file(QString::fromStdString(out_bouton_apo_file),apo_boutons);
//        writeAPO_file(QString::fromStdString(out_init_bouton_apo_file),apolist_init_boutons);
    }
}
void boutonDetection_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int refine_radius,long block_size,int bkg_bias)
{
    cout<<"Bouton detection under terafly datasets"<<endl;
    NeuronTree nt_raw; nt_raw.deepCopy(nt);
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = listNeuron.size();
    for (V3DLONG i=0;i<siz;i++)
         listNeuron[i].level=1;
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}
    int min_dist_to_block_edge=32;
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC ss = listNeuron[i];
        if(ss.level==1)
        {
            //get a block
            long start_x,start_y,start_z,end_x,end_y,end_z;

            start_x = ss.x - block_size; if(start_x<0) start_x = 0;
            end_x = ss.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0]-1;
            start_y =ss.y - block_size;if(start_y<0) start_y = 0;
            end_y = ss.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1]-1;
            start_z = ss.z - block_size;if(start_z<0) start_z = 0;
            end_z = ss.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2]-1;

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

            double imgave,imgstd;
            mean_and_std(inimg1d,pagesz,imgave,imgstd);
            imgstd=MAX(imgstd,10);
            double bkg_thresh= imgave+imgstd+bkg_bias;
            bkg_thresh=MAX(bkg_thresh,20);

            //for all the node inside this block
            for(V3DLONG j=0;j<siz;j++){
                NeuronSWC sj = listNeuron[j];
                if(sj.level==1&&
                        (sj.x-start_x)>=min_dist_to_block_edge&&(end_x-sj.x)>=min_dist_to_block_edge&&
                         (sj.y-start_y)>=min_dist_to_block_edge&&(end_y-sj.y)>=min_dist_to_block_edge&&
                         (sj.z-start_z)>=min_dist_to_block_edge&&(end_z-sj.z)>=min_dist_to_block_edge)
                {
                    V3DLONG thisx,thisy,thisz;
                    thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                    listNeuron[j].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                    //get node radius
                    NeuronSWC sr=sj;        sr.x=thisx; sr.y=thisy;sr.z=thisz;
                    listNeuron[j].r=radiusEstimation(inimg1d,in_sz,sr,4,bkg_thresh);
                    listNeuron[j].timestamp=bkg_thresh;
                    //refine the node to local maximal(surrounding area)
                    if(refine_radius>0)
                    {
                        NeuronSWC out=nodeRefine(inimg1d,sr,in_sz,refine_radius);
                        if(listNeuron[j].level+imgstd<out.level)
                        {
                            listNeuron[j].level=out.level;
                            listNeuron[j].x=float(start_x)+out.x;
                            listNeuron[j].y=float(start_y)+out.y;
                            listNeuron[j].z=float(start_z)+out.z;
                            listNeuron[j].r=radiusEstimation(inimg1d,in_sz,out,4,bkg_thresh);
                        }
                    }
                }
            }
            if(inimg1d) {delete []inimg1d; inimg1d=0;}
            if(in_sz) {delete []in_sz; in_sz=0;}
        }
    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}

}
void boutonDetection_image_fun(V3DPluginCallback2 &callback,string inimg_file, NeuronTree& nt,int refine_radius,int bkg_bias){
    cout<<"Get intensity of each node in a neuron tree"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash <V3DLONG, V3DLONG>  hashNeuron;    hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(listNeuron[i].n,i);

    //read image file
    unsigned char * inimg1d = 0;
    V3DLONG in_sz[4];int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return;
    //get the background threshold
    V3DLONG sz01 = in_sz[0] * in_sz[1];
    V3DLONG sz0 = in_sz[0];
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];

    double imgave,imgstd;
    mean_and_std(inimg1d,total_size,imgave,imgstd);
    imgstd=MAX(imgstd,10);
    double bkg_thresh= imgave+imgstd+bkg_bias;
    bkg_thresh=MAX(bkg_thresh,20);
    cout<<"bkg thresh="<<bkg_thresh<<endl;

    NeuronTree out_nt;
    double upfactor=4.0;
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        V3DLONG thisx,thisy,thisz;
        thisx=s.x;thisy=s.y;thisz=s.z;
        s.level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
        //get node radius
        s.r=radiusEstimation(inimg1d,in_sz,s,upfactor,bkg_thresh);
        s.timestamp=bkg_thresh;
        if(refine_radius>0)
        {
            NeuronSWC out=nodeRefine(inimg1d,listNeuron.at(i),in_sz,refine_radius);
            if(s.level+imgstd<out.level)
            {
                s.level=out.level;
                s.x=out.x;s.y=out.y;s.z=out.z;
                s.r=radiusEstimation(inimg1d,in_sz,out,upfactor,bkg_thresh);
            }
        }
        listNeuron[i].level=s.level;
        listNeuron[i].x=s.x;
        listNeuron[i].y=s.y;
        listNeuron[i].z=s.z;
        listNeuron[i].r=s.r;
        listNeuron[i].timestamp=s.timestamp;
    }
    if(inimg1d) {delete []inimg1d; inimg1d=0;}
}

bool teraImage_swc_crop(V3DPluginCallback2 &callback, string inimg, string inswc, string inapo,QString save_path, int cropx, int cropy, int cropz)
{
    cout<<"Base on the markers in inapo file, crop image block and swc block"<<endl;
    QList <CellAPO> apolist=readAPO_file(QString::fromStdString(inapo));
    if(apolist.size()==0)
        return false;

    //read neuron tree
    NeuronTree nt=readSWC_file(QString::fromStdString(inswc));
    //crop image block
    //read terafly  image
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(inimg,in_zz)) {cout<<"can't load terafly img"<<endl;return false;}
    cout<<"Input crop size x="<<cropx<<";y="<<cropy<<";z="<<cropz<<endl;
    for(V3DLONG i=0;i<apolist.size();i++)
    {
        CellAPO s = apolist.at(i);
        V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
        end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0]-1;
        start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
        end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1]-1;
        start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
        end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2]-1;
//        cout<<"crop size x="<<start_x<<";y="<<start_y<<";z="<<start_z<<endl;
        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = end_x-start_x+1;
        in_sz[1] = end_y-start_y+1;
        in_sz[2] = end_z-start_z+1;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return false;}
        im_cropped = callback.getSubVolumeTeraFly(inimg,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
        if(im_cropped==NULL)
            continue;
        //save cropped image
        QString tmpstr = "";
        tmpstr.append("_x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "Img"+tmpstr+".v3draw";
        QDir path(save_path);
        if(!path.exists()) { path.mkpath(save_path);}
        QString save_path_img =save_path+"/"+default_name;
        cout<<"save cropped image path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}

        //crop swc block
        if(nt.listNeuron.size())
        {
            NeuronTree out; out.listNeuron.clear();out.hashNeuron.clear();
            for(V3DLONG j=0;j<nt.listNeuron.size();j++)
            {
                NeuronSWC sn=nt.listNeuron.at(j);
                if(sn.x>=start_x&&sn.x<=end_x
                        &&sn.y>=start_y&&sn.y<=end_y
                        &&sn.z>=start_z&&sn.z<=end_z){
                    //shift coordinates
                    sn.x-=float(start_x);
                    sn.y-=float(start_y);
                    sn.z-=float(start_z);
                    out.listNeuron.append(sn);
                    out.hashNeuron.insert(sn.n,out.listNeuron.size()-1);
                }
            }
            //save to file
            default_name="swc"+QString::number(i)+tmpstr+".eswc";
            QString save_path_swc =save_path+"/"+default_name;
            writeESWC_file(save_path_swc,out);
        }
    }
}
QList <CellAPO> getBouton_1D_filter(NeuronTree nt,double radius_delta,double intensity_delta,double AXON_BACKBONE_RADIUS){
    /*In this version, i am thinking that:
     * 1. peak detection of radius feature
     * 2.
    */
    int MIN_PROCESSED_SEG_LEN=7;
    QList <CellAPO> apolist;apolist.clear();
    //4 feature extraction
    V_NeuronSWC_list nt_nslist=NeuronTree__2__V_NeuronSWC_list(nt);
    cout<<"segment size="<<nt_nslist.seg.size()<<endl;
    for(int i=0;i<nt_nslist.seg.size();i++)
    {
        V_NeuronSWC curseg=nt_nslist.seg.at(i);

        //4.1 peak detection of radius profile
        vector<double> seg_radius,seg_radius_feas;
        seg_radius.clear(); seg_radius_feas.clear();
        seg_radius=get_sorted_fea_of_seg(curseg,true);

        seg_radius_feas=mean_and_std_seg_fea(seg_radius);
        double radius_mean=seg_radius_feas[0];
        double radius_std=seg_radius_feas[1];
//        double radius_delta=1;
//        if(curseg.row.size()>MIN_PROCESSED_SEG_LEN)
//            radius_delta=1.3;
        if(radius_mean>AXON_BACKBONE_RADIUS)
            radius_delta=2;
        else if(radius_mean<2)
            radius_delta=1;
        double bouton_radius_thre=radius_mean*radius_delta+radius_std*0;
//        cout<<"Min bouton radius "<<bouton_radius_thre<<endl;
//        cout<<"Radius change std "<<radius_std<<endl;
        std::vector<int> outflag_radius=peaks_in_seg(seg_radius,1,0.05*radius_std);

        //4.2 peak detection of intensity profile
        vector<double> seg_levels,seg_levels_feas;
        seg_levels.clear(); seg_levels_feas.clear();
        seg_levels=get_sorted_fea_of_seg(curseg,false);

        seg_levels_feas=mean_and_std_seg_fea(seg_levels);
        double level_mean=seg_levels_feas[0];
        double level_std=seg_levels_feas[1];
        double bouton_intensity_thre=(curseg.row.size()>MIN_PROCESSED_SEG_LEN)
                ?(level_mean+intensity_delta*level_std):level_mean;
        std::vector<int> outflag_level=peaks_in_seg(seg_levels,0,0.05*level_std);
        //4.3 processed
        for(int io=0;io<curseg.row.size();io++){
            if(outflag_level[io]>0&&
                    outflag_radius[io]>0&&
                    curseg.row[io].r>bouton_radius_thre&&
                     curseg.row[io].r<6
                    &&curseg.row[io].level>bouton_intensity_thre){
                CellAPO apo;
                apo.n=apolist.size()+1;
                apo.x=curseg.row[io].x; apo.y=curseg.row[io].y; apo.z=curseg.row[io].z;
                apo.volsize=curseg.row[io].r;
                apo.intensity=curseg.row[io].level; //intensity value
                apo.color.r=0; apo.color.g=20; apo.color.b=200;
                apo.sdev=bouton_radius_thre;
                apo.mass=bouton_intensity_thre;
                apo.comment="bouton_site";
                apolist.push_back(apo);
            }
        }
    }
    cout<<"Bouton size: "<<apolist.size()<<endl;
    return apolist;
}
NeuronTree scale_registered_swc(NeuronTree nt,float xshift_pixels,float scale_xyz){
    V3DLONG siz=nt.listNeuron.size();
    NeuronTree out;    if(siz<=0){return out;}
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        s.x=(s.x-xshift_pixels)*scale_xyz;
        s.y*=scale_xyz;
        s.z*=scale_xyz;
        out.listNeuron.append(s);
        out.hashNeuron.insert(s.n,out.listNeuron.size()-1);
    }
    return out;
}
NeuronTree boutonSWC_internode_pruning(NeuronTree nt,int pruning_dist){
    /*1. if pruning_dist<=0, keep branch nodes, soma node, tip nodes and bouton nodes
     * 2. if pruning_dist>0, pruning the internode distance below pruning_dist and keep bouton nodes
    */
    V3DLONG siz=nt.listNeuron.size();
    NeuronTree out;    if(siz<=0){return out;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }
    vector<int> ntype(siz,0);    ntype=getNodeType(nt);
    vector<int> nprocessed(siz,0);

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(nprocessed[i]==1)
            continue;
        if(s.parent>0){
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=nt.listNeuron.at(pid);
            if(ntype.at(pid)==1&&sp.type!=BoutonType)
            {
                double ssp_dist=sqrt((s.x-sp.x)*(s.x-sp.x)+
                                      (s.y-sp.y)*(s.y-sp.y)+
                                      (s.z-sp.z)*(s.z-sp.z));
                if(ssp_dist<pruning_dist&&sp.parent>0){
                    nt.listNeuron[i].parent=sp.parent;
                    nprocessed[pid]=1;
                }
            }
        }
    }
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(nprocessed.at(i)==0){
            out.listNeuron.append(s);
            out.hashNeuron.insert(s.n,out.listNeuron.size()-1);
        }
    }
    cout<<"pruning size="<<(nt.listNeuron.size()-out.listNeuron.size())<<endl;
    return reindexNT(out);
}
NeuronTree reindexNT(NeuronTree nt)
{
    NeuronTree nt_out_reindex;nt_out_reindex.listNeuron.clear();nt_out_reindex.hashNeuron.clear();
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        s.pn=(s.pn<0)?s.pn:(nt.hashNeuron.value(s.pn)+1);
//        s.n=nt.hashNeuron.value(s.n)+1;
        s.n=i+1;
        nt_out_reindex.listNeuron.append(s);
        nt_out_reindex.hashNeuron.insert(s.n,nt_out_reindex.listNeuron.size()-1);
    }
   return nt_out_reindex;
}
void getBoutonBlock_inImg(V3DPluginCallback2 &callback,string inimg_file,QList <CellAPO> apolist,string outpath,int block_size)
{
    /*crop 3d image-block
     * get mip img
    */
    //read image file
    unsigned char * inimg1d = 0;V3DLONG in_zz[4];int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_zz, datatype)) return;

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
    if(inimg1d) {delete []inimg1d; inimg1d=0;}
}
void getBoutonBlockSWC(NeuronTree nt,string outpath,int half_block_size)
{
    cout<<"Welcome into bouton detection: bouton swc crop part"<<endl;
    cout<<"get bouton swc file and crop block-swc out"<<endl;
}
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int half_block_size,uint mip_flag)
{
    cout<<"Welcome into bouton detection: image crop part"<<endl;
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
        start_x = s.x - half_block_size; if(start_x<0) start_x = 0;
        end_x = s.x + half_block_size; if(end_x > in_zz[0]) end_x = in_zz[0]-1;
        start_y =s.y - half_block_size;if(start_y<0) start_y = 0;
        end_y = s.y + half_block_size;if(end_y > in_zz[1]) end_y = in_zz[1]-1;
        start_z = s.z - half_block_size;if(start_z<0) start_z = 0;
        end_z = s.z + half_block_size;if(end_z > in_zz[2]) end_z = in_zz[2]-1;

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
void maskImg(V3DPluginCallback2 &callback, unsigned char *&inimg1d, QString outpath, long in_sz[], NeuronTree &nt, int maskRadius,int erosion_kernel_size)
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
//    if(erosion_kernel_size)
//        erosionImg(im_transfer,in_sz,erosion_kernel_size);
    //save img
    simple_saveimage_wrapper(callback, outpath.toStdString().c_str(),(unsigned char *)im_transfer,in_sz,1);
    //release pointer
     if(im_transfer) {delete []im_transfer; im_transfer=0;}
}
vector<int> getNodeType(NeuronTree nt)
{
    /*soma: ntype>=3, branch: ntype=2; tip: ntype=0; internodes: ntype=1*/
    /*1. get tip, branch and soma nodes;
    */
    V3DLONG siz=nt.listNeuron.size();
    vector<int> ntype(siz,0);
    if(!siz)
        return ntype;
    /*1. get the index of nt:
                                        * swc_n -> index */
    QHash <int, int>  hashNeuron;hashNeuron.clear();
    V3DLONG somaid=1;
    for (V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        if(nt.listNeuron[i].type==1&&nt.listNeuron[i].pn<0)
            somaid=i;
    }
    // 2. get node type: index -> node_type


    ntype[somaid]=2;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(s.pn&&hashNeuron.contains(s.pn))
        {
            V3DLONG spn_id=hashNeuron.value(s.pn);
            ntype[spn_id]+=1;
        }
    }
    return ntype;
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
void getNTRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronTree& nt, double bkg_thresh)
{
    cout<<"Get neuron tree radius from XY plane"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    for(V3DLONG it=0;it<siz;it++)
        listNeuron[it].r=getNodeRadius_XY(inimg1d,in_sz,listNeuron[it],bkg_thresh);
}
double getNodeRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronSWC s, double bkg_thresh)
{
//    cout<<"Get node radius from XY plane, bkg_threshold="<<bkg_thresh<<endl;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    double max_r = in_sz[0]/2;
    if (max_r > in_sz[1]/2) max_r = in_sz[1]/2;

    int thisx,thisy,thisz;
    thisx=s.x;        thisy=s.y;        thisz=s.z;
    double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir++)
    {
        total_num = background_num = 0;
        double dz, dy, dx;
        double zlower = 0, zupper = 0;
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
    return ir;
}
double getNodeRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronSWC s, double bkg_thresh,int z_half_win_size)
{
//    cout<<"Get node radius from XY plane, bkg_threshold="<<bkg_thresh<<endl;
    //get mip
    V3DLONG N = in_sz[0];    V3DLONG M = in_sz[1];    V3DLONG P = in_sz[2];
    V3DLONG mip_sz[4];
    mip_sz[0] = N;    mip_sz[1] = M;    mip_sz[2] = 1;    mip_sz[3] = 1;

//    int z_half_win_size=4;
//    cout<<"size x="<<N<<" , size y= "<<M<<endl;
    V3DLONG pagesz_mip = mip_sz[0]*mip_sz[1]*mip_sz[2];
    unsigned char *image_mip=0;
    try {image_mip = new unsigned char [pagesz_mip];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

    for(V3DLONG ix = 0; ix < N; ix++)
    {
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            int max_mip = 0;
            for(V3DLONG iz = (s.z-z_half_win_size>=0)?(s.z-z_half_win_size):0;
                iz < ((s.z+z_half_win_size<=P)?(s.z+z_half_win_size):P); iz++)
            {
                if(inimg1d[N*M*iz + N*iy + ix] >= max_mip)
                {
                    image_mip[iy*N + ix] = inimg1d[N*M*iz + N*iy + ix];
                    max_mip = inimg1d[N*M*iz + N*iy + ix];
                }
            }
        }
    }

    long sz0 = in_sz[0];
    double max_r = in_sz[0]/2;
    if (max_r > in_sz[1]/2) max_r = in_sz[1]/2;

    int thisx,thisy,thisz;
    thisx=s.x;        thisy=s.y;        thisz=s.z;
    double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir++)
    {
        total_num = background_num = 0;
        double dz, dy, dx;
        double zlower = 0, zupper = 0;
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
//                        V3DLONG k = thisz+dz;   if (k<0 || k>=in_sz[2]) goto end2;

                        if (image_mip[ j * sz0 + i] <= bkg_thresh)
                        {
                            background_num++;
                            if ((background_num/total_num) > 0.001) goto end2;
                        }
                    }
                }
    }
end2:
    return ir;
}
double radiusEstimation(unsigned char *&inimg1d, long in_zz[], NeuronSWC s, double dfactor, double bkg_thresh){
    /*1. get a block center at node s (block size=32x32x32)
     * 2. according to dfactor, upsample the above block (default: 128x128x32)
     * 3. mip of the upsampled block
     * 4. radius estimation at 2D plane
     * 5. scale down the radius
    */
    NeuronSWC dynamic_s=s;
    //1. center_block cropped
    int cropped_block_size=16; int zcropped_block_size=8;
    long start_x,start_y,start_z,end_x,end_y,end_z;
    start_x = s.x - cropped_block_size; if(start_x<0) {start_x = 0;}
    end_x = s.x + cropped_block_size; if(end_x >= in_zz[0]) {end_x = in_zz[0]-1;}
    start_y =s.y - cropped_block_size;if(start_y<0) {start_y = 0;}
    end_y = s.y + cropped_block_size;if(end_y >= in_zz[1]) {end_y = in_zz[1]-1;}
    start_z = s.z - zcropped_block_size;if(start_z<0) {start_z = 0;}
    end_z = s.z + zcropped_block_size;if(end_z >= in_zz[2]) {end_z = in_zz[2]-1;}

    V3DLONG crop_sz[4];
    crop_sz[0] =end_x-start_x+1;
    crop_sz[1] = end_y-start_y+1;
    crop_sz[2] = end_z-start_z+1;
    crop_sz[3]=1;
    long sz01 = crop_sz[0] * crop_sz[1];
    long sz0 = crop_sz[0];
    unsigned char * im_cropped = 0;
    V3DLONG pagesz= crop_sz[0] * crop_sz[1]*crop_sz[2];
    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for cropping."<<endl; return 0.0;}

    for(V3DLONG iz=0;iz<crop_sz[2];iz++)
    {
        for(V3DLONG iy=0;iy<crop_sz[1];iy++)
        {
            for(V3DLONG ix=0;ix<crop_sz[0];ix++)
            {
                im_cropped[iz * sz01 + iy * sz0 + ix]=
                        inimg1d[(start_z+iz) * in_zz[0]*in_zz[1]
                        +(start_y+ iy) *in_zz[0] + (start_x+ix)];
            }
        }
    }
    dynamic_s.x-=start_x; dynamic_s.y-=start_y; dynamic_s.z-=start_z;

    //2. upsample center_block
    V3DLONG pagesz_resample = (V3DLONG)(ceil(dfactor*dfactor*1*pagesz));
    unsigned char * image_resampled = 0;
    try {image_resampled = new unsigned char [pagesz_resample];/*cout<<"cropped"<<endl;*/}
    catch(...)  {cout<<"cannot allocate memory for upsampling."<<endl; return 0.0;}


    V3DLONG upsampled_sz[4];
    upsampled_sz[0] = long(dfactor)*crop_sz[0];
    upsampled_sz[1] = long(dfactor)*crop_sz[1];
    upsampled_sz[2] =crop_sz[2];

    double upfactor[3];
    upfactor[0]=dfactor;upfactor[1]=dfactor;upfactor[2]=1;

    upsampleImage(im_cropped,image_resampled,crop_sz,upsampled_sz,upfactor);
    dynamic_s.x*=dfactor; dynamic_s.y*=dfactor; dynamic_s.z*=1;
    //release center_block
    if(im_cropped) {delete []im_cropped; im_cropped = 0;}

    //3. MIP of upsampled block
    V3DLONG mip_sz[4];
    mip_sz[0] = upsampled_sz[0];    mip_sz[1] = upsampled_sz[1];    mip_sz[2] = 1;    mip_sz[3] = 1;
    V3DLONG pagesz_mip = mip_sz[0]*mip_sz[1]*mip_sz[2];
    unsigned char *image_mip=0;
    try {image_mip = new unsigned char [pagesz_mip];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return 0.0;}

    for(V3DLONG ix = 0; ix < mip_sz[0]; ix++)
    {
        for(V3DLONG iy = 0; iy < mip_sz[1]; iy++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < upsampled_sz[2]; iz++)
            {
                if(image_resampled[upsampled_sz[0]*upsampled_sz[1]*iz + upsampled_sz[0]*iy + ix] >= max_mip)
                {
                    image_mip[iy*mip_sz[0] + ix] = image_resampled[upsampled_sz[0]*upsampled_sz[1]*iz + upsampled_sz[0]*iy + ix];
                    max_mip = image_resampled[upsampled_sz[0]*upsampled_sz[1]*iz + upsampled_sz[0]*iy + ix];
                }
            }
        }
    }
    dynamic_s.z=0.0;
    //release upsampled image pointer
    if(image_resampled) {delete []image_resampled; image_resampled = 0;}

    //4. radius estimation
    double upRadius=getNodeRadius_XY(image_mip,mip_sz,dynamic_s,bkg_thresh);
    if(image_mip) {delete []image_mip; image_mip = 0;}
//    cout<<"radius estimation finished"<<endl;
    double realRadius=upRadius/dfactor;
    return realRadius;
}
double getNodeRadius(unsigned char *&inimg1d, long in_sz[], NeuronSWC s,double bkg_thresh)
{
//    cout<<"Get node radius"<<endl;

    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    double max_r = in_sz[0]/2;
    if (max_r > in_sz[1]/2) max_r = in_sz[1]/2;
    if (max_r > (in_sz[2])/2) max_r = (in_sz[2])/2;

    long thisx,thisy,thisz;
    thisx=s.x;    thisy=s.y;    thisz=s.z;
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
    return ir;
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
///radiusfea=true, for radius feature; else for level feature
std::vector<double> get_sorted_fea_of_seg(V_NeuronSWC inseg,bool radiusfea)
{
    /*start from tip to root*/
    vector<double> seg_fea_list; seg_fea_list.clear();
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
        if(radiusfea)
            seg_fea_list.push_back((inseg.row.at(cur_id).r));
        else
            seg_fea_list.push_back((inseg.row.at(cur_id).level));
        if(nlist.contains(long(inseg.row.at(cur_id).parent)))
            cur_id=nlist.value(long(inseg.row.at(cur_id).parent));
        else
            break;
    }
    return seg_fea_list;
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
std::vector<double> mean_and_std_seg_fea(std::vector<double> input)
{
    vector<double> out(2, 0.0);
    double fea_mean=0.0;
    for(V3DLONG i=0;i<input.size();i++)
        fea_mean+=input[i];
    fea_mean/=input.size();
    out[0]=fea_mean;

    //standard
    double var_fea=0;
    for (V3DLONG i=0;i<input.size();i++)
        var_fea=var_fea+(input[i]-fea_mean)*(input[i]-fea_mean);
    var_fea/=(input.size()-1);
    double std_fea=sqrt(var_fea);
    out[1]=std_fea;

    return out;
}
std::vector<int> peaks_in_seg(std::vector<double> input,int isRadius_fea, float delta)
{
    /*The first argument is the vector to examine,
     * and the second is the peak threshold:
                * We require a difference of at least 0.5 between a peak and its surrounding in order to declare it as a peak.
      * Same goes with valleys.
      * Note, peak detection are used. this function is also usable for detecting valleys.
    */
    std::vector<int> signals_flag_highpeaks(input.size(), 0.0);
    std::vector<int> signals_flag_lowpeaks(input.size(), 0.0);
    double max_intensity=input.at(0);
    double min_intensity=input.at(0);
    bool lookformax=true;
    V3DLONG maxpos=0;V3DLONG minpos=0;
    for(V3DLONG i=1;i<input.size();i++)
    {
        double this_intensity=input.at(i);
        if(lookformax)
        {
            if(this_intensity>max_intensity) {
                max_intensity=this_intensity;
                maxpos=i;
            }
            if(this_intensity<max_intensity-delta)
            {
                signals_flag_highpeaks[maxpos]=1;
                minpos=i;
                min_intensity=this_intensity;
                lookformax=false;
            }
        }
        else
        {
            if(this_intensity<min_intensity)  {
                min_intensity=this_intensity;
                minpos=i;
            }
            if(this_intensity>min_intensity+delta){
                signals_flag_lowpeaks[minpos]=1;
                max_intensity=this_intensity;
                maxpos=i;
                lookformax=true;
            }
        }
    }
    //peak-brother-checking
    if(isRadius_fea){
        std::vector<int> final_peaks(input.size(), 0.0);
        for(V3DLONG i=0;i<input.size();i++){
            if(signals_flag_highpeaks.at(i)==1){
                V3DLONG bro_index=i;
                while(input.at(bro_index)==input.at(i))
                {
                    final_peaks[bro_index]=1;
                    bro_index++;
                }
            }
        }
        return final_peaks;
    }
    else
        return signals_flag_highpeaks;

}

bool upsampleImage(unsigned char * & inimg1d,unsigned char * & outimg1d,V3DLONG *szin, V3DLONG *szout, double *dfactor){
    return upsample3dvol(outimg1d,inimg1d,szout,szin,dfactor);
}
template <class T> bool upsample3dvol(T *outdata, T *indata, V3DLONG *szout, V3DLONG *szin, double *dfactor)
{

    szout[0] = (V3DLONG)(ceil(dfactor[0]*szin[0]));
    szout[1] = (V3DLONG)(ceil(dfactor[1]*szin[1]));
    szout[2] = (V3DLONG)(ceil(dfactor[2]*szin[2]));


    V3DLONG totallen = szout[0] * szout[1] * szout[2];
    Coord3D * c = new Coord3D [totallen];
    T * v = new T [totallen];

    if (!c || !v)
    {
        fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
        if (c) {delete []c; c=0;}
        if (v) {delete []v; v=0;}
        return false;
    }

    //computation
    V3DLONG i,j,k, ind;
    ind=0;

    for (k=0;k<szout[2];k++)
    {
        for (j=0;j<szout[1];j++)
        {
            for (i=0;i<szout[0];i++)
            {
                c[ind].x = double(i)/dfactor[0];
                c[ind].y = double(j)/dfactor[1];
                c[ind].z = double(k)/dfactor[2];
                v[ind] = -1; //set as a special value
                ind++;
            }
        }
    }

    interpolate_coord_linear(v, c, totallen, indata, szin[0], szin[1], szin[2], 0, szin[0]-1, 0, szin[1]-1, 0, szin[2]-1);

    ind=0;
    for (i=0; i<totallen; i++)
        outdata[i] = (T)(v[i]);

    // free temporary memory
    if (c) {delete []c; c=0;}
    if (v) {delete []v; v=0;}
    return true;
}
template <class T> bool interpolate_coord_linear(T * interpolatedVal, Coord3D *c, V3DLONG numCoord,
                       T *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
                       V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2)
{
    //check if parameters are correct
    if (!interpolatedVal || !c || numCoord<=0 ||
        !templateVol3d || tsz0<=0 || tsz1<=0 || tsz2<=0 ||
        tlow0<0 || tlow0>=tsz0 || tup0<0 || tup0>=tsz0 || tlow0>tup0 ||
        tlow1<0 || tlow1>=tsz1 || tup1<0 || tup1>=tsz1 || tlow1>tup1 ||
        tlow2<0 || tlow2>=tsz2 || tup2<0 || tup2>=tsz2 || tlow2>tup2)
    {
        fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
        return false;
    }

    // generate nearest interpolation

    //V3DLONG i,j,k;

    double curpx, curpy, curpz;
    V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;

    for (V3DLONG ipt=0;ipt<numCoord;ipt++)
    {
        curpx = c[ipt].x; curpx=(curpx<tlow0)?tlow0:curpx; curpx=(curpx>tup0)?tup0:curpx;
//#ifndef POSITIVE_Y_COORDINATE
//		curpy = tsz1-1-c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; //080111: should I use this or the other way around
//#else
        curpy = c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; //080114
//#endif
        curpz = c[ipt].z; curpz=(curpz<tlow2)?tlow2:curpz; curpz=(curpz>tup2)?tup2:curpz;

        cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
        cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
        cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));

        if (cpz0==cpz1)
        {
            if (cpy0==cpy1)
            {
                if (cpx0==cpx1)
                {
                    interpolatedVal[ipt] = (T)(templateVol3d[cpz0][cpy0][cpx0]);
                }
                else
                {
                    double w0x0y0z = (cpx1-curpx);
                    double w1x0y0z = (curpx-cpx0);
                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
                                                     w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]));
                }
            }
            else
            {
                if (cpx0==cpx1)
                {
                    double w0x0y0z = (cpy1-curpy);
                    double w0x1y0z = (curpy-cpy0);
                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
                                                     w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]));
                }
                else
                {
                    double w0x0y0z = (cpx1-curpx)*(cpy1-curpy);
                    double w0x1y0z = (cpx1-curpx)*(curpy-cpy0);
                    double w1x0y0z = (curpx-cpx0)*(cpy1-curpy);
                    double w1x1y0z = (curpx-cpx0)*(curpy-cpy0);
                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
                                                     w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) +
                                                     w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) +
                                                     w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]));
                }
            }
        }
        else
        {
            if (cpy0==cpy1)
            {
                if (cpx0==cpx1)
                {
                    double w0x0y0z = (cpz1-curpz);
                    double w0x0y1z = (curpz-cpz0);

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]));
                }
                else
                {
//					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
//					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);
//
//					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
//					double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

                    // FL 20120410
                    double w0x0y0z = (cpx1-curpx)*(cpz1-curpz);
                    double w0x0y1z = (cpx1-curpx)*(curpz-cpz0);

                    double w1x0y0z = (curpx-cpx0)*(cpz1-curpz);
                    double w1x0y1z = (curpx-cpx0)*(curpz-cpz0);


                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
                                                     w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]));
                }
            }
            else
            {
                if (cpx0==cpx1)
                {
//					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
//					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);
//
//					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
//					double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

                    //FL 20120410
                    double w0x0y0z = (cpy1-curpy)*(cpz1-curpz);
                    double w0x0y1z = (cpy1-curpy)*(curpz-cpz0);

                    double w0x1y0z = (curpy-cpy0)*(cpz1-curpz);
                    double w0x1y1z = (curpy-cpy0)*(curpz-cpz0);


                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
                                                     w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]));
                }
                else
                {
                    double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
                    double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);

                    double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
                    double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

                    double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
                    double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

                    double w1x1y0z = (curpx-cpx0)*(curpy-cpy0)*(cpz1-curpz);
                    double w1x1y1z = (curpx-cpx0)*(curpy-cpy0)*(curpz-cpz0);

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
                                                     w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]) +
                                                     w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]) +
                                                     w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]) + w1x1y1z * double(templateVol3d[cpz1][cpy1][cpx1]) );
                }
            }
        }

    }

    return true;
}
template <class T> bool interpolate_coord_linear(T * interpolatedVal, Coord3D *c, V3DLONG numCoord,
                                                 T * templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
                                                 V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2)
{
    //check if parameters are correct
    if (!interpolatedVal || !c || numCoord<=0 ||
        !templateVol3d || tsz0<=0 || tsz1<=0 || tsz2<=0 ||
        tlow0<0 || tlow0>=tsz0 || tup0<0 || tup0>=tsz0 || tlow0>tup0 ||
        tlow1<0 || tlow1>=tsz1 || tup1<0 || tup1>=tsz1 || tlow1>tup1 ||
        tlow2<0 || tlow2>=tsz2 || tup2<0 || tup2>=tsz2 || tlow2>tup2)
    {
        fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
        return false;
    }

    // generate nearest interpolation

    //V3DLONG i,j,k;

    double curpx, curpy, curpz;
    V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;

    long tsz01 = tsz0*tsz1;

    for (V3DLONG ipt=0;ipt<numCoord;ipt++)
    {
        curpx = c[ipt].x; curpx=(curpx<tlow0)?tlow0:curpx; curpx=(curpx>tup0)?tup0:curpx;
//#ifndef POSITIVE_Y_COORDINATE
//		curpy = tsz1-1-c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; //080111: should I use this or the other way around
//#else
        curpy = c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; //080114
//#endif
        curpz = c[ipt].z; curpz=(curpz<tlow2)?tlow2:curpz; curpz=(curpz>tup2)?tup2:curpz;

        cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
        cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
        cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));

        if (cpz0==cpz1)
        {
            if (cpy0==cpy1)
            {
                if (cpx0==cpx1)
                {
                    long ind = cpz0*tsz01 + cpy0*tsz0 + cpx0;
//					interpolatedVal[ipt] = (T)(templateVol3d[cpz0][cpy0][cpx0]);
                    interpolatedVal[ipt] = (T)(templateVol3d[ind]);

                }
                else
                {
                    double w0x0y0z = (cpx1-curpx);
                    double w1x0y0z = (curpx-cpx0);

//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
//                                               w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]));

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz0*tsz01 + cpy0*tsz0 + cpx1;

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) +
                                               w1x0y0z * double(templateVol3d[ind2]));

                }
            }
            else
            {
                if (cpx0==cpx1)
                {
                    double w0x0y0z = (cpy1-curpy);
                    double w0x1y0z = (curpy-cpy0);

//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
//                                               w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]));

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz0*tsz01 + cpy1*tsz0 + cpx0;

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) +
                                               w0x1y0z * double(templateVol3d[ind2]));

                }
                else
                {
                    double w0x0y0z = (cpx1-curpx)*(cpy1-curpy);
                    double w0x1y0z = (cpx1-curpx)*(curpy-cpy0);
                    double w1x0y0z = (curpx-cpx0)*(cpy1-curpy);
                    double w1x1y0z = (curpx-cpx0)*(curpy-cpy0);

//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
//                                               w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) +
//                                               w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) +
//                                               w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]));

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz0*tsz01 + cpy1*tsz0 + cpx0;
                    long ind3 = cpz0*tsz01 + cpy0*tsz0 + cpx1;
                    long ind4 = cpz0*tsz01 + cpy1*tsz0 + cpx1;

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) +
                                               w0x1y0z * double(templateVol3d[ind2]) +
                                               w1x0y0z * double(templateVol3d[ind3]) +
                                               w1x1y0z * double(templateVol3d[ind4]));


                }
            }
        }
        else
        {
            if (cpy0==cpy1)
            {
                if (cpx0==cpx1)
                {
                    double w0x0y0z = (cpz1-curpz);
                    double w0x0y1z = (curpz-cpz0);

//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]));

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz1*tsz01 + cpy0*tsz0 + cpx0;

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) + w0x0y1z * double(templateVol3d[ind2]));

                }
                else
                {
//					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
//					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);
//
//					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
//					double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
//                                               w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]));

                    // FL 20120410
                    double w0x0y0z = (cpx1-curpx)*(cpz1-curpz);
                    double w0x0y1z = (cpx1-curpx)*(curpz-cpz0);

                    double w1x0y0z = (curpx-cpx0)*(cpz1-curpz);
                    double w1x0y1z = (curpx-cpx0)*(curpz-cpz0);

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz1*tsz01 + cpy0*tsz0 + cpx0;
                    long ind3 = cpz0*tsz01 + cpy0*tsz0 + cpx1;
                    long ind4 = cpz1*tsz01 + cpy0*tsz0 + cpx1;

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) + w0x0y1z * double(templateVol3d[ind2]) +
                                               w1x0y0z * double(templateVol3d[ind3]) + w1x0y1z * double(templateVol3d[ind4]));

                }
            }
            else
            {
                if (cpx0==cpx1)
                {
//					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
//					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);
//
//					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
//					double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
//                                               w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]));

                    //FL 20120410
                    double w0x0y0z = (cpy1-curpy)*(cpz1-curpz);
                    double w0x0y1z = (cpy1-curpy)*(curpz-cpz0);

                    double w0x1y0z = (curpy-cpy0)*(cpz1-curpz);
                    double w0x1y1z = (curpy-cpy0)*(curpz-cpz0);

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz1*tsz01 + cpy0*tsz0 + cpx0;
                    long ind3 = cpz0*tsz01 + cpy1*tsz0 + cpx0;
                    long ind4 = cpz1*tsz01 + cpy1*tsz0 + cpx0;

                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) + w0x0y1z * double(templateVol3d[ind2]) +
                                               w0x1y0z * double(templateVol3d[ind3]) + w0x1y1z * double(templateVol3d[ind4]));

                }
                else
                {
                    double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
                    double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);

                    double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
                    double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

                    double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
                    double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

                    double w1x1y0z = (curpx-cpx0)*(curpy-cpy0)*(cpz1-curpz);
                    double w1x1y1z = (curpx-cpx0)*(curpy-cpy0)*(curpz-cpz0);


//					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
//                                               w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]) +
//                                               w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]) +
//                                               w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]) + w1x1y1z * double(templateVol3d[cpz1][cpy1][cpx1]) );

                    long ind1 = cpz0*tsz01 + cpy0*tsz0 + cpx0;
                    long ind2 = cpz1*tsz01 + cpy0*tsz0 + cpx0;
                    long ind3 = cpz0*tsz01 + cpy1*tsz0 + cpx0;
                    long ind4 = cpz1*tsz01 + cpy1*tsz0 + cpx0;
                    long ind5 = cpz0*tsz01 + cpy0*tsz0 + cpx1;
                    long ind6 = cpz1*tsz01 + cpy0*tsz0 + cpx1;
                    long ind7 = cpz0*tsz01 + cpy1*tsz0 + cpx1;
                    long ind8 = cpz1*tsz01 + cpy1*tsz0 + cpx1;


                    interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[ind1]) + w0x0y1z * double(templateVol3d[ind2]) +
                                               w0x1y0z * double(templateVol3d[ind3]) + w0x1y1z * double(templateVol3d[ind4]) +
                                               w1x0y0z * double(templateVol3d[ind5]) + w1x0y1z * double(templateVol3d[ind6]) +
                                               w1x1y0z * double(templateVol3d[ind7]) + w1x1y1z * double(templateVol3d[ind8]));


                }
            }
        }

    }

    return true;
}
