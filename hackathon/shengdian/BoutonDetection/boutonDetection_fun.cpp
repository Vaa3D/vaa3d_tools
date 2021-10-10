#include "boutonDetection_fun.h"
//bouton detection all in one
void boutonDetection_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool in_terafly){
    /*updated at 2021-08-20
     * 1. refinement
     * 2. profile
     * 3. initial boutons
     * 4. pruning
     * 5. feature
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
    int bkg_thre_bias=(inparas.size()>=1)?atoi(inparas[0]):15;
    int refine_radius=(inparas.size()>=2)?atoi(inparas[1]):3;
    int Shift_Pixels=(inparas.size()>=3)?atoi(inparas[2]):2;

    double radius_delta=(inparas.size()>=4)?atoi(inparas[3]):1.3;
    double intensity_delta=(inparas.size()>=5)?atoi(inparas[4]):1;
    double axon_trunk_radius=(inparas.size()>=6)?atof(inparas[5]):2.5;

    int allnode=(inparas.size()>=7)?atoi(inparas[6]):1;
    long half_crop_size=(inparas.size()>=8)?atoi(inparas[7]):128;
    double block_upsample_factor=(inparas.size()>=9)?atof(inparas[8]):4.0;

    float min_bouton_dist=(inparas.size()>=10)?atof(inparas[9]):4.0;
    float pruning_internode_thre=(inparas.size()>=11)?atof(inparas[10]):5.0;
    float pruning_tip_thre=(inparas.size()>=12)?atof(inparas[11]):5.0;
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
   //1. refinement
   if(in_terafly)
       refinement_terafly_fun(callback,inimg_file,nt_p,2,refine_radius,half_crop_size,Shift_Pixels);
   else
       refinement_Image_fun(callback,inimg_file,nt_p,2,refine_radius,Shift_Pixels);
   NeuronTree nt_pruning=internode_pruning(nt_p);
   NeuronTree nt_interpolated=node_interpolation(nt_pruning,3);

   //2. profile
   if(in_terafly)
       swc_profile_terafly_fun(callback,inimg_file,nt_interpolated,half_crop_size,bkg_thre_bias,block_upsample_factor);
   else
       swc_profile_image_fun(callback,inimg_file,nt_interpolated,bkg_thre_bias,block_upsample_factor);
    cout<<"end of getting intensity and radius profile"<<endl;

    //3. get initial bouton-sites and out to list of NeuronSWC
     QList <AxonalBouton> init_bouton_sites=boutonFilter_fun(nt_interpolated,radius_delta,intensity_delta,axon_trunk_radius);
     QString outpath=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(QString::fromStdString(inswc_file)).path());
     if(!init_bouton_sites.size()){return;}

     //4. filter and pruning
     //map bouton-sites to swc
     NeuronTree nt_bouton; nt_bouton.deepCopy(nt_interpolated);
     map_bouton_2_neuronTree(nt_bouton,init_bouton_sites);
     //get bouton-density and remove boutons that are two close to its neighbor bouton-site
     //also pruning bouton appeared at branch point
     nearBouton_pruning(nt_bouton,min_bouton_dist);
     NeuronTree nt_bouton_internode_pruning=boutonSWC_internode_pruning(nt_bouton,pruning_internode_thre);
     NeuronTree nt_bouton_tip_pruning=tipNode_pruning(nt_bouton_internode_pruning,pruning_tip_thre);

     nt_bouton.listNeuron.clear();nt_bouton.hashNeuron.clear();
     nt_bouton.deepCopy(nt_bouton_tip_pruning);
     nt_bouton_tip_pruning.listNeuron.clear();nt_bouton_tip_pruning.hashNeuron.clear();
     nt_bouton_internode_pruning.listNeuron.clear();nt_bouton_internode_pruning.hashNeuron.clear();

     //5. feature
     //1.
     V3DLONG siz=nt_bouton.listNeuron.size();
     vector<int> norder(siz,0);
     if(!getNodeOrder(nt_bouton,norder)) return;
     for(V3DLONG i=0;i<siz;i++)
         nt_bouton.listNeuron[i].level=norder.at(i);
     //2.
     boutonType_label(nt_bouton);
     //3. append bouton_density feature to fea_val_list
     boutonDesity_computing(nt_bouton);
     //4. distance
     dist_to_soma(nt_bouton);

     QString out_swc_file=outpath+"/"+QFileInfo(QString::fromStdString(inswc_file)).fileName()+"_profiled.eswc";
     QString out_bouton_swc_file=outpath+"/"+QFileInfo(QString::fromStdString(inswc_file)).fileName()+"_bouton.eswc";
     writeESWC_file(out_swc_file,nt_interpolated);
     writeESWC_file(out_bouton_swc_file,nt_bouton);
}
//preprocess
void preprocess_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    NeuronTree nt = readSWC_file(inswc_file);
    if(!nt.listNeuron.size()) return;

    bool single_tree=(((inparas.size()>=1)?atoi(inparas[0]):1)>0)?true:false;
    int tip_br_thre=/*(inparas.size()>=1)?atoi(inparas[0]):*/6;
    float internode_thre=/*(inparas.size()>=2)?atof(inparas[1]):*/1.5;
    int resample_pixels=(inparas.size()>=2)?atoi(inparas[1]):5;
    //
    QString out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file+"_preprocessed.eswc");

    if(single_tree){
        /*0. only one -1
         * 1. precessing of the multiple bifurcation, mostly three bifurcation
         * 2. pruning small feak tip-branches
         * 3. linear interpolation of neuron tree
         * 4. reindex of neuron tree
        */
        NeuronTree nt_4,nt_3, nt_out;
        if(!three_bifurcation_processing(nt)){ cout<<"Error in bifurcation processing"<<endl;return;}
        NeuronTree nt_2=tip_branch_pruning(nt,tip_br_thre); //remove tip-branches which length are below 4 pixels
        nt_3=internode_pruning(nt_2,internode_thre);
        nt_2.listNeuron.clear(); nt_2.hashNeuron.clear();

        if(resample_pixels)
            nt_4=node_interpolation(nt_3,resample_pixels);
        else
            nt_4.deepCopy(nt_3);
        nt_out=reindexNT(nt_4);
        writeESWC_file(out_swc_file,nt_out);
    }
    else{
         NeuronTree nt_out=preprocess_simple(nt);
         writeESWC_file(out_swc_file,reindexNT(nt_out));
    }
}
NeuronTree preprocess_simple(NeuronTree nt){
    V3DLONG niz=nt.listNeuron.size();
    //
    QList<V3DLONG> cnodes; cnodes.clear();
    for(V3DLONG i=0;i<niz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(s.pn<0){
            cout<<"---------------Attempt to process multiple -1 nodes-----------------------"<<endl;
            //find the node with same coordinates
            for(V3DLONG j=0;j<niz;j++){
                NeuronSWC sj=nt.listNeuron.at(j);
                if(s.x==sj.x&&s.y==sj.y&&s.z==sj.z
                        &&i!=j)
                {
                    cnodes.append(i);
                    nt.listNeuron[i].pn=sj.n;
                }
            }
        }
    }
    //remove duplicated nodes

    QVector<V3DLONG> dup_nodes(niz,0);
    for(V3DLONG i=0;i<niz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(s.pn>0&&nt.hashNeuron.contains(s.pn))
        {
            V3DLONG pid=nt.hashNeuron.value(s.pn);
            NeuronSWC sp=nt.listNeuron.at(pid);
            if(sp.pn>0&&nt.hashNeuron.contains(sp.pn)){
                V3DLONG spid=nt.hashNeuron.value(sp.pn);
                NeuronSWC spp=nt.listNeuron.at(spid);
                double ppp_dist=dis(sp,spp);
                if(ppp_dist<1&&cnodes.contains(pid)){
                    //skip sp
                    dup_nodes[pid]=1;
                    nt.listNeuron[i].pn=sp.pn;
                }
            }
        }
    }
    NeuronTree nt_out;
    for(V3DLONG i=0;i<niz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(dup_nodes.at(i)>0)
            continue;
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    return nt_out;
}
//refinement
void refinement_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool in_terafly){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    /*method code:
        * 0: reconstruction refinement
        * 1: node refinement
        * 2: 0+1
    */
    string inswc_file,inimg_file;
    if(infiles.size()>=2) {inimg_file = infiles[0];inswc_file = infiles[1];}
    else
        printHelp();
    //read para list
    int method_code=(inparas.size()>=1)?atoi(inparas[0]):2;
    int refine_radius=/*(inparas.size()>=2)?atoi(inparas[1]):*/3;
    int nodeRefine_radius=/*(inparas.size()>=3)?atoi(inparas[2]):*/2;
    int interpolation_pixels=/*(inparas.size()>=4)?atoi(inparas[3]):*/4;
    int half_crop_size=/*(inparas.size()>=5)?atoi(inparas[4]):*/128;
    //read input swc to neuron-tree
   NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
   if(!nt.listNeuron.size()) return;

   // shift or refinement function

   string refined_swc=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_refined.eswc");
   NeuronTree nt_out;
   if(in_terafly)
   {
       refinement_terafly_fun(callback,inimg_file,nt,method_code,refine_radius,half_crop_size,nodeRefine_radius);
       //string refined_swc1=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_refined1.eswc");
      // writeESWC_file(QString::fromStdString(refined_swc1),nt);
       NeuronTree nt_pruning=internode_pruning(nt,1.0);
       nt_out=node_interpolation(nt_pruning,interpolation_pixels,true);
       //nt_out=reindexNT(nt_pre);
   }
   else
   {
       NeuronTree nt_pre=preprocess_simple(nt);
       nt_out=reindexNT(nt_pre);
       refinement_Image_fun(callback,inimg_file,nt_out,method_code,refine_radius,nodeRefine_radius);
   }
   writeESWC_file(QString::fromStdString(refined_swc),nt_out);
}
/*void refinement_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int method_code,int refine_radius,long half_block_size,int nodeRefine_radius){
    // need a good strategy for fast retrieving all the nodes in neuron tree
    cout<<"Refinement uses mean-shift, under terafly datasets"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = listNeuron.size();
    vector<V3DLONG> scanned(siz,0);
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}

    int min_dist_to_block_edge=32;
    QList<CellAPO> nt_centers;
    nt_centers=nt_2_multi_centers(nt,2*half_block_size,2*half_block_size,2*half_block_size);

    for(int i=0; i<nt_centers.size();i++){
        //for every block
        CellAPO this_center=nt_centers.at(i);
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x=this_center.x-half_block_size-min_dist_to_block_edge;
        start_x=MAX(start_x,0);
        end_x=this_center.x+half_block_size+min_dist_to_block_edge;
        end_x=MIN(end_x,in_zz[0]-1);

        start_y=this_center.y-half_block_size-min_dist_to_block_edge;
        start_y=MAX(start_y,0);
        end_y=this_center.y+half_block_size+min_dist_to_block_edge;
        end_y=MIN(end_y,in_zz[1]-1);

        start_z=this_center.z-half_block_size-min_dist_to_block_edge;
        start_z=MAX(start_z,0);
        end_z=this_center.z+half_block_size+min_dist_to_block_edge;
        end_z=MIN(end_z,in_zz[2]-1);

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
        double bkg_thresh= imgave;
        bkg_thresh=MAX(bkg_thresh,20);
        //for all the node inside this block
        for(V3DLONG j=0;j<siz;j++){
            NeuronSWC sj = listNeuron[j];
            if(scanned.at(j)==0&&
                    abs(sj.x-this_center.x)<=half_block_size&&
                    abs(sj.y-this_center.y)<=half_block_size&&
                    abs(sj.z-this_center.z)<=half_block_size)
            {
                V3DLONG thisx,thisy,thisz;
                thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                NeuronSWC sj_shifted=sj;NeuronSWC out;
                sj_shifted.x-=start_x; sj_shifted.y-=start_y; sj_shifted.z-=start_z;

                if(method_code==NeuronTreeRefine)
                    out=calc_mean_shift_center(inimg1d,sj_shifted,in_sz,bkg_thresh,refine_radius);
                else if(method_code==NodeRefine)
                    out=nodeRefine(inimg1d,sj_shifted,in_sz,refine_radius);
                else if(method_code==RefineAllinOne){
                    out=calc_mean_shift_center(inimg1d,sj_shifted,in_sz,bkg_thresh,refine_radius);
                    out=nodeRefine(inimg1d,out,in_sz,nodeRefine_radius);
                }
                scanned[j]=1;
                listNeuron[j].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                listNeuron[j].timestamp=bkg_thresh;
                listNeuron[j].x=float(start_x)+out.x;
                listNeuron[j].y=float(start_y)+out.y;
                listNeuron[j].z=float(start_z)+out.z;
            }
        }
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}
}*/
void refinement_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int method_code,int refine_radius,long half_block_size,int nodeRefine_radius){
    cout<<"Refinement uses mean-shift, under terafly datasets"<<endl;
//    NeuronTree nt_raw; nt_raw.deepCopy(nt);
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = listNeuron.size(); vector<V3DLONG> scanned(siz,0);
    /*skip nodes are two close to soma*/
    double mindist=20;
    V3DLONG somaid=get_soma(nt);
    if(somaid>=0){
        NeuronSWC soma_node=nt.listNeuron.at(somaid);
        for(V3DLONG i=0;i<siz;i++)
        {
            NeuronSWC s = listNeuron.at(i);
            double dist_to_soma=dis(soma_node,s);
            if(dist_to_soma<mindist)
                scanned[i]=1;
        }
    }
    /*skip the refinement of small tip-branch*/
    double MIN_BR_LEN=20;
    QHash <V3DLONG, V3DLONG>  hashNeuron; hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);
    vector<int> ntype(siz,0);    ntype=getNodeType(nt);
    for(V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        if((ntype.at(i)==0)&&hashNeuron.contains(s.pn)){
            V3DLONG pid=hashNeuron.value(s.pn);
            NeuronSWC sp=listNeuron.at(pid);
            QList<V3DLONG> br_nodes; br_nodes.clear();
            br_nodes.append(i);
            double br_len=0.0;
            while(ntype.at(pid)==1){
                br_len+=dis(s,sp);
                s=sp;
                br_nodes.append(pid);
                if(!hashNeuron.contains(s.pn))
                    break;
                pid=hashNeuron.value(s.pn);
                sp=listNeuron.at(pid);
            }
            if(br_len<MIN_BR_LEN)
                for(int b=0;b<br_nodes.size();b++)
                    scanned[br_nodes.at(b)]=-1;
        }
    }
    vector<int> norder(siz,0);    getNodeOrder(nt,norder);

    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}

    int min_dist_to_block_edge=32; int order_thre=7;
    for(V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC ss = listNeuron[i];
        if(scanned.at(i)<=0)
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
//            V3DLONG sz01 = in_sz[0] * in_sz[1];
//            V3DLONG sz0 = in_sz[0];

            unsigned char * inimg1d_raw = 0;
            V3DLONG pagesz= in_sz[0] * in_sz[1]*in_sz[2]*in_sz[3];
            try {inimg1d_raw = new unsigned char [pagesz];}
            catch(...)  {cout<<"cannot allocate memory for processing."<<endl; return;}
            inimg1d_raw = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            if(inimg1d_raw==NULL){ cout<<"Crop fail"<<endl;continue; }

            /*image enhancement*/
            unsigned char * inimg1d = 0;
            try {inimg1d = new unsigned char [pagesz];}
            catch(...)  {cout<<"cannot allocate memory for enhancement."<<endl; return;}
            enhanceImage(inimg1d_raw,inimg1d,in_sz);
            if(inimg1d==NULL)
            {
//                cout<<"adaptive thresholding and enhancement fail"<<endl;
//                double imgave,imgstd;
//                mean_and_std(inimg1d_raw,pagesz,imgave,imgstd);
//                double bkg_thresh= MIN(MAX(imgave+imgstd+15,30),50);

                //for all the node inside this block
                double bkg_thresh=40;
                vector<int> windowradius_pid(5,0);
                int windowradius_pid_tmp;
                int windowradius_limit=15;
                for(V3DLONG j=0;j<siz;j++){
                    NeuronSWC sj = listNeuron[j];
                    if(scanned.at(j)==0&&
                            (sj.x-start_x)>=min_dist_to_block_edge&&(end_x-sj.x)>=min_dist_to_block_edge&&
                             (sj.y-start_y)>=min_dist_to_block_edge&&(end_y-sj.y)>=min_dist_to_block_edge&&
                             (sj.z-start_z)>=min_dist_to_block_edge&&(end_z-sj.z)>=min_dist_to_block_edge)
                    {
                        V3DLONG thisx,thisy,thisz;                    thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                        NeuronSWC sj_shifted=sj;NeuronSWC out;
                        sj_shifted.x-=start_x; sj_shifted.y-=start_y; sj_shifted.z-=start_z;
                        if(method_code==NeuronTreeRefine)
                        {
                            if(norder.at(j)<=order_thre){
                              out=calc_mean_shift_center_v5(inimg1d_raw,sj_shifted,in_sz,bkg_thresh,windowradius_pid_tmp,refine_radius,windowradius_limit);
                            }else{
                              out=calc_mean_shift_center(inimg1d_raw,sj_shifted,in_sz,bkg_thresh,refine_radius);
                           }
                        }
                        else if(method_code==NodeRefine)
                            out=nodeRefine(inimg1d_raw,sj_shifted,in_sz,nodeRefine_radius);
                        else if(method_code==RefineAllinOne){
                            if(norder.at(j)<=order_thre){
                                if(j%5==0){
                                    out=calc_mean_shift_center_v5(inimg1d_raw,sj_shifted,in_sz,bkg_thresh,windowradius_pid_tmp,refine_radius,windowradius_limit);
                                    windowradius_pid.at(j%5)=windowradius_pid_tmp;
                                }else{
                                    out=calc_mean_shift_center_v4(inimg1d_raw,sj_shifted,in_sz,bkg_thresh,windowradius_pid_tmp,refine_radius,windowradius_limit);
                                    windowradius_pid.at(j%5)=windowradius_pid_tmp;
                                }
                            }else{
                                out=calc_mean_shift_center(inimg1d_raw,sj_shifted,in_sz,bkg_thresh,refine_radius);
                            }
                            if(scanned.at(j)<0)
                                out=nodeRefine(inimg1d_raw,out,in_sz,1);
                            else
                                out=nodeRefine(inimg1d_raw,out,in_sz,nodeRefine_radius);
                        }
                        scanned[j]=1/*inimg1d_raw[thisz * sz01 + thisy* sz0 + thisx]*/;
                        listNeuron[j].x=float(start_x)+out.x;
                        listNeuron[j].y=float(start_y)+out.y;
                        listNeuron[j].z=float(start_z)+out.z;
                    }
                    if(j>=4){
                           int windowradius_mean=0;
                           for(int k=0;k<=4;k++){
                               windowradius_mean+=windowradius_pid.at(k);
                           }
                           windowradius_mean=windowradius_mean/5;
                           windowradius_limit=MIN(windowradius_mean,14);
                    }
                }
                if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
            }
            else
            {
                cout<<"adaptive thresholding and enhancement finished"<<endl;
                if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
                double bkg_thresh=40;
                vector<int> windowradius_pid(5,0);
                int windowradius_pid_tmp;
                int windowradius_limit=15;

                //for all the node inside this block
                for(V3DLONG j=0;j<siz;j++){
                    NeuronSWC sj = listNeuron[j];
                    if(scanned.at(j)==0&&
                            (sj.x-start_x)>=min_dist_to_block_edge&&(end_x-sj.x)>=min_dist_to_block_edge&&
                             (sj.y-start_y)>=min_dist_to_block_edge&&(end_y-sj.y)>=min_dist_to_block_edge&&
                             (sj.z-start_z)>=min_dist_to_block_edge&&(end_z-sj.z)>=min_dist_to_block_edge)
                    {
                        V3DLONG thisx,thisy,thisz;                    thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                        NeuronSWC sj_shifted=sj;NeuronSWC out;
                        sj_shifted.x-=start_x; sj_shifted.y-=start_y; sj_shifted.z-=start_z;
                        if(method_code==NeuronTreeRefine)
                        {
                            if(norder.at(j)<=order_thre){
                              out=calc_mean_shift_center_v5(inimg1d,sj_shifted,in_sz,bkg_thresh,windowradius_pid_tmp,refine_radius,windowradius_limit);
                            }else{
                              out=calc_mean_shift_center(inimg1d,sj_shifted,in_sz,bkg_thresh,refine_radius);
                           }
                        }
                        else if(method_code==NodeRefine)
                            out=nodeRefine(inimg1d,sj_shifted,in_sz,nodeRefine_radius);
                        else if(method_code==RefineAllinOne){
                            if(norder.at(j)<=order_thre){
                                if(j%5==0){
                                    out=calc_mean_shift_center_v5(inimg1d,sj_shifted,in_sz,bkg_thresh,windowradius_pid_tmp,3,windowradius_limit);
                                    windowradius_pid.at(j%5)=windowradius_pid_tmp;
                                }else{
                                    out=calc_mean_shift_center_v4(inimg1d,sj_shifted,in_sz,bkg_thresh,windowradius_pid_tmp,3,windowradius_limit);
                                    windowradius_pid.at(j%5)=windowradius_pid_tmp;
                                }
                            }else{
                                out=calc_mean_shift_center(inimg1d,sj_shifted,in_sz,bkg_thresh,3);
                            }
                            if(scanned.at(j)<0)
                                out=nodeRefine(inimg1d,out,in_sz,1);
                            else
                                out=nodeRefine(inimg1d,out,in_sz,2);
                        }
                        scanned[j]=1/*inimg1d[thisz * sz01 + thisy* sz0 + thisx]*/;
                        //listNeuron[j].level=bkg_thresh;
                        listNeuron[j].x=float(start_x)+out.x;
                        listNeuron[j].y=float(start_y)+out.y;
                        listNeuron[j].z=float(start_z)+out.z;
                    }
                    if(j>=4){
                           int windowradius_mean=0;
                           for(int k=0;k<=4;k++){
                               windowradius_mean+=windowradius_pid.at(k);
                           }
                           windowradius_mean=windowradius_mean/5;
                           windowradius_limit=MIN(windowradius_mean,14);
                    }
                }
                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }
    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}
}
void refinement_Image_fun(V3DPluginCallback2 &callback,string inimg_file, NeuronTree& nt,int method_code,int refine_radius,int nodeRefine_radius){
    cout<<"Refinement uses mean-shift, under image block"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();

    //read image file
    unsigned char * inimg1d_raw = 0;V3DLONG in_sz[4];int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d_raw, in_sz, datatype)) return;

    V3DLONG sz01 = in_sz[0] * in_sz[1];
    V3DLONG sz0 = in_sz[0];
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
    /*image enhancement*/
    unsigned char * inimg1d = 0;
    try {inimg1d = new unsigned char [total_size];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
    enhanceImage(inimg1d_raw,inimg1d,in_sz,true);
    if(inimg1d==NULL/*true*/)
    {
        cout<<"adaptive thresholding and enhancement fail"<<endl;
        double imgave,imgstd;
        mean_and_std(inimg1d_raw,total_size,imgave,imgstd);
        double bkg_thresh=MIN(MAX(imgave+imgstd+15,30),60);
        cout<<"bkg thresh="<<bkg_thresh<<endl;

        for(V3DLONG i=0;i<siz;i++)
        {
            //for all the node
            NeuronSWC s = listNeuron[i];
            V3DLONG thisx,thisy,thisz;        thisx=s.x;thisy=s.y;thisz=s.z;
            s.level=inimg1d_raw[thisz * sz01 + thisy * sz0 + thisx];
            listNeuron[i].level=bkg_thresh;

            NeuronSWC out;
            if(method_code==NeuronTreeRefine)
                out=calc_mean_shift_center(inimg1d_raw,listNeuron.at(i),in_sz,bkg_thresh,refine_radius);
            else if(method_code==NodeRefine)
                out=nodeRefine(inimg1d_raw,listNeuron.at(i),in_sz,nodeRefine_radius);
            else if(method_code>=RefineAllinOne){
//                out=calc_mean_shift_center_v2(inimg1d_raw,listNeuron.at(i),in_sz,bkg_thresh,refine_radius);
                int windowradius_pid_tmp;
                out=calc_mean_shift_center_v5(inimg1d_raw,listNeuron.at(i),in_sz,bkg_thresh,windowradius_pid_tmp,refine_radius,15);
                out=nodeRefine(inimg1d_raw,out,in_sz,nodeRefine_radius);
            }
            listNeuron[i].x=out.x;
            listNeuron[i].y=out.y;
            listNeuron[i].z=out.z;
            listNeuron[i].level=bkg_thresh;
        }
        if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
    }
    else
    {
        cout<<"adaptive thresholding and enhancement finished"<<endl;
        if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
        double imgave,imgstd;
        mean_and_std(inimg1d,total_size,imgave,imgstd);
        double bkg_thresh=MIN(MAX(imgave+imgstd+15,30),60);
        cout<<"bkg thresh="<<bkg_thresh<<endl;

        for(V3DLONG i=0;i<siz;i++)
        {
            //for all the node
            NeuronSWC s = listNeuron.at(i);
            NeuronSWC out;
            if(method_code==NeuronTreeRefine)
                out=calc_mean_shift_center(inimg1d,s,in_sz,bkg_thresh,refine_radius);
            else if(method_code==NodeRefine)
                out=nodeRefine(inimg1d,s,in_sz,nodeRefine_radius);
            else if(method_code>=RefineAllinOne){
                out=calc_mean_shift_center(inimg1d,s,in_sz,bkg_thresh,refine_radius);
                out=nodeRefine(inimg1d,out,in_sz,nodeRefine_radius);
            }
            listNeuron[i].x=out.x;
            listNeuron[i].y=out.y;
            listNeuron[i].z=out.z;
            listNeuron[i].level=bkg_thresh;
        }
       cout<<"Refine finished"<<endl;
       if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
}
NeuronSWC calc_mean_shift_center_v5(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double &bkg_thre,int &windowradius_pid,int windowradius,int windowradius_limit)
{
    //qDebug()<<"methodcode:"<<methodcode;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    V3DLONG pos;
//    vector<V3DLONG> coord;

    float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
    //float center_dis=1;
    vector<float> center_float(3,0);

    x=snode.x;y=snode.y;z=snode.z;
    //qDebug()<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<ind;

    //find out the channel with the maximum intensity for the marker
    v_prev=inimg1d[long(z)*z_offset+long(y)*y_offset+long(x)];
    int testCount=0;
    int testCount1=0;
    int mycount=0;
    int thresHold = 0;//最佳二值化阈值
    //int v_color_mean=bkg_thre;
    int v_color_mean=0;
    pos=z*z_offset+y*y_offset+x;
    int v_color_xyz=inimg1d[pos];
    int v_color_max=inimg1d[pos];
    int c=0;
    int windowradius1 = 5;
    for(V3DLONG dx=MAX(x+0.5-windowradius1,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius1); dx++){
        for(V3DLONG dy=MAX(y+0.5-windowradius1,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius1); dy++){
            for(V3DLONG dz=MAX(z+0.5-windowradius1,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius1); dz++){
                     pos=dz*z_offset+dy*y_offset+dx;
                     v_color=inimg1d[pos];
                        //hist[v_color]+=1;
                        v_color_mean+=v_color;
                        c+=1;
                        v_color_max=MAX(v_color_max,v_color);
            }
        }
     }
    if(v_color_xyz>=v_color_max-1){
        printf("11:\n");
        return snode;
    }
    v_color_mean=v_color_mean/c;
    thresHold=MAX(40,2*(v_color_mean-31)+40);
    thresHold=MIN(thresHold,60);
    bkg_thre=thresHold;
    //if(v_color_xyz>=thresHold)
    //thresHold=MAX(40,2*(bkg_thre-31)+40);
//    printf("thresHold: %d\n",thresHold);
//    printf("v_color_mean: %d\n",v_color_mean);
    int windowradius2= windowradius;
    while(v_color_mean >= 14 && mycount<=5 && windowradius2<=windowradius_limit){
        mycount=0;
    for(V3DLONG dx=MAX(x+0.5-windowradius2,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius2); dx++){
        for(V3DLONG dy=MAX(y+0.5-windowradius2,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius2); dy++){
            for(V3DLONG dz=MAX(z+0.5-windowradius2,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius2); dz++){
                   pos=dz*z_offset+dy*y_offset+dx;
                    double tmp1=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    v_color=inimg1d[pos];
                    if(tmp1>=((windowradius2+0.5)*(windowradius2+0.5))) v_color=0;
                    if (v_color>=thresHold) mycount+=1;
            }
        }
     }
     if(mycount<=5) windowradius2+=1;
    }
    windowradius=windowradius2;
//    if(v_color_mean<=20){windowradius=3;}
    windowradius_pid=windowradius;
    //windowradius=3;
    //printf("windows: %d\n",windowradius);
    while (testCount<1)//center_dis>=0.5 &&
    {
        total_x=total_y=total_z=sum_v=0;
        testCount++;
        testCount1=0;

        for(V3DLONG dx=MAX(x+0.5-windowradius,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius); dx++){
            for(V3DLONG dy=MAX(y+0.5-windowradius,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius); dy++){
                for(V3DLONG dz=MAX(z+0.5-windowradius,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius); dz++){
                    pos=dz*z_offset+dy*y_offset+dx;
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=inimg1d[pos];
                    if(v_color<thresHold)
                        v_color=0;
                    total_x=v_color*(float)dx+total_x;
                    total_y=v_color*(float)dy+total_y;
                    total_z=v_color*(float)dz+total_z;
                    sum_v=sum_v+v_color;

                    //testCount++;
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

//        float tmp_1=(center_float[0]-x)*(center_float[0]-x)+(center_float[1]-y)*(center_float[1]-y)
//                    +(center_float[2]-z)*(center_float[2]-z);
//        center_dis=sqrt(tmp_1);
        x=center_float[0]; y=center_float[1]; z=center_float[2];
    }

    NeuronSWC out_center=snode;
    out_center.x=center_float[0]; out_center.y=center_float[1]; out_center.z=center_float[2];
    out_center.level=inimg1d[long(out_center.z)*z_offset+long(out_center.y)*y_offset+long(out_center.x)];
//    if(out_center.level>v_prev)
//        cout<<"shift intensity improve= "<<out_center.level-v_prev<<endl;
    //printf("testCount: %d\n",testCount);
    return out_center;
}
NeuronSWC calc_mean_shift_center_v4(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double &bkg_thre,int &windowradius_pid,int windowradius,int windowradius_limit)
{
    //qDebug()<<"methodcode:"<<methodcode;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    V3DLONG pos;
//    vector<V3DLONG> coord;

    float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
    //float center_dis=1;
    vector<float> center_float(3,0);

    x=snode.x;y=snode.y;z=snode.z;
    //qDebug()<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<ind;

    //find out the channel with the maximum intensity for the marker
    v_prev=inimg1d[long(z)*z_offset+long(y)*y_offset+long(x)];
    int testCount=0;
    int testCount1=0;
    int mycount=0;
    int thresHold = 0;//最佳二值化阈值
    int v_color_mean=bkg_thre;

    thresHold=MAX(40,2*(v_color_mean-31)+40);
    thresHold=MIN(thresHold,60);

    int windowradius2= windowradius;
    while(v_color_mean >= 14 && mycount<=5 && windowradius2<=windowradius_limit){
        mycount=0;
    for(V3DLONG dx=MAX(x+0.5-windowradius2,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius2); dx++){
        for(V3DLONG dy=MAX(y+0.5-windowradius2,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius2); dy++){
            for(V3DLONG dz=MAX(z+0.5-windowradius2,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius2); dz++){
                   pos=dz*z_offset+dy*y_offset+dx;
                    double tmp1=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    v_color=inimg1d[pos];
                    if(tmp1>=((windowradius2+0.5)*(windowradius2+0.5))) v_color=0;
                    if (v_color>=thresHold) mycount+=1;
            }
        }
     }
     if(mycount<=5) windowradius2+=1;
    }
    windowradius=windowradius2;
    if(v_color_mean<=20){windowradius=3;}
    windowradius_pid=windowradius;
    //windowradius=3;
    //printf("windows: %d\n",windowradius);
    while (testCount<1)//center_dis>=0.5 &&
    {
        total_x=total_y=total_z=sum_v=0;
        testCount++;
        testCount1=0;

        for(V3DLONG dx=MAX(x+0.5-windowradius,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius); dx++){
            for(V3DLONG dy=MAX(y+0.5-windowradius,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius); dy++){
                for(V3DLONG dz=MAX(z+0.5-windowradius,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius); dz++){
                    pos=dz*z_offset+dy*y_offset+dx;
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=inimg1d[pos];
                    if(v_color<thresHold)
                        v_color=0;
                    total_x=v_color*(float)dx+total_x;
                    total_y=v_color*(float)dy+total_y;
                    total_z=v_color*(float)dz+total_z;
                    sum_v=sum_v+v_color;

                    //testCount++;
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
        x=center_float[0]; y=center_float[1]; z=center_float[2];
    }

    NeuronSWC out_center=snode;
    out_center.x=center_float[0]; out_center.y=center_float[1]; out_center.z=center_float[2];
    out_center.level=inimg1d[long(out_center.z)*z_offset+long(out_center.y)*y_offset+long(out_center.x)];
    return out_center;
}
NeuronSWC calc_mean_shift_center(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double bkg_thre,int windowradius)
{
    //qDebug()<<"methodcode:"<<methodcode;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    V3DLONG pos;
//    vector<V3DLONG> coord;

    float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
    //float center_dis=1;
    vector<float> center_float(3,0);

    x=snode.x;y=snode.y;z=snode.z;
    //qDebug()<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<ind;
    int windowradius1 = 3;
    int thresHold = bkg_thre;//最佳二值化阈值
    int v_color_mean=0;
    int c=0;
    for(V3DLONG dx=MAX(x+0.5-windowradius1,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius1); dx++){
        for(V3DLONG dy=MAX(y+0.5-windowradius1,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius1); dy++){
            for(V3DLONG dz=MAX(z+0.5-windowradius1,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius1); dz++){
                     pos=dz*z_offset+dy*y_offset+dx;
                     v_color=inimg1d[pos];
                        //hist[v_color]+=1;
                        v_color_mean+=v_color;
                        c+=1;
            }
        }
     }

    //v_color_mean=bkg_thre;
    v_color_mean= v_color_mean/c;
    if(v_color_mean<14){
        return snode;
    }
//    thresHold=MAX(40,2*(v_color_mean-31)+40);
//    thresHold=MIN(thresHold,60);
    //find out the channel with the maximum intensity for the marker
    v_prev=inimg1d[long(z)*z_offset+long(y)*y_offset+long(x)];
    int testCount=0;
    int testCount1=0;

    while (testCount<1) //center_dis>=0.5 &&
    {
        total_x=total_y=total_z=sum_v=0;
        testCount++;
        testCount1=0;

        for(V3DLONG dx=MAX(x+0.5-windowradius,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius); dx++){
            for(V3DLONG dy=MAX(y+0.5-windowradius,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius); dy++){
                for(V3DLONG dz=MAX(z+0.5-windowradius,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius); dz++){
                    pos=dz*z_offset+dy*y_offset+dx;
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=inimg1d[pos];
                    if(v_color<thresHold)
                        v_color=0;
                    total_x=v_color*(float)dx+total_x;
                    total_y=v_color*(float)dy+total_y;
                    total_z=v_color*(float)dz+total_z;
                    sum_v=sum_v+v_color;

                    //testCount++;
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

//            v3d_msg("Sphere surrounding the marker is zero. Mean-shift cannot happen. Marker location will not move",0);
            center_float[0]=x;
            center_float[1]=y;
            center_float[2]=z;
            return snode;
        }

//        float tmp_1=(center_float[0]-x)*(center_float[0]-x)+(center_float[1]-y)*(center_float[1]-y)
//                    +(center_float[2]-z)*(center_float[2]-z);
//        center_dis=sqrt(tmp_1);
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

//radius and intensity profile
void swc_profile_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool in_terafly){
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
    int bkg_thre_bias=(inparas.size()>=1)?atoi(inparas[0]):15;
    //read input swc to neuron-tree
   NeuronTree nt_raw = readSWC_file(QString::fromStdString(inswc_file));
   if(!nt_raw.listNeuron.size()) return;

   //save to file: intensity_radius_profiled_file, bouton_apo_file, bouton_eswc_file
   string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_profiled.eswc");

   double block_upsample_factor=4;
   int half_crop_size=128;
   NeuronTree nt_p=node_interpolation(nt_raw,4,true);
   if(in_terafly)
   {
          //remove duplicated nodes and resample
       swc_profile_terafly_fun(callback,inimg_file,nt_p,half_crop_size,bkg_thre_bias,block_upsample_factor);
       NeuronTree nt=internode_pruning(nt_p,2,true);
       if(true){
           //smooth very large radius
           float radius_thre=8.0;
           for(V3DLONG i=0;i<nt.listNeuron.size();i++){
               NeuronSWC s=nt.listNeuron.at(i);
               if(s.pn<0||!nt.hashNeuron.contains(s.pn)){continue;}
               if(s.r>radius_thre){
                   V3DLONG spn_id=nt.hashNeuron.value(s.pn);
                   NeuronSWC spn=nt.listNeuron.at(spn_id);
                   while(true)
                   {
                       if(spn.r<radius_thre){nt.listNeuron[i].r=spn.r;break;}
                       s=spn;
                       if(s.pn<0||!nt.hashNeuron.contains(s.pn)){
                           nt.listNeuron[i].r=1;break;
                       }
                       spn_id=nt.hashNeuron.value(s.pn);
                       spn=nt.listNeuron.at(spn_id);
                   }
               }
           }
       }
       writeESWC_file(QString::fromStdString(out_swc_file),nt);
   }
   else
   {
//       writeESWC_file(QString::fromStdString(inswc_file + "_resample.eswc"),nt_p);
       swc_profile_image_fun(callback,inimg_file,nt_p,bkg_thre_bias,block_upsample_factor);
       NeuronTree nt_out=internode_pruning(nt_p,2,true);
       writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
   }

}
void swc_profile_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,long block_size,int bkg_bias,double upfactor)
{
    cout<<"get radius and intensity profile from terafly datasets"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = listNeuron.size(); vector<V3DLONG> scanned(siz,0);

    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz)){cout<<"can't load terafly img"<<endl;return;}
    int min_dist_to_block_edge=16;
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
        NeuronSWC ss = listNeuron[i];
        if(scanned.at(i)==0)
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

            unsigned char * inimg1d_raw = 0;
            V3DLONG pagesz= in_sz[0] * in_sz[1]*in_sz[2]*in_sz[3];
            try {inimg1d_raw = new unsigned char [pagesz];}
            catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
            inimg1d_raw = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            if(inimg1d_raw==NULL){cout<<"Crop fail"<<endl;continue; }

            double imgave_raw,imgstd_raw; imgave_raw=imgstd_raw=10;
            //mean_and_std(inimg1d_raw,pagesz,imgave_raw,imgstd_raw);
            double bkg_thresh= MIN(MAX(imgave_raw+imgstd_raw+bkg_bias,30),60);
            /*image enhancement*/
            unsigned char * inimg1d = 0;
            try {inimg1d = new unsigned char [pagesz];}
            catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
            enhanceImage(inimg1d_raw,inimg1d,in_sz);
            if(inimg1d==NULL)
            {
                cout<<"adaptive thresholding and enhancement fail"<<endl;

                //for all the node inside this block
                for(V3DLONG j=0;j<siz;j++){
                    NeuronSWC sj = listNeuron[j];
                    if(scanned.at(j)==0&&
                            (sj.x-start_x)>=min_dist_to_block_edge&&(end_x-sj.x)>=min_dist_to_block_edge&&
                             (sj.y-start_y)>=min_dist_to_block_edge&&(end_y-sj.y)>=min_dist_to_block_edge&&
                             (sj.z-start_z)>=min_dist_to_block_edge&&(end_z-sj.z)>=min_dist_to_block_edge)
                    {
                        V3DLONG thisx,thisy,thisz;
                        thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                        listNeuron[j].level=inimg1d_raw[thisz * sz01 + thisy* sz0 + thisx];

                        //get node radius
                        NeuronSWC sr=sj;
                        sr.x=thisx; sr.y=thisy;sr.z=thisz;
                        listNeuron[j].r=radiusEstimation(inimg1d_raw,in_sz,sr,upfactor,bkg_thresh);
                        scanned[j]=1;
                        listNeuron[j].timestamp=bkg_thresh;
                        if(false){
                            NeuronSWC out;
                            out=nodeRefine(inimg1d_raw,sr,in_sz,2);
                            if(out.level>imgave_raw+listNeuron[j].level){
                                //get node radius
                                listNeuron[j].x=out.x+start_x;
                                listNeuron[j].y=out.y+start_y;
                                listNeuron[j].z=out.z+start_z;
                                listNeuron[j].r=radiusEstimation(inimg1d_raw,in_sz,out,upfactor,bkg_thresh);
                            }
                        }
                    }
                }
                if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
            }
            else
            {
                cout<<"adaptive thresholding and enhancement"<<endl;
                if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
//                double imgave,imgstd;
//                mean_and_std(inimg1d,pagesz,imgave,imgstd);
//                double bkg_thresh= MIN(MAX(imgave+imgstd+bkg_bias,30),60);
                //for all the node inside this block
                for(V3DLONG j=0;j<siz;j++){
                    NeuronSWC sj = listNeuron[j];
                    if(scanned.at(j)==0&&
                            (sj.x-start_x)>=min_dist_to_block_edge&&(end_x-sj.x)>=min_dist_to_block_edge&&
                             (sj.y-start_y)>=min_dist_to_block_edge&&(end_y-sj.y)>=min_dist_to_block_edge&&
                             (sj.z-start_z)>=min_dist_to_block_edge&&(end_z-sj.z)>=min_dist_to_block_edge)
                    {
                        V3DLONG thisx,thisy,thisz;
                        thisx=sj.x-start_x;        thisy=sj.y-start_y;        thisz=sj.z-start_z;
                        listNeuron[j].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                        NeuronSWC sj_shifted=sj;
                        sj_shifted.x-=start_x; sj_shifted.y-=start_y; sj_shifted.z-=start_z;
                        listNeuron[j].r=radiusEstimation(inimg1d,in_sz,sj_shifted,upfactor,bkg_thresh);
                        listNeuron[j].timestamp=bkg_thresh;
                        scanned[j]=1;
                        if(false){
                            NeuronSWC out;
                            out=nodeRefine(inimg1d,sj_shifted,in_sz,2);
                            if(out.level>imgave_raw+listNeuron[j].level){
                                //get node radius
                                listNeuron[j].x=out.x+start_x;
                                listNeuron[j].y=out.y+start_y;
                                listNeuron[j].z=out.z+start_z;
                                listNeuron[j].r=radiusEstimation(inimg1d,in_sz,out,upfactor,bkg_thresh);
                            }
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
void swc_profile_image_fun(V3DPluginCallback2 &callback,string inimg_file, NeuronTree& nt,int bkg_bias, double upfactor){
    cout<<"get radius and intensity profile from image block"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();

    //read image file
    unsigned char * inimg1d_raw = 0;
    V3DLONG in_sz[4];int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d_raw, in_sz, datatype)) return;
    //get the background threshold
    V3DLONG sz01 = in_sz[0] * in_sz[1];
    V3DLONG sz0 = in_sz[0];
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];

    /*image enhancement*/
    unsigned char * inimg1d = 0;
    try {inimg1d = new unsigned char [total_size];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
    enhanceImage(inimg1d_raw,inimg1d,in_sz);
    if(inimg1d==NULL)
    {
        cout<<"adaptive thresholding and enhancement fail"<<endl;
        double imgave,imgstd;
        mean_and_std(inimg1d_raw,total_size,imgave,imgstd);
        double bkg_thresh=MIN(MAX(imgave+imgstd+bkg_bias,30),60);
        cout<<"bkg thresh="<<bkg_thresh<<","<<imgave<<","<<imgstd<<endl;

        for(V3DLONG i=0;i<siz;i++)
        {
            //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
            NeuronSWC s = listNeuron.at(i);
            V3DLONG thisx,thisy,thisz;
            thisx=s.x;thisy=s.y;thisz=s.z;
            s.level=inimg1d_raw[thisz * sz01 + thisy * sz0 + thisx];
            //get node radius
            s.r=radiusEstimation(inimg1d_raw,in_sz,s,upfactor,bkg_thresh);
            s.timestamp=bkg_thresh;
            listNeuron[i].level=s.level;
            listNeuron[i].x=s.x;
            listNeuron[i].y=s.y;
            listNeuron[i].z=s.z;
            listNeuron[i].r=s.r;
        }
        if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
    }
    else
    {
        cout<<"adaptive thresholding and enhancement"<<endl;
        if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}

        double imgave,imgstd;
        mean_and_std(inimg1d,total_size,imgave,imgstd);
        double bkg_thresh=MIN(MAX(imgave+imgstd+bkg_bias,30),60);
        cout<<"bkg thresh="<<bkg_thresh<<","<<imgave<<","<<imgstd<<endl;

        for(V3DLONG i=0;i<siz;i++)
        {
            //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
            NeuronSWC s = listNeuron.at(i);
            V3DLONG thisx,thisy,thisz;
            thisx=s.x;thisy=s.y;thisz=s.z;
            s.level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
            //get node radius
            s.r=radiusEstimation(inimg1d,in_sz,s,upfactor,bkg_thresh);
            s.timestamp=bkg_thresh;
            listNeuron[i].level=s.level;
            listNeuron[i].x=s.x;
            listNeuron[i].y=s.y;
            listNeuron[i].z=s.z;
            listNeuron[i].r=s.r;
        }
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
}
//ccf profile
void ccf_profile_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString in_raw_swc_file,in_registered_swc_file;
    if(infiles.size()>=2) {
        in_raw_swc_file = infiles[0];
        in_registered_swc_file = infiles[1];
    }
    float shift_pixels=(inparas.size()>=1)?atof(inparas[0]):0.0;
    float scale_para=(inparas.size()>=2)?atof(inparas[1]):1.0;

    NeuronTree nt = readSWC_file(in_raw_swc_file);
    NeuronTree nt_registered = readSWC_file(in_registered_swc_file);
    if(!nt.listNeuron.size()||
            !nt_registered.listNeuron.size()||
            nt.listNeuron.size()!=nt_registered.listNeuron.size())
        return;
    QString out_nt_filename=(outfiles.size()>=1)?outfiles[0]:(in_raw_swc_file + "_ccfprofiled.eswc");
    scale_registered_swc(nt_registered,shift_pixels,scale_para);
    merge_registered_swc_onto_raw(nt,nt_registered);
    //remove this part later
//    if(!three_bifurcation_processing(nt_raw)){return;}
//    NeuronTree nt_2nd=tip_branch_pruning(nt_raw,4.0);
//    NeuronTree nt_3=duplicated_tip_branch_pruning(nt_2nd,10);
//    NeuronTree nt=reindexNT(nt_3);
    if(true){
        //smooth very large radius
        float radius_thre=8.0;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.pn<0||!nt.hashNeuron.contains(s.pn)){continue;}
            if(s.r>radius_thre){
                V3DLONG spn_id=nt.hashNeuron.value(s.pn);
                NeuronSWC spn=nt.listNeuron.at(spn_id);
                while(true)
                {
                    if(spn.r<radius_thre){nt.listNeuron[i].r=spn.r;break;}
                    s=spn;
                    if(s.pn<0||!nt.hashNeuron.contains(s.pn)){
                        nt.listNeuron[i].r=1;break;
                    }
                    spn_id=nt.hashNeuron.value(s.pn);
                    spn=nt.listNeuron.at(spn_id);
                }
            }
        }
    }
    writeESWC_file(out_nt_filename,nt);
}
void merge_registered_swc_onto_raw(NeuronTree& nt_raw,NeuronTree nt_registered){
    V3DLONG siz=nt_raw.listNeuron.size();
    for(V3DLONG i=0;i<siz;i++){
        AxonalBouton bs;
        bs.init_bouton(nt_raw.listNeuron.at(i));
        bs.ccfx=nt_registered.listNeuron.at(i).x;
        bs.ccfy=nt_registered.listNeuron.at(i).y;
        bs.ccfz=nt_registered.listNeuron.at(i).z;
        bs.out_to_NeuronSWC(nt_raw.listNeuron[i]);
    }
}
void scale_registered_swc(NeuronTree& nt,float xshift_pixels,float scale_xyz){
    V3DLONG siz=nt.listNeuron.size();
    for(V3DLONG i=0;i<siz;i++){
        nt.listNeuron[i].x-=xshift_pixels;
        nt.listNeuron[i].x*=scale_xyz;
        nt.listNeuron[i].y*=scale_xyz;
        nt.listNeuron[i].z*=scale_xyz;
    }
}

/*bouton filter*/
void boutonFilter_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output){
    /*bouton features will be saved at SWC feature value list:
     * 1-3. CCF coordinates {x,y,z}
     * 4. bouton_flag {0: not a bouton; 1: bouton; 2: tip-bouton; 3: terminaux_bouton}
     * 5-7. bouton_radius,branch_radius_mean,branch_radius_std
     * 8-10. bouton_intensity,branch_intensity_mean,branch_intensity_std
     * 11. bouton density
     * 12-13. path_distance_to_soma, euler_dist_to_soma
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    else {  printHelp(); return;}
    //read para list
    double radius_delta=(inparas.size()>=1)?atof(inparas[0]):1.5;
    double intensity_delta=(inparas.size()>=2)?atof(inparas[1]):1.5;
    double axon_trunk_radius=(inparas.size()>=3)?atof(inparas[2]):2;

    //read input swc to neuron-tree
   NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
   if(!nt.listNeuron.size()) return;

    QList <AxonalBouton> init_bouton_sites=boutonFilter_fun(nt,radius_delta,intensity_delta,axon_trunk_radius);
    if(!init_bouton_sites.size()) {return;}
    //map bouton-sites to swc, enlarge fea_val size to 12
    map_bouton_2_neuronTree(nt,init_bouton_sites);
    QString out_bouton_swc_file=(outfiles.size()>=1)?outfiles[0]:(QString::fromStdString(inswc_file)+"_bouton.eswc");
    writeESWC_file(out_bouton_swc_file,nt);
}
QList <CellAPO> boutonFilter_1D(NeuronTree nt,double radius_delta,double intensity_delta,double AXON_BACKBONE_RADIUS){

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
                apo.sdev=radius_mean;
                apo.mass=level_mean;
                apo.comment="bouton_site";
                apolist.push_back(apo);
            }
        }
    }
    cout<<"Bouton size: "<<apolist.size()<<endl;
    return apolist;
}

QList <AxonalBouton> boutonFilter_fun(NeuronTree nt,double radius_delta,double intensity_delta,double AXON_BACKBONE_RADIUS){

    int MIN_PROCESSED_SEG_LEN=20; int MAX_PROCESSED_SEG_LEN=100;
    QList <AxonalBouton> bouton_sites;bouton_sites.clear();
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
        double bouton_radius_thre=radius_mean*
                ((radius_delta>AXON_BACKBONE_RADIUS)?(radius_delta+1):radius_delta);
        bouton_radius_thre=MIN(MAX(bouton_radius_thre,0.49),8);

        std::vector<int> outflag_radius=peaks_in_seg(seg_radius,1,0.01*radius_std);

        //4.2 peak detection of intensity profile
        vector<double> seg_levels,seg_levels_feas;
        seg_levels.clear(); seg_levels_feas.clear();
        seg_levels=get_sorted_fea_of_seg(curseg,false);

        seg_levels_feas=mean_and_std_seg_fea(seg_levels);
        double level_mean=seg_levels_feas[0];
        double level_std=seg_levels_feas[1];
        double bouton_intensity_thre=level_mean*
                ((MAX_PROCESSED_SEG_LEN<curseg.row.size())?(intensity_delta+2):intensity_delta);
        bouton_intensity_thre=MIN(MAX(bouton_intensity_thre,100),250);
        std::vector<int> outflag_level=peaks_in_seg(seg_levels,0,0.05*level_std);
        //4.3 processed
        if(MIN_PROCESSED_SEG_LEN>curseg.row.size()){
            for(int io=0;io<curseg.row.size();io++){
                if(outflag_level[io]>0&&
                        outflag_radius[io]>0&&
                        curseg.row[io].r>AXON_BACKBONE_RADIUS&&
                        curseg.row[io].level>level_mean){
                    AxonalBouton ab;
                    ab.x=curseg.row.at(io).x;
                    ab.y=curseg.row.at(io).y;
                    ab.z=curseg.row.at(io).z;
                    ab.nodeR=curseg.row[io].r;
                    ab.nodetype=BoutonSWCNodeType;
                    ab.btype=BoutonType;
                    ab.ccfR=curseg.row[io].r;
                    ab.br_r_mean=radius_mean;
                    ab.br_r_std=radius_std;
                    ab.broder=curseg.row[io].level;
                    ab.intensity=curseg.row[io].level;
                    ab.br_intensity_mean=level_mean;
                    ab.br_intensity_std=level_std;
                    bouton_sites.append(ab);
                }
            }
        }
        else{
            for(int io=0;io<curseg.row.size();io++){
                if(outflag_level[io]>0&&
                        outflag_radius[io]>0&&
                        curseg.row[io].r>bouton_radius_thre
                        &&curseg.row[io].level>bouton_intensity_thre){
                    AxonalBouton ab;
                    ab.x=curseg.row.at(io).x;
                    ab.y=curseg.row.at(io).y;
                    ab.z=curseg.row.at(io).z;
                    ab.nodeR=curseg.row[io].r;
                    ab.nodetype=BoutonSWCNodeType;
                    ab.btype=BoutonType;
                    ab.ccfR=curseg.row[io].r;
                    ab.br_r_mean=radius_mean;
                    ab.br_r_std=radius_std;
                    ab.broder=curseg.row[io].level;
                    ab.intensity=curseg.row[io].level;
                    ab.br_intensity_mean=level_mean;
                    ab.br_intensity_std=level_std;
                    bouton_sites.append(ab);
                }
            }
        }
    }
    cout<<"initial bouton size: "<<bouton_sites.size()<<endl;
    return bouton_sites;
}
void map_bouton_2_neuronTree(NeuronTree& nt_bouton,QList <AxonalBouton>  bouton_sites){
    V3DLONG biz=bouton_sites.size();
    if(biz<=0) {return;}
    for(V3DLONG i=0;i<nt_bouton.listNeuron.size();i++)
    {
        AxonalBouton sb;
        sb.init_bouton(nt_bouton.listNeuron.at(i));

        for(V3DLONG b=0;b<bouton_sites.size();b++)
        {
            AxonalBouton bs;bs=bouton_sites.at(b);
            if(sb.x==bs.x&&sb.y==bs.y&&sb.z==bs.z)
            {
                bs.ccfx=sb.ccfx;
                bs.ccfy=sb.ccfy;
                bs.ccfz=sb.ccfz;
                bs.out_to_NeuronSWC(nt_bouton.listNeuron[i]);
                sb.btype=bs.btype;
                break;
            }
        }
        if(sb.btype<BoutonType)
            sb.out_to_NeuronSWC(nt_bouton.listNeuron[i]);
    }
}
void map_bouton_2_neuronTree(NeuronTree& nt_bouton,QList <NeuronSWC> bouton_sites){
    V3DLONG biz=bouton_sites.size();
    if(biz<=0) {return;}
    int bfea_size=bouton_sites.at(0).fea_val.size();
    int ntfea_size=nt_bouton.listNeuron.at(0).fea_val.size();
    cout<<"nt fea size="<<ntfea_size<<endl;
    for(V3DLONG i=0;i<nt_bouton.listNeuron.size();i++)
    {
        for(int f=0;f<bfea_size;f++)
            nt_bouton.listNeuron[i].fea_val.append(0.0);
        for(V3DLONG b=0;b<bouton_sites.size();b++)
        {
            if(nt_bouton.listNeuron[i].x==bouton_sites[b].x
                    &&nt_bouton.listNeuron[i].y==bouton_sites[b].y
                    &&nt_bouton.listNeuron[i].z==bouton_sites[b].z)
            {
                nt_bouton.listNeuron[i].r=bouton_sites.at(b).r;
                nt_bouton.listNeuron[i].type=bouton_sites.at(b).type;
                for(int f=0;f<bfea_size;f++)
                    nt_bouton.listNeuron[i].fea_val[ntfea_size+f]=bouton_sites.at(b).fea_val.at(f);
                break;
            }
        }
    }
}
//peak detection
std::vector<double> get_sorted_fea_of_seg(V_NeuronSWC inseg,bool radiusfea)
{
    //radiusfea=true, for radius feature; else for level feature
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
                final_peaks[i]=1;
                V3DLONG bro1_index=(i>0)?i-1:0;
                V3DLONG bro2_index=(i<input.size()-1)?i+1:(i);
                if(input.at(i)-input.at(bro1_index)<=double(0.1)*input.at(i))
                    final_peaks[bro1_index]=1;
                if(input.at(i)-input.at(bro2_index)<=double(0.1)*input.at(i))
                    final_peaks[bro2_index]=1;
            }
        }
        return final_peaks;
    }
    else
        return signals_flag_highpeaks;
}

//bouton swc pruning
void boutonswc_pruning_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    int ccf_domain_flag=(inparas.size()>=1)?atoi(inparas[0]):0;
    float pruning_vol_r=(inparas.size()>=2)?atof(inparas[1]):100.0;
    float pruning_vol_num=(inparas.size()>=3)?atof(inparas[2]):5.0;
    float pruning_tip_thre=(inparas.size()>=4)?atof(inparas[3]):5.0;

    bool ccf_domain=(ccf_domain_flag>1)?true:false;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    if(!nt.listNeuron.size()) return;
    string out_nt_filename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_pruned.eswc");
    /*pruning very small branches*/
    float min_bouton_dist=8.0;
    nearBouton_pruning(nt,min_bouton_dist,ccf_domain);
    sparseBouton_pruning(nt,pruning_vol_r,pruning_vol_num,false);
    float pruning_thre=5.0;
    NeuronTree nt_internode_pruning=boutonSWC_internode_pruning(nt,pruning_thre,ccf_domain);
    NeuronTree nt_bouton_tip_pruning=tipNode_pruning(nt_internode_pruning,pruning_tip_thre,ccf_domain);

//    rendering_different_bouton(nt_bouton_tip_pruning);
    writeESWC_file(QString::fromStdString(out_nt_filename),nt_bouton_tip_pruning);
}
void sparseBouton_pruning(NeuronTree& nt,float pruning_dist,int pruning_num,bool ccf_domain){
    /*In the nearby area, if bouton number is below pruning_num, remove this bouton*/
    int btype_index=3;
    V3DLONG siz=nt.listNeuron.size();
    if(siz<=0){return;}
    vector<bool> pruning_flag(siz,false);

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.fea_val.at(btype_index)<BoutonType)
            continue;
        double vol_density=0.0;
        for(V3DLONG ib=0;ib<siz;ib++){
            NeuronSWC ss = nt.listNeuron.at(ib);
            if(ss.fea_val.at(btype_index)<BoutonType)
                continue;
            double b_dist=0.0;
            if(ccf_domain)
                b_dist=sqrt((s.fea_val.at(0)-ss.fea_val.at(0))*(s.fea_val.at(0)-ss.fea_val.at(0))+
                                 (s.fea_val.at(1)-ss.fea_val.at(1))*(s.fea_val.at(1)-ss.fea_val.at(2))+
                                 (s.fea_val.at(2)-ss.fea_val.at(2))*(s.fea_val.at(1)-ss.fea_val.at(2)));
            else
                b_dist=dis(s,ss);
            if(b_dist<=pruning_dist)
                vol_density+=1.0;
        }
        if(vol_density<=pruning_num)
            pruning_flag[i]=true;
    }
    //out
    V3DLONG removing_num=0;
    for(V3DLONG i=0;i<siz;i++){
        if(pruning_flag.at(i)){
            nt.listNeuron[i].type=2;
            nt.listNeuron[i].fea_val[btype_index]=0;
            removing_num++;
        }
    }
    cout<<"Removing "<<removing_num<<" sparse boutons."<<endl;
}
void nearBouton_pruning(NeuronTree& nt,float pruning_dist,bool ccf_domain){
    /*
     *  for bouton-site, compute the distance to it's parent-bouton-sites.
    */
    int btype_index=3;
    V3DLONG siz=nt.listNeuron.size();
    if(siz<=0){return;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }
    QHash <V3DLONG, V3DLONG>  child_parent_bouton;child_parent_bouton.clear();
    vector<float> dist_to_parent(siz,0.0);
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.parent<0||!hashNeuron.contains(s.parent))
            continue;
        float dist_to_parent_bouton=0.0;
        if(s.fea_val.at(btype_index)>=BoutonType){
            //start from a bouton
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=nt.listNeuron.at(pid);
            while(true){
                if(ccf_domain)
                    dist_to_parent_bouton+=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                                (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                                (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                else
                    dist_to_parent_bouton+=sqrt((s.x-sp.x)*(s.x-sp.x)+
                                                (s.y-sp.y)*(s.y-sp.y)+
                                                (s.z-sp.z)*(s.z-sp.z));

                if(sp.fea_val.at(btype_index)==BoutonType){
                    child_parent_bouton.insert(i,pid);
                    break;
                }
                if(sp.parent>0&&hashNeuron.contains(sp.parent)){
                    s=sp;
                    pid=hashNeuron.value(s.parent);
                    sp=nt.listNeuron.at(pid);
                }
                else{//can't find parent-bouton-node
                    dist_to_parent_bouton+=pruning_dist;
                    child_parent_bouton.insert(i,-1);
                    break;
                }
            }
            dist_to_parent[i]=dist_to_parent_bouton;
        }
    }
    vector<int> ntype(siz,0);    ntype=getNodeType(nt);
    vector<bool> pruning_flag(siz,false);
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
//        if(ntype.at(i)==2) {pruning_flag[i]=true;continue;}
        if(pruning_flag.at(i)) {continue;}
        if(s.fea_val.at(btype_index)>=BoutonType&&dist_to_parent.at(i)<pruning_dist
                &&child_parent_bouton.contains(i)){
            //compare with it's parent bouton, remove the one with small radius
            V3DLONG pid=child_parent_bouton.value(i);
            NeuronSWC sp=nt.listNeuron.at(pid);
            if(s.r<=sp.r)
                pruning_flag[i]=true;
            else
                pruning_flag[pid]=true;
        }
    }
    //make sure no duplicated boutons
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s1 = nt.listNeuron.at(i);
        if(s1.parent<0||!hashNeuron.contains(s1.parent))
            continue;
        if(s1.fea_val.at(btype_index)>=BoutonType){
            for(V3DLONG j=0;j<siz;j++){
                NeuronSWC s2 = nt.listNeuron.at(j);
                if(i!=j&&!pruning_flag.at(i)&&!pruning_flag.at(j)
                        &&s2.fea_val.at(btype_index)>=BoutonType)
                    if(sqrt((s1.x-s2.x)*(s1.x-s2.x)+
                            (s1.y-s2.y)*(s1.y-s2.y)+
                            (s1.z-s2.z)*(s1.z-s2.z))<1)
                        if(s1.r<=s2.r)
                            pruning_flag[i]=true;
                        else
                            pruning_flag[j]=true;
            }
        }
    }
    //out
    V3DLONG removing_num=0;
    for(V3DLONG i=0;i<siz;i++){
        if(pruning_flag.at(i)){
            nt.listNeuron[i].type=2;
            nt.listNeuron[i].fea_val[btype_index]=0;
            removing_num++;
        }
    }
    cout<<"Removing "<<removing_num<<" boutons."<<endl;
}
NeuronTree boutonSWC_internode_pruning(NeuronTree nt,float pruning_dist,bool ccf_domain){
    /*1. if pruning_dist<=0, keep branch nodes, soma node, tip nodes and bouton nodes
     * 2. if pruning_dist>0, pruning the internode distance below pruning_dist and keep bouton nodes
    */
    int btype_index=3;
    V3DLONG siz=nt.listNeuron.size();
    NeuronTree out;    if(siz<=0){return out;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }
    vector<int> ntype(siz,0);    ntype=getNodeType(nt);
    vector<int> nprocessed(siz,0);

    if(pruning_dist>0){
        for(V3DLONG i=0;i<siz;i++){
            NeuronSWC s = nt.listNeuron.at(i);
            if(nprocessed[i]==1)
                continue;
            if(s.parent>0&&hashNeuron.contains(s.parent)){
                V3DLONG pid=hashNeuron.value(s.parent);
                NeuronSWC sp=nt.listNeuron.at(pid);
                if(ntype.at(pid)==1&&sp.fea_val.at(btype_index)<BoutonType)
                {
                    double ssp_dist=0;
                    if(ccf_domain){
                        ssp_dist=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                              (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                              (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                    }
                    else
                        ssp_dist=sqrt((s.x-sp.x)*(s.x-sp.x)+
                                              (s.y-sp.y)*(s.y-sp.y)+
                                              (s.z-sp.z)*(s.z-sp.z));
                    if(ssp_dist<pruning_dist&&sp.parent>0){
                        nt.listNeuron[i].parent=sp.parent;
                        nprocessed[pid]=1;
                    }
                }
            }
        }
    }
    else{
        for(V3DLONG i=0;i<siz;i++){
            NeuronSWC s = nt.listNeuron.at(i);
            if(s.type==1&&s.parent<0)
                continue;
            if(nprocessed[i]==1)
                continue;
            //start from tip / branch point / bouton point
            AxonalBouton sb;sb.init_bouton(s);
            if(ntype.at(i)==0||ntype.at(i)==2||sb.btype>=BoutonType){
                //kept find parent node, until it is branch or Bouton
                if(!hashNeuron.contains(s.parent))
                    continue;
                V3DLONG pid=hashNeuron.value(s.parent);
                NeuronSWC sp=nt.listNeuron.at(pid);
                AxonalBouton spb;spb.init_bouton(sp);
                while(true){
                    if(ntype.at(pid)>=2||spb.btype>=BoutonType||
                            !hashNeuron.contains(sp.parent)||sp.parent<0){
                        nt.listNeuron[i].parent=sp.n;
                        break;
                    }
                    nprocessed[pid]=1;
                    s=sp;
                    pid=hashNeuron.value(s.parent);
                    sp=nt.listNeuron.at(pid);
                    spb.init_bouton(sp);
                }
            }
            else
            {
                nprocessed[i]=1;
                continue;
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
NeuronTree tipNode_pruning(NeuronTree nt, float pruning_dist,bool ccf_domain){
    /*for pruning of the redundancy nodes at tip-branch*/
    int btype_index=3;
    V3DLONG siz=nt.listNeuron.size();
    NeuronTree nt_out;
    if(siz==0){return nt_out;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }
     vector<int> ntype(siz,0);    ntype=getNodeType(nt);
     //tip branch
     vector<V3DLONG> kept(siz,1);

     for(V3DLONG i=0;i<siz;i++){
         NeuronSWC s=nt.listNeuron.at(i);
         if(s.fea_val.at(btype_index)<BoutonType&&
                 ntype.at(i)==0&&
                 s.parent>0&&hashNeuron.contains(s.parent)){
             QList<V3DLONG> scan_nodes; scan_nodes.clear();
             scan_nodes.append(i);
             V3DLONG pid=hashNeuron.value(s.parent);
             NeuronSWC sp=nt.listNeuron.at(pid);
             float dist_count=0.0;
             while(ntype.at(pid)==1){
                 if(ccf_domain)
                     dist_count+=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                      (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                      (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                 else
                     dist_count+=sqrt((s.x-sp.x)*(s.x-sp.x)+
                                      (s.y-sp.y)*(s.y-sp.y)+
                                      (s.z-sp.z)*(s.z-sp.z));
                 if(dist_count>pruning_dist){
                     scan_nodes.clear();
                     break;
                 }
                 if(sp.fea_val.at(btype_index)>=BoutonType){
                     //redundancy nodes
                     break;
                 }
                 else{
                     scan_nodes.append(pid);
                     s=sp;
                     if(s.parent>0&&hashNeuron.contains(s.parent)){
                         pid=hashNeuron.value(s.parent);
                         sp=nt.listNeuron.at(pid);
                     }
                     else{
                         scan_nodes.clear();
                         break;
                     }
                 }
             }
             if(scan_nodes.size()>0)
                 for(int si=0;si<scan_nodes.size();si++)
                     kept[scan_nodes.at(si)]=0;
         }
     }
     for(V3DLONG i=0;i<siz;i++){
         NeuronSWC s = nt.listNeuron.at(i);
         if(kept.at(i)>0){
             nt_out.listNeuron.append(s);
             nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
         }
     }
     return nt_out;
}

/*bouton features computing*/
void bouton_feature_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    /*get bouton related feature:
    1.  branch-order to `level`
    2.  bouton type distinguishing
    3.  bouton density  along axon and bouton density in volume
    4.  (ccf-domain)path distance to soma
    5.  (ccf-domain)distance to soma (Euler distance)
    6. normalize radius feature
    */
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}

    float xy_res=(inparas.size()>=1)?atof(inparas[0]):(0.3);
    float vol_radius=(inparas.size()>=2)?atof(inparas[1]):(50.0);
    int ccf_domain_flag=(inparas.size()>=3)?atoi(inparas[2]):0;

    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return;}
    //1.
    vector<int> norder(siz,0);
    if(!getNodeOrder(nt,norder)) {return;};
    for(V3DLONG i=0;i<siz;i++)
        nt.listNeuron[i].level=norder.at(i);
    //2.
    bool ccf_domain=(ccf_domain_flag>0)?true:false;
    boutonType_label(nt,xy_res,ccf_domain);
    //3. bouton_density feature to fea_val_list
    boutonDesity_computing(nt,xy_res,ccf_domain);
    boutonVolDesity_computing(nt,vol_radius,xy_res,ccf_domain);
    //4. distance
    dist_to_soma(nt,xy_res,ccf_domain,false);
    //5.
    for(V3DLONG i=0;i<siz;i++){
         nt.listNeuron[i].fea_val[4]*=xy_res;
         nt.listNeuron[i].fea_val[5]*=xy_res;
         nt.listNeuron[i].fea_val[6]*=xy_res;
    }
    QString out_nt_filename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_featured.eswc");
    writeESWC_file(out_nt_filename,nt);
}
void boutonType_label(NeuronTree& nt,float xy_res,bool ccf_domain){
    /*1. tip bouton: boutonType+=1
     * 2. terminaux bouton: boutonType+=1
        * for all the tip branch whose length are less than max_terminaux_bouton_branch_len,
        * if there only exist one bouton at the tip, this is Terminaux bouton.
    */
    int btype_index=3;    float z_res=1.0;
    float max_terminaux_bouton_branch_len=5.0;

    V3DLONG siz=nt.listNeuron.size();
    if(siz==0){return;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }
     vector<int> ntype(siz,0);    ntype=getNodeType(nt);
     //tip bouton
     for(V3DLONG i=0;i<siz;i++){
         NeuronSWC s=nt.listNeuron.at(i);
         if(ntype.at(i)==0&&s.fea_val.at(btype_index)==BoutonType)
             nt.listNeuron[i].fea_val[btype_index]+=1;
     }
     //terminaux bouton
     for(V3DLONG i=0;i<siz;i++){
         NeuronSWC s=nt.listNeuron.at(i);
         if(s.fea_val.at(btype_index)==(BoutonType+1)&&
                 s.parent>0&&hashNeuron.contains(s.parent)){
             //get branch distance
             V3DLONG pid=hashNeuron.value(s.parent);
             NeuronSWC sp=nt.listNeuron.at(pid);
             float dist_count=0.0;
             while(ntype.at(pid)>0){
                 if(ccf_domain)
                     dist_count+=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                      (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                      (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                 else
                     dist_count+=sqrt(xy_res*xy_res*(s.x-sp.x)*(s.x-sp.x)+
                                      xy_res*xy_res*(s.y-sp.y)*(s.y-sp.y)+
                                      z_res*z_res*(s.z-sp.z)*(s.z-sp.z));
                 if(ntype.at(pid)==2)
                     break;
                 //update node
                 s=sp;
                 if(s.parent<0||!hashNeuron.contains(s.parent)){
                     dist_count+=(1+max_terminaux_bouton_branch_len);
                     break;
                 }
                 pid=hashNeuron.value(s.parent);
                 sp=nt.listNeuron.at(pid);
             }
             if(dist_count<=max_terminaux_bouton_branch_len)
                 nt.listNeuron[i].fea_val[btype_index]+=1;
         }
     }
     //rendering at  swc node
}
void rendering_different_bouton(NeuronTree& nt, int type_bias){
    V3DLONG siz=nt.listNeuron.size();
    if(siz==0){return;}
    for(V3DLONG i=0;i<siz;i++){
        if(nt.listNeuron.at(i).fea_val.at(3)==BoutonType||nt.listNeuron.at(i).fea_val.at(3)==BoutonType+1)
            nt.listNeuron[i].type=type_bias+1;
        else if(nt.listNeuron.at(i).fea_val.at(3)==BoutonType+2)
            nt.listNeuron[i].type=type_bias+2;
    }
}
void boutonVolDesity_computing(NeuronTree& nt,float vol_r,float xy_res,bool ccf_domain){
    int vol_den_index=11;
    int btype_index=3;
    float z_res=1.0;
    V3DLONG siz=nt.listNeuron.size();
    if(siz<=0){return;}

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.fea_val.at(btype_index)<BoutonType)
            continue;
        double vol_density=0;
        for(V3DLONG ib=0;ib<siz;ib++){
            NeuronSWC ss = nt.listNeuron.at(ib);
            if(ss.fea_val.at(btype_index)<BoutonType)
                continue;
            double b_dist=0.0;
            if(ccf_domain)
                b_dist=sqrt((s.fea_val.at(0)-ss.fea_val.at(0))*(s.fea_val.at(0)-ss.fea_val.at(0))+
                                 (s.fea_val.at(1)-ss.fea_val.at(1))*(s.fea_val.at(1)-ss.fea_val.at(2))+
                                 (s.fea_val.at(2)-ss.fea_val.at(2))*(s.fea_val.at(1)-ss.fea_val.at(2)));
            else
                b_dist=sqrt(xy_res*xy_res*(s.x-ss.x)*(s.x-ss.x)+
                                 xy_res*xy_res*(s.y-ss.y)*(s.y-ss.y)+
                                 z_res*z_res*(s.z-ss.z)*(s.z-ss.z));
            if(b_dist<=vol_r)
                vol_density+=1.0;
        }
        nt.listNeuron[i].fea_val[vol_den_index]=float(1000.0)*vol_density/float(8.0*vol_r*vol_r*vol_r);
    }
}
void boutonDesity_computing(NeuronTree& nt,float xy_res,bool ccf_domain){
    /* 0. get bouton type
     * 1. for terminaux bouton, bouton density= branch length of this bouton.
     * 2. limit the calculation of en passant bouton density under a branch
            *get the branch length and the number of boutons at this branch
            * density= branch_len / #boutons
    */
    int density_index=10;
    int btype_index=3;
    float z_res=1.0;

    V3DLONG siz=nt.listNeuron.size();
    if(siz<=0){return;}
    vector<int> ntype(siz,0);    ntype=getNodeType(nt);
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.parent<0||!hashNeuron.contains(s.parent))
            continue;
        if(s.fea_val.at(btype_index)==(BoutonType+2)){
                //1. for terminax bouton
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=nt.listNeuron.at(pid);
            double t_density=0.0;
            while(true){
                if(ccf_domain)
                    t_density+=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                     (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                     (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                else
                    t_density+=sqrt(xy_res*xy_res*(s.x-sp.x)*(s.x-sp.x)+
                                     xy_res*xy_res*(s.y-sp.y)*(s.y-sp.y)+
                                     z_res*z_res*(s.z-sp.z)*(s.z-sp.z));
                if(ntype.at(pid)>=2)
                    break;
                //update node
                s=sp;
                if(s.parent<0||!hashNeuron.contains(s.parent)){
                    break;
                }
                pid=hashNeuron.value(s.parent);
                sp=nt.listNeuron.at(pid);
            }
            nt.listNeuron[i].fea_val[density_index]=1/t_density;
        }
    }
    //2. for en passant bouton
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.parent<0||!hashNeuron.contains(s.parent)||s.fea_val.at(btype_index)==BoutonType+2)
            continue;
        if(s.fea_val.at(density_index)>0)
            continue;
        if(ntype.at(i)==0||ntype.at(i)==2){
            //start from tip or branch point and end to branch or soma point
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=nt.listNeuron.at(pid);
            double b_length=0.0;
            QList<V3DLONG> scaned_bouton_list;
            scaned_bouton_list.clear();
            if(s.fea_val.at(btype_index)>=BoutonType)
                scaned_bouton_list.append(i);
            while(true){
                if(ccf_domain)
                    b_length+=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                     (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                     (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                else
                    b_length+=sqrt(xy_res*xy_res*(s.x-sp.x)*(s.x-sp.x)+
                                     xy_res*xy_res*(s.y-sp.y)*(s.y-sp.y)+
                                     z_res*z_res*(s.z-sp.z)*(s.z-sp.z));
                if(sp.fea_val.at(btype_index)>=BoutonType)
                    scaned_bouton_list.append(pid);
                if(ntype.at(pid)>=2)
                    break;
                //update node
                s=sp;
                if(s.parent<0||!hashNeuron.contains(s.parent)){break;}
                pid=hashNeuron.value(s.parent);
                sp=nt.listNeuron.at(pid);
            }
            if(scaned_bouton_list.size()>0)
                for(int sb=0;sb<scaned_bouton_list.size();sb++)
                    nt.listNeuron[scaned_bouton_list.at(sb)].fea_val[density_index]=(double)scaned_bouton_list.size()/b_length;
        }
    }
}
void dist_to_soma(NeuronTree& nt,float xy_res,bool ccf_domain,bool bouton_only){
    int path_dist_index=12;
    int euler_dist_index=13;
    int btype_index=3;
    float z_res=1.0;

    V3DLONG siz=nt.listNeuron.size();    if(siz<=0){return;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;    hashNeuron.clear();
    for(V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron.at(i).n,i);

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        NeuronSWC sraw=s;
        if(s.parent<0||!hashNeuron.contains(s.parent))
            continue;
        bool tmp_flag=(bouton_only)?(s.fea_val.at(btype_index)>=BoutonType):true;
        if(tmp_flag)
        {
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=nt.listNeuron.at(pid);
            double path_length=0.0;
            double euler_dist=0.0;
            while(true){
                if(ccf_domain)
                    path_length+=sqrt((s.fea_val.at(0)-sp.fea_val.at(0))*(s.fea_val.at(0)-sp.fea_val.at(0))+
                                     (s.fea_val.at(1)-sp.fea_val.at(1))*(s.fea_val.at(1)-sp.fea_val.at(2))+
                                     (s.fea_val.at(2)-sp.fea_val.at(2))*(s.fea_val.at(1)-sp.fea_val.at(2)));
                else
                    path_length+=sqrt(xy_res*xy_res*(s.x-sp.x)*(s.x-sp.x)+
                                     xy_res*xy_res*(s.y-sp.y)*(s.y-sp.y)+
                                     z_res*z_res*(s.z-sp.z)*(s.z-sp.z));
                //update node
                s=sp;
                if(s.parent<0||!hashNeuron.contains(s.parent)){
                    if(ccf_domain)
                        euler_dist=sqrt((s.fea_val.at(0)-sraw.fea_val.at(0))*(s.fea_val.at(0)-sraw.fea_val.at(0))+
                                        (s.fea_val.at(1)-sraw.fea_val.at(1))*(s.fea_val.at(1)-sraw.fea_val.at(2))+
                                        (s.fea_val.at(2)-sraw.fea_val.at(2))*(s.fea_val.at(1)-sraw.fea_val.at(2)));
                    else
                        euler_dist=sqrt(xy_res*xy_res*(s.x-sraw.x)*(s.x-sraw.x)+
                                         xy_res*xy_res*(s.y-sraw.y)*(s.y-sraw.y)+
                                         z_res*z_res*(s.z-sraw.z)*(s.z-sraw.z));
                    break;
                }
                pid=hashNeuron.value(s.parent);
                sp=nt.listNeuron.at(pid);
            }
            nt.listNeuron[i].fea_val[path_dist_index]=path_length;
            nt.listNeuron[i].fea_val[euler_dist_index]=euler_dist;
        }
    }
}
/*File io*/
void bouton_file_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    /*filetype=0, apo file in raw brain
     * filetype=1, (terafly) 3d image block, 2d mip image block and cropped swc file
     * filetype=2, (image block) 3d image block, 2d mip image block and cropped swc file
     * filetype=3, marker file in ccf
     * filetype=4,  swc file in ccf
     * filetype=5, bouton features in csv table
     * filetype=6, swc internode pruning; default: only keep branch, tip,soma and bouton node
     * filetype=other, {0, 2 and 3}
    */
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    NeuronTree nt = readSWC_file(inswc_file);
    if(!nt.listNeuron.size()) return;

    int filetype=(inparas.size()>=1)?atoi(inparas[0]):-1;
    switch (filetype) {
    case 0:
    {
        QList<CellAPO> apoboutons=bouton_to_apo(nt);
        QString apofilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ ".apo");
        writeAPO_file(apofilename,apoboutons);
    }
        break;
    case 1:
    {
        //must input image
        string inimg_file;
        if(infiles.size()>=2)
            inimg_file = infiles[1];
        else
            return;
        //read input paras
        /*all the boutons: getbtype=1;
         * tip-boutons: getbtype=2;
         * TEB: getbtype=3;
        */
        int getbtype=(inparas.size()>=2)?atoi(inparas[1]):1;
        int crop_half_size=(inparas.size()>=3)?atoi(inparas[2]):16;
        bool mip_flag=((inparas.size()>=4)?atoi(inparas[3]):1)>0?true:false;
        int maske_size=(inparas.size()>=5)?atoi(inparas[4]):0;

        QString outpath=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
        getBoutonBlock(callback,inimg_file,nt,outpath,getbtype,crop_half_size,mip_flag,maske_size);
    }
        break;
    case 3:
    {
        QString mfilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ "_ccf.marker");
        QList<ImageMarker> mboutons=bouton_to_imageMarker(nt);
        writeMarker_file(mfilename,mboutons);
    }
        break;
    case 4:
    {
        float radius_scale=(inparas.size()>=2)?atof(inparas[1]):1.0;
        QString ccfname=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ "_ccf.eswc");
        boutonswc_to_ccf(nt,radius_scale);
        writeESWC_file(ccfname,nt);
    }
        break;
    case 5:
    {
        float res[3];
        res[0]=(inparas.size()>=2)?atof(inparas[1]):(0.3);
        res[1]=res[0]; res[2]=1.0;
        QString outfilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_ft.csv");
        featureTable(outfilename,nt,res);
    }
        break;
    case 6:
    {
        float pruning_thre=(inparas.size()>=2)?atof(inparas[1]):0;
        QString outname=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ "_pruned.eswc");
        NeuronTree nt_out=boutonSWC_internode_pruning(nt,pruning_thre);
        writeESWC_file(outname,nt_out);
    }
        break;
    default:
    {
        float radius_scale=(inparas.size()>=2)?atof(inparas[1]):1.0;
        QList<CellAPO> apoboutons=bouton_to_apo(nt);
        QList<ImageMarker> mboutons=bouton_to_imageMarker(nt);
        boutonswc_to_ccf(nt,radius_scale);
        QString apofilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ ".apo");
        QString mfilename=(outfiles.size()>=2)?outfiles[1]:(inswc_file+ "_ccf.marker");
        QString ccfname=(outfiles.size()>=3)?outfiles[2]:(inswc_file+ "_ccf.eswc");
        writeAPO_file(apofilename,apoboutons);
        writeMarker_file(mfilename,mboutons);
        writeESWC_file(ccfname,nt);
    }
        break;
    }
}
void boutonswc_to_ccf(NeuronTree& nt,float scale){
    V3DLONG siz=nt.listNeuron.size();
    for(V3DLONG i=0;i<siz;i++){
        if(nt.listNeuron.at(i).fea_val.size()>=5){
            nt.listNeuron[i].x=nt.listNeuron[i].fea_val[0];
            nt.listNeuron[i].y=nt.listNeuron[i].fea_val[1];
            nt.listNeuron[i].z=nt.listNeuron[i].fea_val[2];
            nt.listNeuron[i].r=nt.listNeuron[i].fea_val[4]/scale;
        }
    }
}
QList<CellAPO> bouton_to_apo(NeuronTree nt){
    /*this function will generate bouton.apo file and bouton.marker file*/
    V3DLONG siz=nt.listNeuron.size();
    QList<CellAPO> boutons; boutons.clear();
    // to apo
     for(V3DLONG i=0;i<siz;i++){
         NeuronSWC s=nt.listNeuron.at(i);
         AxonalBouton sb; sb.init_bouton(s);
         if(sb.btype<BoutonType){continue;}
         CellAPO bca=sb.out_to_APO();
         bca.n=boutons.size()+1;
         boutons.append(bca);
     }
     return boutons;
}
QList<ImageMarker> bouton_to_imageMarker(NeuronTree nt){
    /*this function will generate bouton.marker file*/
    V3DLONG siz=nt.listNeuron.size();
    QList<ImageMarker> boutons; boutons.clear();
    // to apo
     for(V3DLONG i=0;i<siz;i++){
         NeuronSWC s=nt.listNeuron.at(i);
         AxonalBouton sb; sb.init_bouton(s);
         if(sb.btype<BoutonType){continue;}
         ImageMarker bim=sb.out_to_ImageMarker();
         boutons.append(bim);
     }
     return boutons;
}
void featureTable(const QString &filename,NeuronTree nt,float *res){
    V3DLONG siz=nt.listNeuron.size();
    QFile tofile(filename);
    if(tofile.exists())
        cout<<"File overwrite to "<<filename.toStdString()<<endl;
    QString fb=QFileInfo(filename).baseName();
    //global features
    QString notes="file,Nlength,Ntips,Nradius_mean,Nradius_std,Max_order,Max_pathdist,Max_dist,";
    notes+="NBs,NEnBs,NTeBs,NTiBs,NB_R,NEnB_R,NTeB_R,NTiB_R,NB_density,NB_vol_density,NBorder,NB_pathdist,NB_dist\n";
    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        tofile.write(notes.toAscii());
    }
    vector<int> ntype(siz,0);    ntype=getNodeType(nt);
    double ntlen=getNT_len(nt,res);
    //branches and tips
    int ntbr=-2;    int nttips=0;
    //nt radius mean and std
    float ntr_mean=0.0;
    int max_br_order=0;
    float max_path_dist=0.0; float max_dist=0.0;

    //global feature of bouton
    V3DLONG ntbs,ntebs,nttebs,nttipbs;
    ntbs=ntebs=nttebs=nttipbs=0;

    float ntbs_orders,ntbs_path_dist,ntbs_euler_dist,
            ntbs_rmean,ntenbs_rmean,nttebs_rmean,ntibs_rmean,ntbs_dmean,ntbs_vdmean;
    ntbs_orders=ntbs_path_dist=ntbs_euler_dist= ntbs_dmean=ntbs_vdmean=
            ntbs_rmean=ntenbs_rmean=nttebs_rmean=ntibs_rmean=0.0;

    for(V3DLONG i=0;i<siz;i++){
        ntbr+=((ntype.at(i)>=2)?(ntype.at(i)):0);
        nttips+=((ntype.at(i)==0)?1:0);
        ntr_mean+=nt.listNeuron.at(i).r*res[0];
        max_br_order=MAX(nt.listNeuron.at(i).level,max_br_order);
        NeuronSWC s=nt.listNeuron.at(i);
        AxonalBouton sb; sb.init_bouton(s);
        max_path_dist=MAX(max_path_dist,sb.path_dist_to_soma);
        max_dist=MAX(max_dist,sb.euler_dist_to_soma);
        if(sb.btype>=BoutonType){
            ntbs+=1;
            ntebs+=(sb.btype<BoutonType+2?1:0);
            nttebs+=(sb.btype==BoutonType+2?1:0);
            nttipbs+=(sb.btype>BoutonType?1:0);
            ntbs_orders+=sb.broder;
            ntbs_path_dist+=sb.path_dist_to_soma;
            ntbs_euler_dist+=sb.euler_dist_to_soma;
            ntbs_rmean+=sb.ccfR;
            ntenbs_rmean+=((sb.btype<BoutonType+2)?sb.ccfR:0);
            nttebs_rmean+=((sb.btype==BoutonType+2)?sb.ccfR:0);
            ntibs_rmean+=((sb.btype>BoutonType)?sb.ccfR:0);
            ntbs_dmean+=sb.density;
            ntbs_vdmean+=sb.vol_density;
        }
    }
    ntr_mean/=float(siz);
    float ntr_std=0;
    for(int j=0;j<siz;j++)
        ntr_std+=(nt.listNeuron.at(j).r*res[0]-ntr_mean)*(nt.listNeuron.at(j).r*res[0]-ntr_mean);
    ntr_std/=float(siz-1);             ntr_std=sqrt(ntr_std);

    ntbs_orders/=(float)ntbs;
    ntbs_path_dist/=(float)ntbs;
    ntbs_euler_dist/=(float)ntbs;
    ntbs_rmean/=(float)ntbs;
    ntenbs_rmean/=(float)ntebs;
    nttebs_rmean/=(float)nttebs;
    ntibs_rmean/=(float)nttipbs;
    ntbs_dmean/=(float)ntbs;
    ntbs_vdmean/=(float)ntbs;
    QString outgfline=fb+","+
            QString::number(ntlen)+","+
            QString::number(nttips)+","+
            QString::number(ntr_mean)+","+
            QString::number(ntr_std)+","+
            QString::number(max_br_order)+","+
            QString::number(max_path_dist)+","+
            QString::number(max_dist)+","+
            QString::number(ntbs)+","+
            QString::number(ntebs)+","+
            QString::number(nttebs)+","+
            QString::number(nttipbs)+","+
            QString::number(ntbs_rmean)+","+
            QString::number(ntenbs_rmean)+","+
            QString::number(nttebs_rmean)+","+
            QString::number(ntibs_rmean)+","+
            QString::number(ntbs_dmean)+","+
            QString::number(ntbs_vdmean)+","+
            QString::number(ntbs_orders)+","+
            QString::number(ntbs_path_dist)+","+
            QString::number(ntbs_euler_dist)+"\n";
    tofile.write(outgfline.toAscii());
    //all bouton features
    QString confTitle="n,type,tip,br_order,density,vol_density,path_dist_to_soma,dist_to_soma,radius,br_rmean,br_rstd,I,br_Imean,br_Istd,x,y,z,ccfx,ccfy,ccfz\n";
    tofile.write(confTitle.toAscii());
    V3DLONG bn=0;
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        AxonalBouton sb; sb.init_bouton(s);
        if(sb.btype<BoutonType){continue;}
        bn++;
        QString btype=(sb.btype<BoutonType+2)?"En-passant-bouton":"Terminaux-bouton";
        QString btip=(sb.btype>BoutonType)?"Tip":"Internal";
        QString outline
                =(QString::number(bn)+","
                  +btype+","
                  +btip+","
                  +QString::number(sb.broder)+","
                  +QString::number(sb.density)+","
                  +QString::number(sb.vol_density)+","
                  +QString::number(sb.path_dist_to_soma)+","
                  +QString::number(sb.euler_dist_to_soma)+","
                  +QString::number(sb.ccfR)+","
                  +QString::number(sb.br_r_mean)+","
                  +QString::number(sb.br_r_std)+","
                  +QString::number(sb.intensity)+","
                  +QString::number(sb.br_intensity_mean)+","
                  +QString::number(sb.br_intensity_std)+","
                  +QString::number(sb.x)+","
                  +QString::number(sb.y)+","
                  +QString::number(sb.z)+","
                  +QString::number(sb.ccfx)+","
                  +QString::number(sb.ccfy)+","
                  +QString::number(sb.ccfz)+"\n");
        tofile.write(outline.toAscii());
    }
    tofile.close();
}
void getBoutonBlock_inImg(V3DPluginCallback2 &callback,string inimg_file,QList <CellAPO> apolist,string outpath,int block_size)
{
    /*crop 3d image-block
     * get mip img
     * not finished
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
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,NeuronTree nt,QString outpath,
                    int getbtype,int crop_half_size,bool mip_flag,int mask_size)
{
    cout<<"crop bouton block from terafly dataset"<<endl;

    V3DLONG siz = nt.listNeuron.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz)){
        cout<<"can't load terafly img"<<endl;
        return;
    }
    QString save_path_3d = outpath+"/3d";
    QDir path(save_path_3d);
    if(!path.exists()) {
        cout<<"make a new dir for saving 3D blocks "<<endl;
        path.mkpath(save_path_3d);
    }
    QString save_path_traced = outpath+"/swc";
    QDir path_traced(save_path_traced);
    if(!path_traced.exists()) {
        cout<<"make a new dir for saving traced nodes"<<endl;
        path_traced.mkpath(save_path_traced);
    }
    QString save_path_mip="";
    if(mip_flag){
         save_path_mip= outpath+"/mip";
        QDir path_mip(save_path_mip);
        if(!path_mip.exists()) {
            cout<<"make a new dir for saving mip blocks "<<endl;
            path_mip.mkpath(save_path_mip);
        }
    }

    for(V3DLONG i=0;i<siz;i++){
       NeuronSWC s=nt.listNeuron.at(i);
       AxonalBouton sb; sb.init_bouton(s);
       if(sb.btype>=getbtype){
           V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
           start_x = s.x - crop_half_size; if(start_x<0) start_x = 0;
           end_x = s.x + crop_half_size; if(end_x >= in_zz[0]) end_x = in_zz[0]-1;
           start_y =s.y - crop_half_size;if(start_y<0) start_y = 0;
           end_y = s.y + crop_half_size;if(end_y >= in_zz[1]) end_y = in_zz[1]-1;
           start_z = s.z - crop_half_size;if(start_z<0) start_z = 0;
           end_z = s.z + crop_half_size;if(end_z >= in_zz[2]) end_z = in_zz[2]-1;
           V3DLONG *in_sz = new V3DLONG[4];
           in_sz[0] = end_x-start_x+1;
           in_sz[1] = end_y-start_y+1;
           in_sz[2] = end_z-start_z+1;
           in_sz[3]=in_zz[3];
           unsigned char * im_cropped = 0;
           V3DLONG pagesz;
           pagesz = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
           try {im_cropped = new unsigned char [pagesz];}
           catch(...)  {cout<<"cannot allocate memory for cropping."<<endl; return;}
           im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
           if(im_cropped==NULL) { cout<<"crop "<<s.n<<" fail"<<endl;continue;}

           QString tmpstr = "";
           tmpstr.append("_n_").append(QString("%1").arg(s.n));
           tmpstr.append("_x_").append(QString("%1").arg(s.x));
           tmpstr.append("_y_").append(QString("%1").arg(s.y));
           tmpstr.append("_z_").append(QString("%1").arg(s.z));
           //crop swc block
           NeuronTree nt_saved;
           nt_saved.listNeuron.clear();nt_saved.hashNeuron.clear();
           if(true)
           {
               NeuronTree out;
               out.listNeuron.clear();out.hashNeuron.clear();
               for(V3DLONG j=0;j<siz;j++)
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
               if(!out.listNeuron.size()){cout<<"can not find swc node inside this block"<<endl;;continue;}
               cout<<"raw swc nodes="<<out.listNeuron.size()<<endl;
               //filter out unrelated swc nodes

               for(V3DLONG j=0;j<out.listNeuron.size();j++){
                   NeuronSWC so=out.listNeuron.at(j);
                   bool kept=false;
                   if(so.n==nt.listNeuron.at(i).n)
                       kept=true;
                   //for those nodes, whose parent- node are bouton
                   if(so.parent>0&&out.hashNeuron.contains(so.parent)&&!kept){
                       V3DLONG sop_id=out.hashNeuron.value(so.parent);
                       NeuronSWC sop=out.listNeuron.at(sop_id);
                       while(true){
                           if(sop.n==nt.listNeuron.at(i).n){kept=true;break;}
                           so=sop;
                           if(so.parent<0||!out.hashNeuron.contains(so.parent)){kept=false;break;}
                           sop_id=out.hashNeuron.value(so.parent);
                           sop=out.listNeuron.at(sop_id);
                       }
                   }
                   if(kept){
                       nt_saved.listNeuron.append(out.listNeuron.at(j));
                       nt_saved.hashNeuron.insert(out.listNeuron[j].n,nt_saved.listNeuron.size()-1);
                   }
               }
               //for those nodes, whose node are parent- node of bouton.
               NeuronSWC ss=nt.listNeuron.at(i);
               while(ss.parent>0&&out.hashNeuron.contains(ss.parent)){
                   V3DLONG ssp_id=out.hashNeuron.value(ss.parent);
                   NeuronSWC ssp=out.listNeuron.at(ssp_id);
                   nt_saved.listNeuron.append(ssp);
                   nt_saved.hashNeuron.insert(ssp.n,nt_saved.listNeuron.size()-1);
                   ss=ssp;
               }
           }
           if(nt_saved.listNeuron.size()){
               cout<<"final out swc size="<<nt_saved.listNeuron.size()<<endl;
               //save to file
               QString default_name="swc"+tmpstr+".eswc";
               QString save_path_swc =save_path_traced+"/"+default_name;
               writeESWC_file(save_path_swc,reindexNT(nt_saved));
           }
           //3d image block
           QString default_name = "Bouton"+tmpstr+".tif";
           QString save_path_img =save_path_3d+"/"+default_name;
           if(mask_size>0){
               unsigned char * im_masked = 0;
               V3DLONG pagesz;               pagesz = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
               try {im_masked = new unsigned char [pagesz];}
               catch(...)  {cout<<"cannot allocate memory for masking image."<<endl; return;}
               maskImg(im_cropped,im_masked,in_sz,nt_saved,mask_size);
               simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_masked,in_sz,1);
               //2d mip
               if(mip_flag)
               {
                   QString mip_save_path_img =save_path_mip+"/"+"Bouton"+tmpstr+".tif";
                   getBoutonMIP(callback,im_masked,in_sz, mip_save_path_img);
               }
               if(im_masked) {delete []im_masked; im_masked = 0;}
           }
           else
           {
               simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
               //2d mip
               if(mip_flag)
               {
                   QString mip_save_path_img =save_path_mip+"/"+"Bouton"+tmpstr+".tif";
                   getBoutonMIP(callback,im_cropped,in_sz, mip_save_path_img);
               }
           }
           if(im_cropped) {delete []im_cropped; im_cropped = 0;}
       }
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
void maskImg(unsigned char *&inimg1d,unsigned char * & im_transfer, long in_sz[], NeuronTree nt, int maskRadius)
{
    /*for all the pixels in the dst block*/
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = listNeuron.size();
    if(siz<1) return;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    int startx,starty,startz;
    int endx,endy,endz;

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
        end_x = s.x + cropx/2; if(end_x >= in_zz[0]) end_x = in_zz[0]-1;
        start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
        end_y = s.y + cropy/2;if(end_y >= in_zz[1]) end_y = in_zz[1]-1;
        start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
        end_z = s.z + cropz/2;if(end_z >= in_zz[2]) end_z = in_zz[2]-1;
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

/*radius estimation*/
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
    int cropped_block_size=16; int zcropped_block_size=3;
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
    dynamic_s.x-=float(start_x); dynamic_s.y-=float(start_y); dynamic_s.z-=float(start_z);

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
double radiusEstimation(V3DPluginCallback2 &callback,unsigned char *&inimg1d, long in_zz[], NeuronSWC s, double dfactor, double bkg_thresh){
    //for debug
    NeuronSWC dynamic_s=s;
    //1. center_block cropped
    int cropped_block_size=16; int zcropped_block_size=3;
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
    dynamic_s.x-=float(start_x); dynamic_s.y-=float(start_y); dynamic_s.z-=float(start_z);
    //debug:save
//    string out_image_file="/home/penglab/PBserver/SD-Jiang/Bouton/Validation/radius/upfactor/crop.v3draw";
//    simple_saveimage_wrapper(callback, (char*)out_image_file.c_str(),(unsigned char *)im_cropped, crop_sz, 1);

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
    cout<<"XY: "<<dynamic_s.x<<", "<<dynamic_s.y<<endl;
    dynamic_s.x*=dfactor; dynamic_s.y*=dfactor; dynamic_s.z*=1;
    //release center_block
    if(im_cropped) {delete []im_cropped; im_cropped = 0;}

    //debug:save
//    out_image_file="/home/penglab/PBserver/SD-Jiang/Bouton/Validation/radius/upfactor/up_sampled.tif";
//    simple_saveimage_wrapper(callback, (char*)out_image_file.c_str(),(unsigned char *)image_resampled, upsampled_sz, 1);

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

    //debug:save
    string out_image_file="/home/penglab/PBserver/SD-Jiang/Bouton/Validation/radius/upfactor/up_sampled_mip.v3draw";
    simple_saveimage_wrapper(callback, (char*)out_image_file.c_str(),(unsigned char *)image_mip, mip_sz, 1);


    //4. radius estimation
    cout<<"New XY: "<<dynamic_s.x<<", "<<dynamic_s.y<<",";
    double upRadius=getNodeRadius_XY(image_mip,mip_sz,dynamic_s,bkg_thresh);
    cout<<"R="<<upRadius<<endl;
    cout<<"post level="<<int(image_mip[long(dynamic_s.y)*mip_sz[0] + long(dynamic_s.x)])<<endl;
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

void enhanceImage(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz,bool biilateral_filter){

    double gain=5, cutoff=25;
    double spaceSigmaXY, spaceSigmaZ, colorSigma=35;

    int k_sz[3] = {3, 3, 1};

    spaceSigmaXY = k_sz[0]/3.0;
    spaceSigmaZ = k_sz[2]/3.0;

    V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];

    cout<<"adaptive thresholding "<<endl;
    adaptiveThresholding(data1d,dst,mysz);
    for(V3DLONG i=0; i<tolSZ; i++)
        data1d[i]=dst[i];

    sigma_correction(data1d, mysz, cutoff, gain, dst, 1);
    cout<<"finish sigma correction "<<endl;

    for(V3DLONG i=0; i<tolSZ; i++)
        data1d[i]=dst[i];

    cout<<"do subtract min"<<endl;
    subtract_min(data1d, mysz, dst);
    cout<<"finish subtract min"<<endl;

    for(V3DLONG i=0; i<tolSZ; i++)
        data1d[i]=dst[i];
    if(biilateral_filter){
        cout<<"do bilateral filter "<<endl;
        bilateralfilter(data1d, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 1);
        cout<<"finish bilateral filter "<<endl;
    }

    cout<<"do intensity rescale"<<endl;
    intensity_rescale(data1d, mysz, dst, 1);
    cout<<"finish intensity rescale"<<endl;
}
void adaptiveThresholding(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz)
{
    V3DLONG h = 5, d = 3;
    V3DLONG iImageWidth, iImageHeight,iImageLayer;
    iImageWidth=mysz[0];
    iImageHeight=mysz[1];
    iImageLayer=mysz[2];

    V3DLONG i, j,k,n,count;
    double t, temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n =1 ; n <= d  ;n++)
                {
                    if (k>h*n) {temp += data1d[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) { temp += data1d[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += data1d[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += data1d[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += data1d[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += data1d[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  data1d[curpos]-temp/(count);
                dst[curpos]= (t > 0)? t : 0;
            }
        }
    }
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
void printHelp()
{
    cout<<"Please find README.md in project folder"<<endl;
}
QList <CellAPO> nt_2_multi_centers(NeuronTree nt,float xs,float ys,float zs){
    QList <CellAPO> nt_centers; nt_centers.clear();
    V3DLONG niz=nt.listNeuron.size();
    if(niz<=0)
        return nt_centers;
    float xmin,ymin,zmin,xmax,ymax,zmax;
    xmin=xmax=nt.listNeuron.at(0).x;
    ymin=ymax=nt.listNeuron.at(0).y;
    zmin=zmax=nt.listNeuron.at(0).z;
    for(V3DLONG i=0;i<niz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        xmin=MIN(s.x,xmin); xmax=MAX(s.x,xmax);
        ymin=MIN(s.y,ymin); ymax=MAX(s.y,ymax);
        zmin=MIN(s.z,zmin); zmax=MAX(s.z,zmax);
    }
    for(float xc=xs+xmin;xc<=xmax;xc+=xs)
        for(float yc=ys+ymin;yc<=ymax;yc+=ys)
            for(float zc=zs+zmin;zc<=zmax;zc+=zs)
            {
                bool validated=false;
                for(V3DLONG i=0;i<niz;i++)
                {
                    NeuronSWC s = nt.listNeuron[i];
                    if(s.x>=xc&&s.x<=(xc+xs)&&
                            s.y>=yc&&s.y<=(yc+ys)&&
                            s.z>=zc&&s.z<=(zc+zs)){
                        validated=true;
                        break;
                    }
                }
                CellAPO scenter;
                scenter.x=xc+xs/2;
                scenter.y=yc+ys/2;
                scenter.z=zc+zs/2;
                nt_centers.append(scenter);
            }
    cout<<"neuron tree is split into "<<nt_centers.size()<<endl;
    return nt_centers;
}
