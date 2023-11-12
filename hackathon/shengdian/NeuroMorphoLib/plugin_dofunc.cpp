#include "plugin_dofunc.h"
bool swc_parallization(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /*preprocess:
     * soma checking
     * loop checking
     * multiple bifurcation checking
     *pruning small tip branch
     * pruning internal node
     * resample node
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}


    bool connect=(((inparas.size()>=1)?atoi(inparas[0]):0)>0)?true:false;
    int three_bifs_label=(inparas.size()>=2)?atoi(inparas[1]):0;
    int tip_br_thre=/*(inparas.size()>=1)?atoi(inparas[0]):*/6;
    float internode_thre=/*(inparas.size()>=2)?atof(inparas[1]):*/1.5;
    int resample_pixels=/*(inparas.size()>=1)?atoi(inparas[0]):*/4;

    cout<<"swc file="<<inswc_file<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".eswc");

    writeESWC_file(QString::fromStdString(out_f),nt);
    return true;
}
bool swc_ada_sampling(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* resample swc based on curvature, thickness and intensity of curve
     *
     * fitting strategy: linear interpolation
     * step:
     * 1, swc2branches
     * 2, for each branch, from tip to head
     * 3,
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
//    int sample_feas=(inparas.size()>=1)?atoi(inparas[0]):0;
    float curvature_ratio=(inparas.size()>=1)?atof(inparas[0]):1.0;
    float dist_thre=(inparas.size()>=2)?atof(inparas[1]):2.0;
    float thickness_ratio=(inparas.size()>=3)?atof(inparas[2]):1.0;
    float r_thre=(inparas.size()>=1)?atof(inparas[0]):0.5;
    float intensity_ratio=(inparas.size()>=2)?atof(inparas[1]):1.0;
    float i_thre=(inparas.size()>=3)?atof(inparas[2]):30;

    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    NeuronTree nt_out;
    if(ada_sampling(nt,nt_out,
                    curvature_ratio,dist_thre,
                    thickness_ratio,r_thre,
                    intensity_ratio,i_thre))
    {
        string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".eswc");
        writeESWC_file(QString::fromStdString(out_f),nt);
        return true;
    }
    return false;
}
bool ada_sampling(NeuronTree nt, NeuronTree& nt_sampled,
                  float curvature_ratio, float dist_thre,
                  float thickness_ratio, float r_thre,
                  float intensity_ratio, float i_thre)
{
//    unit max_pnodes=10;
//    BranchTree bt; bt.initialized=bt.init(nt);
//    if(!bt.initialized){return false;}
//    if (bt.listBranch.size()==0) { return false;}
//    V3DLONG bsiz=bt.listBranch.size();
//    for(V3DLONG i=0;i<bsiz;i++)
//    {
//        BranchUnit bu = bt.listBranch.at(i);
//        //start from tip
//        for(V3DLONG n=bu.listNode.size()-2;n>0;n--){
//            for(uint p=1;p<max_pnodes;p++){
//                NeuronSWC s0=bu.listNode.at(n);
//            }
//        }

//    }
//    if(!branchTree2NeuronTree(bt,nt_sampled))
//        return false;
    return true;
}

bool soma_connection(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* soma connection:
     * 1. 多分叉点为soma点，但soma已经定义了，并在多分叉点的附近。
     * 2. 多分叉点为soma点，soma未定义
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    double s_connect_dist=(inparas.size()>=1)?atof(inparas[0]):20;
    int save_new=(inparas.size()>=2)?atoi(inparas[1]):0;

    cout<<"swc file="<<inswc_file.toStdString()<<endl;
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    QString file_suffix=QFileInfo(inswc_file).suffix();
    cout<<"save to "<<file_suffix.toStdString()<<endl;
    QString out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "."+file_suffix);

    // step1: try to identify soma and make a connection of duplicated nodes
    V3DLONG somaid=get_soma(nt,true,true);
    if(somaid<0){
        cout<<"No soma was defined in type(1). Now try to check is there only one -1."<<endl;
        somaid=get_soma(nt,false,false);
    }
    // step2: check the number of start points
    vector<int> nchilds(siz,0); // how many child-nodes: index -> node_type
    QList<V3DLONG> spoints; // used to identify start points.
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        if(nt.hashNeuron.contains(s.pn))
        {
            V3DLONG spn_id=nt.hashNeuron.value(s.pn);
            nchilds[spn_id]+=1;
        }
        else
            spoints.append(i);
    }
    cout<<"# of start points in this neuron tree: "<<spoints.size()<<endl;
    //check multiple-bifurcation points
    QList<V3DLONG> bifur_idlist; bifur_idlist.clear();
    for (V3DLONG i=0;i<siz;i++)
        if(nchilds.at(i)>2)
            bifur_idlist.append(i);
    cout<<"# of multiple bifurcations ="<<bifur_idlist.size()<<endl;
    if(spoints.size()>1){
        // in this way, soma-id might not be included in spoints. keep this in mind
        cout<<"not a single tree"<<endl;
        // check the distance between those start points
        double s_dist=0;
        for(V3DLONG i=0;i<spoints.size();i++)
        {
            for(V3DLONG j=0;j<spoints.size();j++)
            {
                if(i>=j)
                    continue;
                double tmps_dist=dis(nt.listNeuron.at(spoints.at(i)),
                                     nt.listNeuron.at(spoints.at(j)));
                if(s_dist<tmps_dist)
                    s_dist=tmps_dist;
            }
        }
        if(s_dist>s_connect_dist){
            cout<<"The  max distance ("<<s_dist<<") between start points is large than threshold_dist: "<<s_connect_dist<<endl;
            return false;
        }
        else{
            cout<<"Try to connect sperated starting points"<<endl;
            NeuronSWC new_s=nt.listNeuron.at(spoints.at(0));
            for(V3DLONG i=1;i<spoints.size();i++){
                new_s.x+=nt.listNeuron.at(spoints.at(i)).x;
                new_s.y+=nt.listNeuron.at(spoints.at(i)).y;
                new_s.z+=nt.listNeuron.at(spoints.at(i)).z;
            }
            new_s.x/=double(spoints.size());
            new_s.y/=double(spoints.size());
            new_s.z/=double(spoints.size());
            new_s.pn=-1; new_s.type=1;
            //identify new index
            NeuronTree nt_new;
            for (V3DLONG i=0;i<siz;i++){
                if(!nt.hashNeuron.contains(i)){
                    new_s.n=siz+i;
                    break;
                }
            }
            nt_new.listNeuron.append(new_s);
            nt_new.hashNeuron.insert(new_s.n,0);
            for (V3DLONG i=0;i<siz;i++){
                NeuronSWC s = nt.listNeuron.at(i);
                if(spoints.contains(i))
                    s.pn=new_s.n;
                nt_new.listNeuron.append(s);
                nt_new.hashNeuron.insert(s.n,nt_new.listNeuron.size()-1);
            }
            //save
            NeuronTree nt_renew=reindexNT(nt_new);
            if(!save_new){
                if(file_suffix.toLower()==QString("eswc"))
                    writeESWC_file(out_f,nt_renew);
                else
                    writeSWC_file(out_f,nt_renew);
            }
        }
    }else{
        QList<V3DLONG> refine_bifs; refine_bifs.clear();
        if(spoints.at(0)==somaid){
            //normal status
            if(bifur_idlist.contains(somaid))
                return true;
            if(bifur_idlist.size()==1){
                //check distance between soma and bifurcation point
                double sb_dist=dis(nt.listNeuron.at(somaid),nt.listNeuron.at(bifur_idlist.at(0)));
                if(sb_dist<s_connect_dist)
                    refine_bifs.append(bifur_idlist.at(0));
            }else if(bifur_idlist.size()>1){
                for(int b=0;b<bifur_idlist.size();b++){
                    double sb_dist=dis(nt.listNeuron.at(somaid),nt.listNeuron.at(bifur_idlist.at(b)));
                    if(sb_dist<s_connect_dist)
                        refine_bifs.append(bifur_idlist.at(b));
                }
            }else
                return true;
        }else{
            cout<<"Wrong definition of soma. please check."<<endl;
            return false;
        }
        //refine soma
        if(refine_bifs.size()==1){
            //find the path from 3-bifurcation point to soma point
            QList<V3DLONG> bspath;
            bspath.append(refine_bifs.at(0));
            NeuronSWC s=nt.listNeuron.at(refine_bifs.at(0));
            V3DLONG spid=refine_bifs.at(0);
            NeuronSWC sp;
            while(true){
                if(!nt.hashNeuron.contains(s.pn)||spid==somaid)
                    break;
                spid=nt.hashNeuron.value(s.pn);
                bspath.append(spid);
                sp=nt.listNeuron.at(spid);
                s=sp;
            }
            // reverse topo of bspath
            int reset_type=1;
            for(V3DLONG i=0;i<siz;i++)
                if(nt.listNeuron.at(i).pn==nt.listNeuron.at(somaid).n&&!bspath.contains(i))
                    reset_type=nt.listNeuron.at(i).type;
            cout<<"Rest topo nodes="<<bspath.size()<<endl;
            cout<<"Rest topo nodes type="<<reset_type<<endl;
            for(int i=bspath.size()-1;i>0;i--){
                V3DLONG si=bspath.at(i);
                V3DLONG si1=bspath.at(i-1);
//                cout<<"operate node index="<<nt.listNeuron.at(si).n<<endl;
                nt.listNeuron[si].pn=nt.listNeuron.at(si1).n;
                nt.listNeuron[si].type=reset_type;
            }
            nt.listNeuron[refine_bifs.at(0)].pn=-1;
            nt.listNeuron[refine_bifs.at(0)].type=1;
            // reindex
            NeuronTree nt_new=reindexNT(nt);
            if(save_new){
                if(file_suffix.toLower()==QString("eswc"))
                    writeESWC_file(out_f,nt_new);
                else
                    writeSWC_file(out_f,nt_new);
            }

        }else{
            cout<<"Bifurcation can't be refined to soma point"<<endl;
            return false;
        }
    }
    return true;
}

bool nt_check(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /*preprocess:
     * single tree checking
     * soma checking
     * loop checking
     * multiple bifurcation checking
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    int out_ok_nt=(inparas.size()>=1)?atoi(inparas[1]):0;

    cout<<"swc file="<<inswc_file<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".eswc");
    //single tree checking
    QList<NeuronTree> ntrees=nt_2_trees(nt);
    if(ntrees.size()!=1){
        cout<<"Not single tree, and tree size ="<<ntrees.size()<<endl;
        writeESWC_file(QString::fromStdString(out_f),nt);
        return false;
    }
    V3DLONG somaid=get_soma(nt,false); if(somaid<0){
        cout<<"Soma Error"<<endl;
        writeESWC_file(QString::fromStdString(out_f),nt);
        return false;
    }
    if(loop_checking(nt)){
        writeESWC_file(QString::fromStdString(out_f),nt);
        return false;
    }
    QList<CellAPO> out_3bifs;
    bool mbif=multi_bifurcations_checking(nt,out_3bifs,somaid);
    if(mbif){
        writeESWC_file(QString::fromStdString(out_f),nt);
        return false;
    }
    if(out_ok_nt)
        writeESWC_file(QString::fromStdString(out_f),nt);
    return true;
}

bool nt_qc(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /*preprocess:
     * soma checking
     * loop checking
     * multiple bifurcation checking
     *pruning small tip branch
     * pruning internal node
     * resample node
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}


    bool connect=(((inparas.size()>=1)?atoi(inparas[0]):0)>0)?true:false;
    int three_bifs_label=(inparas.size()>=2)?atoi(inparas[1]):0;
    int tip_br_thre=/*(inparas.size()>=1)?atoi(inparas[0]):*/6;
    float internode_thre=/*(inparas.size()>=2)?atof(inparas[1]):*/1.5;
    int resample_pixels=/*(inparas.size()>=1)?atoi(inparas[0]):*/0;

//    cout<<"swc file="<<inswc_file<<endl;
    if(connect)
        cout<<"connect"<<endl;
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    QString infile_suffix=QFileInfo(inswc_file).suffix();
    QString out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "."+infile_suffix);
    V3DLONG somaid=get_soma(nt,connect); if(somaid<0){
        cout<<"Soma Error"<<endl;
        return false;
    }

    if(loop_checking(nt)){return false;}
    QList<CellAPO> out_3bifs;
    bool mbif=multi_bifurcations_checking(nt,out_3bifs,somaid);
    if(three_bifs_label&&mbif){
        out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".apo");
        writeAPO_file((out_f),out_3bifs);
        return false;
    }
    else{
        if(!mbif){
            cout<<"no multiple bifurcations"<<endl;
        }else{
            cout<<"Now into multiple bifurcation processing"<<endl;
            cout<<"Node raw size="<<nt.listNeuron.size()<<endl;
    //        cout<<"soma index="<<somaid<<endl;
            if(!multi_bifurcations_processing(nt,somaid))
                return false;
            cout<<"Node processed size="<<nt.listNeuron.size()<<endl;
        }
    }
    NeuronTree nt_4,nt_3, nt_out;
    NeuronTree nt_2=tip_branch_pruning(nt,tip_br_thre); //remove tip-branches which length are below 4 pixels
    nt_3=internode_pruning_br(nt_2,internode_thre);
    nt_2.listNeuron.clear(); nt_2.hashNeuron.clear();

    if(resample_pixels)
        nt_4=node_interpolation(nt_3,resample_pixels);
    else
        nt_4.deepCopy(nt_3);
    nt_3.listNeuron.clear(); nt_3.hashNeuron.clear();
    nt_out=reindexNT(nt_4);
    nt_4.listNeuron.clear(); nt_4.hashNeuron.clear();

    if(!QString::compare(QString("swc"),infile_suffix,Qt::CaseInsensitive))
    {
//        cout<<"Out format="<<infile_suffix.toStdString()<<endl;
        writeSWC_file(out_f,nt_out);
    }
    else
        writeESWC_file(out_f,nt_out);
    return true;
}

bool swc2branches(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /*type: specified branch type*/
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    int type=(inparas.size()>=1)?atoi(inparas[0]):0;
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    QString outpath=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
    QDir path(outpath);
    if(!path.exists()) {
        cout<<"make a new dir for saving branches "<<endl;
        path.mkpath(outpath);
    }
    write_branches(nt,outpath,type);
    return true;
}

bool neuron_split(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){

    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
    int toeswc=(inparas.size()>=1)?atoi(inparas[0]):1;

    QString outpath=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
    QDir path(outpath);
    if(!path.exists()) {
        cout<<"make a new dir for saving different neurite types "<<endl;
        path.mkpath(outpath);
    }
    split_neuron_type(inswc_file,outpath,toeswc);
    return true;
}

bool crop_swc_terafly_image_block(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    string inimg_file,inswc_file;
    if(infiles.size()>=2) {
        inimg_file = infiles[0];
        inswc_file = infiles[1];
        int crop_neighbor_voxels=(inparas.size()>=1)?atoi(inparas[0]):0;
        int cropx=(inparas.size()>=2)?atoi(inparas[1]):0;
        int cropy=(inparas.size()>=3)?atoi(inparas[2]):0;
        int cropz=(inparas.size()>=4)?atoi(inparas[3]):0;
        //get out_image_path and out_swc_path
        QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(QString::fromStdString(inswc_file)).path());
        teraImage_swc_crop(callback,inimg_file,inswc_file,out_path,cropx,cropy,cropz,crop_neighbor_voxels);
    }
    return true;
}

bool crop_local_swc(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    string inswc_file;
    if(infiles.size()>=1) {
        inswc_file = infiles[0];
        double cropx=(inparas.size()>=1)?atof(inparas[0]):1024;
        double cropy=(inparas.size()>=2)?atof(inparas[1]):1024;
        double cropz=(inparas.size()>=3)?atof(inparas[2]):256;
        //get out_image_path and out_swc_path
        QString save_path=(outfiles.size()>=1)?outfiles[0]:(QString::fromStdString(inswc_file)+"_local.eswc");
        crop_local_swc_func(callback,inswc_file,save_path,cropx,cropy,cropz);
    }
    return true;
}

bool somalist_in_folder(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString inpath;
    if(infiles.size()>=1) {inpath = infiles[0];} else { return false;}
    QString outpath=(outfiles.size()>=1)?outfiles[0]:(inpath+".apo");
    QStringList filefilters; filefilters<<"swc"<<"eswc";
    QStringList inswcfiles;
    if(!get_files_in_dir(inpath,inswcfiles,filefilters)){return false;}
    cout<<"Input swcs = "<<inswcfiles.size()<<endl;
    int soma_typed=(inparas.size()>=1)?atoi(inparas[0]):1;
//    QStringList tovols; tovols.clear();
    QList<CellAPO> somaapo;
    for(V3DLONG i=0;i<inswcfiles.size();i++){
        QString inswc=inswcfiles.at(i);
        NeuronTree nt=readSWC_file(inswc);
        V3DLONG somaid=get_soma(nt,false,bool(soma_typed));
        if(somaid<0){
            cout<<"No soma is defined in "<<inswc.toStdString()<<endl;
            continue;
        }
        NeuronSWC soma_node=nt.listNeuron.at(somaid);
        CellAPO soma;
        soma.name=QFileInfo(inswc).completeBaseName();
        soma.n=i+1;
        soma.x=soma_node.x;
        soma.y=soma_node.y;
        soma.z=soma_node.z;
        soma.intensity=soma.volsize=soma.sdev=soma.pixmax=soma.mass=10;
        somaapo.append(soma);
    }
    if(somaapo.size())
        writeAPO_file(outpath,somaapo);
    return true;
}

bool bswcTo(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* Bouton swc file conversion
     * filetype=1, to apo file
     * filetype=2, to image marker file
     * otherwise, to apo and marker file
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
//    int statistics=(inparas.size()>=1)?atoi(inparas[0]):1;
    int filetype=(inparas.size()>=1)?atoi(inparas[0]):1;
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    switch (filetype) {
    case 1:
    {        // to apo
        int random_color=(inparas.size()>=2)?atoi(inparas[1]):1;
        QList<CellAPO> boutons; boutons.clear();

         for(V3DLONG i=0;i<siz;i++){
             NeuronSWC s=nt.listNeuron.at(i);
             if(s.type!=5){continue;}
             CellAPO bca;
             bca.n=boutons.size()+1;
             bca.x=s.x;
             bca.y=s.y;
             bca.z=s.z;
             bca.volsize=100;
             if(random_color){
                 bca.color.r=int(siz%255);
                 bca.color.g=int(siz*2%255);
                 bca.color.b=int(siz*3%255);
             }
             else{
                 bca.color.r=0;
                 bca.color.g=200;
                 bca.color.b=0;
             }

             bca.name="axonal bouton";
             boutons.append(bca);
         }
         QString apofilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ ".apo");
         writeAPO_file(apofilename,boutons);
    }
        break;
    case 2:
    {
        // to image marker
        QList<ImageMarker> boutons; boutons.clear();
         for(V3DLONG i=0;i<siz;i++){
             NeuronSWC s=nt.listNeuron.at(i);
             if(s.type!=5){continue;}
             ImageMarker bim;
             bim.x=s.x+(float)1.0;
             bim.y=s.y+(float)1.0;
             bim.z=s.z+(float)1.0;
             bim.radius=s.radius;
             bim.name="axonal bouton";
             bim.color.r=200;
             bim.color.g=0;
             bim.color.b=0;
             boutons.append(bim);
         }
         QString mfilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ ".marker");
         writeMarker_file(mfilename,boutons);
    }
        break;
    default:
    {
        QList<CellAPO> boutons; boutons.clear();
        QList<ImageMarker> mboutons; mboutons.clear();
         for(V3DLONG i=0;i<siz;i++){
             NeuronSWC s=nt.listNeuron.at(i);
             if(s.type!=5){continue;}
             CellAPO bca;
             bca.n=boutons.size()+1;
             bca.x=s.x;
             bca.y=s.y;
             bca.z=s.z;
             bca.volsize=100;
             bca.color.r=200;
             bca.color.g=0;
             bca.color.b=0;
             bca.name="axonal bouton";
             boutons.append(bca);

             ImageMarker bim;
             bim.x=s.x+(float)1.0;
             bim.y=s.y+(float)1.0;
             bim.z=s.z+(float)1.0;
             bim.radius=s.radius;
             bim.name="axonal bouton";
             bim.color.r=200;
             bim.color.g=0;
             bim.color.b=0;
             mboutons.append(bim);
         }
          QString apofilename=(outfiles.size()>=1)?outfiles[0]:(inswc_file+ ".apo");
          QString mfilename=(outfiles.size()>=2)?outfiles[1]:(inswc_file+ ".marker");
          writeAPO_file(apofilename,boutons);
          writeMarker_file(mfilename,mboutons);
    }
        break;
    }

    return true;
}

bool branch_features(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* branch level features, for each branching point
     * 1. branch length
     * 2. distance to soma
     * 3. type
     * 4. asymmetry (#tips)
     * 5. branching angle (remote,local,tilt)
     * 6. subtree length, tips
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
//    int statistics=(inparas.size()>=1)?atoi(inparas[0]):1;
    int nt_check=(inparas.size()>=1)?atoi(inparas[0]):0;
    int process_3_bifs=(inparas.size()>=2)?atoi(inparas[1]):0;
    int with_bouton=(inparas.size()>=3)?atoi(inparas[2]):0;

    cout<<"swc file="<<inswc_file<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    cout<<"size of nodes="<<siz<<endl;
    V3DLONG somaid=get_soma(nt); if(somaid<0){return false;}
//     cout<<"soma index="<<somaid<<endl;
    if(nt_check){
        cout<<"soma index="<<somaid<<endl;
        bool nt_loop=loop_checking(nt);
        QList<CellAPO> out_3bifs;
        bool mbif=multi_bifurcations_checking(nt,out_3bifs,somaid);
        if(process_3_bifs)
        {
            cout<<"try to process 3 bifs"<<endl;
            mbif=multi_bifurcations_processing(nt,somaid);
            if(!mbif||nt_loop){return false;}
        }
        else
            if(mbif||nt_loop)
                return false;
    }
    string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".csv");
    write_branch_features(QString::fromStdString(out_f),nt,bool(with_bouton));
    return true;
}
bool bouton_distribution(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* branch level
     * get bouton distribution density
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
//    int statistics=(inparas.size()>=1)?atoi(inparas[0]):1;
    int split_times=(inparas.size()>=1)?atoi(inparas[0]):4;

    cout<<"swc file="<<inswc_file<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    cout<<"size of nodes="<<siz<<endl;
    V3DLONG somaid=get_soma(nt); if(somaid<0){return false;}
    string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".csv");
    write_bouton_distribution(QString::fromStdString(out_f),nt,split_times);
    return true;
}
bool lm_statistic_features(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* neuron morphological features (statistics: min, max, std, mean, median)
     * -----add neurite type ??
     * the number of stems
     * (overall) width, height, depth, volume, length
     * branch
                * number
                * {path,Euclidean} length
                * contraction
     *branching point:
                * {path,Euclidean}dist2soma
                * partition asymmetry
                * order
     * Angle:
                * bif_ampl_local and big_ampl_remote
                * bif_tilt_local and bif_tilt_remote
     NOTE: make sure neuron is a single connected tree with
            1. one soma (type=1, pn=-1),
            2. no loop,
            3. no three bifurcations
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    string inswc_file;
    if(infiles.size()>=1) {inswc_file = infiles[0];}
//    int statistics=(inparas.size()>=1)?atoi(inparas[0]):1;
    int nt_check=(inparas.size()>=1)?atoi(inparas[0]):0;
    int process_3_bifs=(inparas.size()>=2)?atoi(inparas[1]):0;

    cout<<"swc file="<<inswc_file<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    cout<<"size of nodes="<<siz<<endl;
    V3DLONG somaid=get_soma(nt); if(somaid<0){return false;}
//     cout<<"soma index="<<somaid<<endl;
    if(nt_check){
        cout<<"soma index="<<somaid<<endl;
        bool nt_loop=loop_checking(nt);
        QList<CellAPO> out_3bifs;
        bool mbif=multi_bifurcations_checking(nt,out_3bifs,somaid);
        if(process_3_bifs)
        {
            cout<<"try to process 3 bifs"<<endl;
            mbif=multi_bifurcations_processing(nt,somaid);
            if(!mbif||nt_loop){return false;}
        }
        else
            if(mbif||nt_loop)
                return false;
    }
    string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".csv");
    write_lm_features(QString::fromStdString(out_f),nt);
    return true;
}
bool write_branches(NeuronTree nt,const QString& outpath,int type){
    BranchTree bt; bt.initialized=bt.init(nt);
    if(!bt.initialized){return false;}
    if (outpath.isEmpty()||bt.listBranch.size()==0) { return false;}
    V3DLONG bsiz=bt.listBranch.size();
    for(V3DLONG i=0;i<bsiz;i++)
    {
        BranchUnit bu = bt.listBranch.at(i);
        if(type&&type!=bu.type)
            continue;
        NeuronTree br_nt;
        for(V3DLONG n=0;n<bu.listNode.size();n++){
            br_nt.listNeuron.append(bu.listNode.at(n));
            br_nt.hashNeuron.insert(bu.listNode.at(n).n,br_nt.listNeuron.size()-1);
        }
        QString br_f=outpath+"/"+QString::number(i)+"_branch.eswc";
        writeESWC_file(br_f,br_nt);
    }
    return true;
}
bool write_lm_features(const QString &filename, NeuronTree nt){
    BranchTree bt; bt.initialized=bt.init(nt);
    if(!bt.initialized){cout<<"fail init"<<endl;;return false;}
    if (filename.isEmpty()||bt.listBranch.size()==0) { return false;}
    QFile tofile(filename);
    if(tofile.exists()){cout<<"File overwrite to "<<filename.toStdString()<<endl;}
    cout<<"finished init"<<endl;
    bt.get_globalFeatures();
    bt.get_enhacedFeatures();
    bt.get_branch_angle_io();
    bt.get_branch_child_angle();
    cout<<"feature init finished"<<endl;
//    vector<int> ntype(nt.listNeuron.size(),0);
//    if(!getNodeType(nt,ntype)){return false;}
    V3DLONG internal_brs=0;
    V3DLONG bsiz=bt.listBranch.size();
    for(V3DLONG i=0;i<bsiz;i++)
        if(bt.listBranch.at(i).lstips>0)
            internal_brs++;
    cout<<"initernal branches = "<<internal_brs<<endl;
    cout<<"tip branches="<<bt.tip_branches<<endl;
    std::vector <double> br_plen(bsiz,0.0),br_elen(bsiz,0.0),br_order(bsiz,0.0),br_tor(bsiz,0.0);
    std::vector <double> bif_edist2soma(bsiz,0.0),bif_pdist2soma(bsiz,0.0);
    std::vector <double> /*bif_partasy_elen(bsiz,0.0),bif_partasy_plen(bsiz,0.0),*/bif_partasy(internal_brs,1.0);
    std::vector <double> bif_ampl_local(internal_brs,0.0),bif_ampl_remote(internal_brs,0.0),bif_tilt_local(internal_brs,0.0),bif_tilt_remote(internal_brs,0.0);
    //inside for each branch
    V3DLONG ii=0;
    for(V3DLONG i=0;i<bsiz;i++)
    {
        BranchUnit bu = bt.listBranch.at(i);
        br_plen[i]=bu.pathLength;
        br_elen[i]=bu.length;
//        cout<<bu.length<<endl;
        br_order[i]=bu.level;
        if(bu.pathLength<=0)
            br_tor[i]=0;
        else
            br_tor[i]=1-bu.length/bu.pathLength;
        if(bu.lstips>0){
            if(bu.lstips+bu.rstips-2>0)
                bif_partasy[ii]=abs(int(bu.lstips-bu.rstips))/(bu.lstips+bu.rstips-2);
            bif_ampl_local[ii]=bu.angle;
            bif_ampl_remote[ii]=bu.angle_remote;
            bif_tilt_local[ii]=MIN(bu.angle_io1,bu.angle_io2);
            bif_tilt_remote[ii]=MIN(bu.angle_io1_remote,bu.angle_io2_remote);
            ii++;
//            if((bu.lslength+bu.rslength)>0){
//                bif_partasy_elen[i]=(bu.lslength)/(bu.lslength+bu.rslength);
//                bif_partasy_plen[i]=(bu.lspathLength)/(bu.lspathLength+bu.rspathLength);
//            }
        }
        bif_edist2soma[i]=bu.edist2soma;
        bif_pdist2soma[i]=bu.pdist2soma;

//        cout<<"x="<<bu.listNode.at(bu.listNode.size()-1).x<<",y="<<bu.listNode.at(bu.listNode.size()-1).y<<",z="<<bu.listNode.at(bu.listNode.size()-1).z<<",angle="<<bu.angle<<endl;
    }
//    cout<<vector_mean(bif_ampl_local)<<endl;
//    cout<<"sqrt="<<sqrt(0)<<endl;
    cout<<"feature loading finished"<<endl;
//    cout<<vector_mean(bif_tilt_local)<<endl;
    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString fea_title="feature,value\n";
        tofile.write(fea_title.toAscii());
        QStringList writedata;
        writedata.append("stems,"+QString::number(bt.soma_branches)+"\n");
        writedata.append("branches,"+QString::number(bt.total_branches)+"\n");
        writedata.append("overall_width,"+QString::number(bt.width)+"\n");
        writedata.append("overall_height,"+QString::number(bt.height)+"\n");
        writedata.append("overall_depth,"+QString::number(bt.depth)+"\n");
        writedata.append("overall_volume,"+QString::number(bt.volume)+"\n");
//        writedata.append("total_branch_straight_line_dist,"+QString::number(bt.total_length)+"\n");
        writedata.append("total_length,"+QString::number(bt.total_path_length)+"\n");
        writedata.append("br_length_mean,"+QString::number(vector_mean(br_plen))+"\n");
        writedata.append("br_length_std,"+QString::number(vector_std(br_plen))+"\n");
        writedata.append("br_length_min,"+QString::number(vector_min(br_plen))+"\n");
        writedata.append("br_length_max,"+QString::number(vector_max(br_plen))+"\n");
//        writedata.append("branch_straight_line_dist_mean,"+QString::number(vector_mean(br_elen))+"\n");
//        writedata.append("branch_straight_line_dist_std,"+QString::number(vector_std(br_elen))+"\n");
//        writedata.append("branch_straight_line_dist_min,"+QString::number(vector_min(br_elen))+"\n");
//        writedata.append("branch_straight_line_dist_max,"+QString::number(vector_max(br_elen))+"\n");
        writedata.append("br_order_mean,"+QString::number(vector_mean(br_order))+"\n");
        writedata.append("br_order_std,"+QString::number(vector_std(br_order))+"\n");
        writedata.append("br_order_min,"+QString::number(vector_min(br_order))+"\n");
        writedata.append("br_order_max,"+QString::number(vector_max(br_order))+"\n");
        writedata.append("br_tortuosity_mean,"+QString::number(vector_mean(br_tor))+"\n");
        writedata.append("br_tortuosity_std,"+QString::number(vector_std(br_tor))+"\n");
        writedata.append("br_tortuosity_min,"+QString::number(vector_min(br_tor))+"\n");
        writedata.append("br_tortuosity_max,"+QString::number(vector_max(br_tor))+"\n");
        writedata.append("bif_EucDist2soma_mean,"+QString::number(vector_mean(bif_edist2soma))+"\n");
        writedata.append("bif_EucDist2soma_std,"+QString::number(vector_std(bif_edist2soma))+"\n");
        writedata.append("bif_EucDist2soma_min,"+QString::number(vector_min(bif_edist2soma))+"\n");
        writedata.append("bif_EucDist2soma_max,"+QString::number(vector_max(bif_edist2soma))+"\n");
        writedata.append("bif_PathDist2soma_mean,"+QString::number(vector_mean(bif_pdist2soma))+"\n");
        writedata.append("bif_PathDist2soma_std,"+QString::number(vector_std(bif_pdist2soma))+"\n");
        writedata.append("bif_PathDist2soma_min,"+QString::number(vector_min(bif_pdist2soma))+"\n");
        writedata.append("bif_PathDist2soma_max,"+QString::number(vector_max(bif_pdist2soma))+"\n");
//        writedata.append("bif_partasy_elen_mean,"+QString::number(vector_mean(bif_partasy_elen))+"\n");
//        writedata.append("bif_partasy_elen_std,"+QString::number(vector_std(bif_partasy_elen))+"\n");
//        writedata.append("bif_partasy_elen_min,"+QString::number(vector_min(bif_partasy_elen))+"\n");
//        writedata.append("bif_partasy_elen_max,"+QString::number(vector_max(bif_partasy_elen))+"\n");
//        writedata.append("bif_partasy_plen_mean,"+QString::number(vector_mean(bif_partasy_plen))+"\n");
//        writedata.append("bif_partasy_plen_std,"+QString::number(vector_std(bif_partasy_plen))+"\n");
//        writedata.append("bif_partasy_plen_min,"+QString::number(vector_min(bif_partasy_plen))+"\n");
//        writedata.append("bif_partasy_plen_max,"+QString::number(vector_max(bif_partasy_plen))+"\n");
        writedata.append("asymmetry_mean,"+QString::number(vector_mean(bif_partasy))+"\n");
        writedata.append("asymmetry_std,"+QString::number(vector_std(bif_partasy))+"\n");
        writedata.append("asymmetry_min,"+QString::number(vector_min(bif_partasy))+"\n");
        writedata.append("asymmetry_max,"+QString::number(vector_max(bif_partasy))+"\n");
        writedata.append("ampl_local_mean,"+QString::number(vector_mean(bif_ampl_local))+"\n");
        writedata.append("ampl_local_std,"+QString::number(vector_std(bif_ampl_local))+"\n");
        writedata.append("ampl_local_min,"+QString::number(vector_min(bif_ampl_local))+"\n");
        writedata.append("ampl_local_max,"+QString::number(vector_max(bif_ampl_local))+"\n");
        writedata.append("ampl_remote_mean,"+QString::number(vector_mean(bif_ampl_remote))+"\n");
        writedata.append("ampl_remote_std,"+QString::number(vector_std(bif_ampl_remote))+"\n");
        writedata.append("ampl_remote_min,"+QString::number(vector_min(bif_ampl_remote))+"\n");
        writedata.append("ampl_remote_max,"+QString::number(vector_max(bif_ampl_remote))+"\n");
        writedata.append("tilt_local_mean,"+QString::number(vector_mean(bif_tilt_local))+"\n");
        writedata.append("tilt_local_std,"+QString::number(vector_std(bif_tilt_local))+"\n");
        writedata.append("tilt_local_min,"+QString::number(vector_min(bif_tilt_local))+"\n");
        writedata.append("tilt_local_max,"+QString::number(vector_max(bif_tilt_local))+"\n");
        writedata.append("tilt_remote_mean,"+QString::number(vector_mean(bif_tilt_remote))+"\n");
        writedata.append("tilt_remote_std,"+QString::number(vector_std(bif_tilt_remote))+"\n");
        writedata.append("tilt_remote_min,"+QString::number(vector_min(bif_tilt_remote))+"\n");
        writedata.append("tilt_remote_max,"+QString::number(vector_max(bif_tilt_remote))+"\n");
        cout<<"feature data ready"<<endl;
        for(int i=0;i<writedata.size();i++)
            tofile.write(writedata.at(i).toAscii());
//        tofile.write(brf.toAscii());
        tofile.close();
        return true;
    }
    return false;
}
bool write_branch_features(const QString &filename, NeuronTree nt, bool bouton_fea){
    BranchTree bt; bt.initialized=bt.init(nt,bouton_fea);
    if(!bt.initialized){return false;}
    if (filename.isEmpty()||bt.listBranch.size()==0) { return false;}
    QFile tofile(filename);
    if(tofile.exists()){cout<<"File overwrite to "<<filename.toStdString()<<endl;}
    bt.get_globalFeatures();
    bt.get_enhacedFeatures(bouton_fea);
    bt.get_branch_angle_io();
    bt.get_branch_child_angle();
    cout<<"feature init finished"<<endl;
//    vector<int> ntype(nt.listNeuron.size(),0);
//    if(!getNodeType(nt,ntype)){return false;}
    V3DLONG bsiz=bt.listBranch.size();

//    QString confTitle="#This file is used for recording branch-level motif in a neuron tree (by shengdian).\n";
    QString brfHead="id,parent_id,x,y,z,type,level,dist2soma,path_dist2soma,angle,angle_remote,angle_io1,angle_io1_remote,angle_io2,angle_io2_remote,radius,lcradius,rcradius,";
    brfHead+="length,pathlength,contraction,lclength,lcpathlength,rclength,rcpathlength,width,height,depth,lslength,lspathlength,rslength,rspathlength,lstips,rstips";
    if(bouton_fea)
        brfHead+=",bnum,pbnum,lcbnum,rcbnum,dist2nb,neigborbs,interb_dist,uniform_bdist";
    brfHead+="\n";
    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        tofile.write(brfHead.toAscii());
        //inside for each branch
        for(V3DLONG i=0;i<bsiz;i++)
        {
            BranchUnit bu = bt.listBranch.at(i);
            QString brf=QString::number(bu.id);
            brf+=(","+QString::number(bu.parent_id));
            brf+=(","+QString::number(bu.listNode.at(0).x));
            brf+=(","+QString::number(bu.listNode.at(0).y));
            brf+=(","+QString::number(bu.listNode.at(0).z));
            brf+=(","+QString::number(bu.type));
            brf+=(","+QString::number(bu.level));
            brf+=(","+QString::number(bu.edist2soma));
            brf+=(","+QString::number(bu.pdist2soma));
            brf+=(","+QString::number(bu.angle));
            brf+=(","+QString::number(bu.angle_remote));
            brf+=(","+QString::number(bu.angle_io1));
            brf+=(","+QString::number(bu.angle_io1_remote));
            brf+=(","+QString::number(bu.angle_io2));
            brf+=(","+QString::number(bu.angle_io2_remote));
            brf+=(","+QString::number(bu.radius));
            brf+=(","+QString::number(bu.lcradius));
            brf+=(","+QString::number(bu.rcradius));
            brf+=(","+QString::number(bu.length));
            brf+=(","+QString::number(bu.pathLength));
            if(bu.pathLength<=0)
                brf+=(","+QString::number(1.0));
            else
                brf+=(","+QString::number(double(bu.length/bu.pathLength)));
            brf+=(","+QString::number(bu.lclength));
            brf+=(","+QString::number(bu.lcpathLength));
            brf+=(","+QString::number(bu.rclength));
            brf+=(","+QString::number(bu.rcpathLength));
            brf+=(","+QString::number(bu.width));
            brf+=(","+QString::number(bu.height));
            brf+=(","+QString::number(bu.depth));
            brf+=(","+QString::number(bu.lslength));
            brf+=(","+QString::number(bu.lspathLength));
            brf+=(","+QString::number(bu.rslength));
            brf+=(","+QString::number(bu.rspathLength));
            brf+=(","+QString::number(bu.lstips));
            brf+=(","+QString::number(bu.rstips));
            if(bouton_fea){
                brf+=(","+QString::number(bu.boutons));
                brf+=(","+QString::number(bu.pboutons));
                brf+=(","+QString::number(bu.lcboutons));
                brf+=(","+QString::number(bu.rcboutons));
                brf+=(","+QString::number(bu.mean_MINdist2topo_bouton));
                brf+=(","+QString::number(bu.mean_spatial_neighbor_boutons));
                brf+=(","+QString::number(bu.mean_dist2parent_bouton));
                brf+=(","+QString::number(bu.uniform_bouton_dist));
            }
            brf+="\n";
            tofile.write(brf.toAscii());
        }
        tofile.close();
        return true;
    }
    return false;
}
bool write_bouton_distribution(const QString &filename, NeuronTree nt,int split_times){
    BranchTree bt; bt.initialized=bt.init(nt,true);
    if(!bt.initialized){return false;}
    if (filename.isEmpty()||bt.listBranch.size()==0) { return false;}
    QFile tofile(filename);
    if(tofile.exists()){cout<<"File overwrite to "<<filename.toStdString()<<endl;}
    bt.get_globalFeatures();
    bt.get_enhacedFeatures(true);
    V3DLONG bsiz=bt.listBranch.size();
    QString brfHead="";
    for(int s=0;s<split_times;s++)
        brfHead+=("S"+QString::number(s+1)+",");
    brfHead+="tip_br,total,order,dist2soma,contraction,length\n";

    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        tofile.write(brfHead.toAscii());
        //inside for each branch, get bouton-branch
        for(V3DLONG i=0;i<bsiz;i++)
        {
            BranchUnit bu = bt.listBranch.at(i);
            if(bu.boutons<=0)
                continue;
            double bin_len=bu.pathLength/(double)split_times;
            int tip_br=1;
            if(bu.lstips&&bu.rstips)
                tip_br=0;
            QList<float> hit_boutons;
            int sindex=bu.listNode.size()-1;
            double left_len=0;
            for(int s=0;s<split_times;s++)
            {
                //from tail to head node
                int hit_bouton=0;
                double scan_len=left_len;

                for(int n=sindex;n>0;n--){
                    if(bu.listNode.at(n).type>=5)
                        hit_bouton++;
                    scan_len+=dis(bu.listNode.at(n),bu.listNode.at(n-1));
                    if(scan_len>bin_len){left_len=scan_len-bin_len;sindex=n-1; break;}
                }
                if(s==split_times-1&&bu.listNode.at(0).type>=5)
                    hit_bouton++;
                hit_boutons.append(float(hit_bouton));
//                hit_boutons.append(float(hit_bouton)/float(bu.boutons));
                hit_bouton=0;
            }
            QString brf="";
            for(int s=0;s<split_times;s++)
                brf+=(QString::number(hit_boutons.at(s))+",");
            brf+=(QString::number(tip_br)+",");
            brf+=(QString::number(bu.boutons)+",");
            brf+=(QString::number(bu.level)+",");
            brf+=(QString::number(bu.listNode.at(0).fea_val.at(7))+",");
            brf+=(QString::number(bu.length/bu.pathLength)+",");
            brf+=(QString::number(bu.pathLength)+"\n");
            tofile.write(brf.toAscii());
        }
        tofile.close();
        return true;
    }
    return false;
}
