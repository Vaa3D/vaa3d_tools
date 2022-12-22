#include "plugin_dofunc.h"
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
    int nt_check=(inparas.size()>=1)?atoi(inparas[1]):0;

    cout<<"swc file="<<inswc_file<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}
    cout<<"size of nodes="<<siz<<endl;
    V3DLONG somaid=get_soma(nt); if(somaid<0){return false;}
//     cout<<"soma index="<<somaid<<endl;
    if(nt_check){
        //single tree check

        cout<<"soma index="<<somaid<<endl;
        bool nt_loop=loop_checking(nt);
        bool mbif=multi_bifurcations_checking(nt,somaid);
        if(mbif||nt_loop){return false;}
    }
    string out_f=(outfiles.size()>=1)?outfiles[0]:(inswc_file + ".csv");
    write_lm_features(QString::fromStdString(out_f),nt);
    return true;
}
bool write_lm_features(const QString &filename, NeuronTree nt){
    BranchTree bt; bt.initialized=bt.init(nt);
    if(!bt.initialized){return false;}
    if (filename.isEmpty()||bt.listBranch.size()==0) { return false;}
    QFile tofile(filename);
    if(tofile.exists()){cout<<"File overwrite to "<<filename.toStdString()<<endl;}
    bt.get_globalFeatures();
    bt.get_enhacedFeatures();
    bt.get_branch_angle_io();
    bt.get_branch_child_angle();
    cout<<"feature init finished"<<endl;
//    vector<int> ntype(nt.listNeuron.size(),0);
//    if(!getNodeType(nt,ntype)){return false;}
    V3DLONG bsiz=bt.listBranch.size();
    std::vector <double> br_plen(bsiz,0.0),br_elen(bsiz,0.0),br_order(bsiz,0.0),br_contraction(bsiz,0.0);
    std::vector <double> bif_edist2soma(bsiz,0.0),bif_pdist2soma(bsiz,0.0),bif_partasy(bsiz,0.0);
    std::vector <double> bif_ampl_local(bsiz,0.0),bif_ampl_remote(bsiz,0.0),bif_tilt_local(bsiz,0.0),bif_tilt_remote(bsiz,0.0);
    //inside for each branch
    for(V3DLONG i=0;i<bsiz;i++)
    {
        BranchUnit bu = bt.listBranch.at(i);
        BranchSequence brs;
        bt.to_soma_br_seq(i,brs);
        br_plen[i]=bu.pathLength;
        br_elen[i]=bu.length;
//        cout<<bu.length<<endl;
        br_order[i]=bu.level;
        br_contraction[i]=bu.length/bu.pathLength;
        if(bu.lstips+bu.rstips-2)
            bif_partasy[i]=abs(int(bu.lstips-bu.rstips))/(bu.lstips+bu.rstips-2);
        bif_edist2soma[i]=brs.seqLength;
        bif_pdist2soma[i]=brs.seqPathLength;
        bif_ampl_local[i]=bu.angle;
        bif_ampl_remote[i]=bu.angle_remote;
        bif_tilt_local[i]=MIN(bu.angle_io1,bu.angle_io2);
        bif_tilt_remote[i]=MIN(bu.angle_io1_remote,bu.angle_io2_remote);
    }
    cout<<"feature loading finished"<<endl;
    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString fea_title="feature,value\n";
        tofile.write(fea_title.toAscii());
        QStringList writedata;
        writedata.append("stems,"+QString::number(bt.soma_branches)+"\n");
        writedata.append("branches,"+QString::number(bt.total_branches)+"\n");
        writedata.append("width,"+QString::number(bt.width)+"\n");
        writedata.append("height,"+QString::number(bt.height)+"\n");
        writedata.append("depth,"+QString::number(bt.depth)+"\n");
        writedata.append("volume,"+QString::number(bt.volume)+"\n");
        writedata.append("elen,"+QString::number(bt.total_length)+"\n");
        writedata.append("plen,"+QString::number(bt.total_path_length)+"\n");
        writedata.append("br_plen_mean,"+QString::number(vector_mean(br_plen))+"\n");
        writedata.append("br_plen_std,"+QString::number(vector_std(br_plen))+"\n");
        writedata.append("br_plen_min,"+QString::number(vector_min(br_plen))+"\n");
        writedata.append("br_plen_max,"+QString::number(vector_max(br_plen))+"\n");
        writedata.append("br_elen_mean,"+QString::number(vector_mean(br_elen))+"\n");
        writedata.append("br_elen_std,"+QString::number(vector_std(br_elen))+"\n");
        writedata.append("br_elen_min,"+QString::number(vector_min(br_elen))+"\n");
        writedata.append("br_elen_max,"+QString::number(vector_max(br_elen))+"\n");
        writedata.append("br_order_mean,"+QString::number(vector_mean(br_order))+"\n");
        writedata.append("br_order_std,"+QString::number(vector_std(br_order))+"\n");
        writedata.append("br_order_min,"+QString::number(vector_min(br_order))+"\n");
        writedata.append("br_order_max,"+QString::number(vector_max(br_order))+"\n");
        writedata.append("br_contraction_mean,"+QString::number(vector_mean(br_contraction))+"\n");
        writedata.append("br_contraction_std,"+QString::number(vector_std(br_contraction))+"\n");
        writedata.append("br_contraction_min,"+QString::number(vector_min(br_contraction))+"\n");
        writedata.append("br_contraction_max,"+QString::number(vector_max(br_contraction))+"\n");
        writedata.append("bif_edist2soma_mean,"+QString::number(vector_mean(bif_edist2soma))+"\n");
        writedata.append("bif_edist2soma_std,"+QString::number(vector_std(bif_edist2soma))+"\n");
        writedata.append("bif_edist2soma_min,"+QString::number(vector_min(bif_edist2soma))+"\n");
        writedata.append("bif_edist2soma_max,"+QString::number(vector_max(bif_edist2soma))+"\n");
        writedata.append("bif_pdist2soma_mean,"+QString::number(vector_mean(bif_pdist2soma))+"\n");
        writedata.append("bif_pdist2soma_std,"+QString::number(vector_std(bif_pdist2soma))+"\n");
        writedata.append("bif_pdist2soma_min,"+QString::number(vector_min(bif_pdist2soma))+"\n");
        writedata.append("bif_pdist2soma_max,"+QString::number(vector_max(bif_pdist2soma))+"\n");
        writedata.append("bif_partasy_mean,"+QString::number(vector_mean(bif_partasy))+"\n");
        writedata.append("bif_partasy_std,"+QString::number(vector_std(bif_partasy))+"\n");
        writedata.append("bif_partasy_min,"+QString::number(vector_min(bif_partasy))+"\n");
        writedata.append("bif_partasy_max,"+QString::number(vector_max(bif_partasy))+"\n");
        writedata.append("bif_ampl_local_mean,"+QString::number(vector_mean(bif_ampl_local))+"\n");
        writedata.append("bif_ampl_local_std,"+QString::number(vector_std(bif_ampl_local))+"\n");
        writedata.append("bif_ampl_local_min,"+QString::number(vector_min(bif_ampl_local))+"\n");
        writedata.append("bif_ampl_local_max,"+QString::number(vector_max(bif_ampl_local))+"\n");
        writedata.append("bif_ampl_remote_mean,"+QString::number(vector_mean(bif_ampl_remote))+"\n");
        writedata.append("bif_ampl_remote_std,"+QString::number(vector_std(bif_ampl_remote))+"\n");
        writedata.append("bif_ampl_remote_min,"+QString::number(vector_min(bif_ampl_remote))+"\n");
        writedata.append("bif_ampl_remote_max,"+QString::number(vector_max(bif_ampl_remote))+"\n");
        writedata.append("bif_tilt_local_mean,"+QString::number(vector_mean(bif_tilt_local))+"\n");
        writedata.append("bif_tilt_local_std,"+QString::number(vector_std(bif_tilt_local))+"\n");
        writedata.append("bif_tilt_local_min,"+QString::number(vector_min(bif_tilt_local))+"\n");
        writedata.append("bif_tilt_local_max,"+QString::number(vector_max(bif_tilt_local))+"\n");
        writedata.append("bif_tilt_remote_mean,"+QString::number(vector_mean(bif_tilt_remote))+"\n");
        writedata.append("bif_tilt_remote_std,"+QString::number(vector_std(bif_tilt_remote))+"\n");
        writedata.append("bif_tilt_remote_min,"+QString::number(vector_min(bif_tilt_remote))+"\n");
        writedata.append("bif_tilt_remote_max,"+QString::number(vector_max(bif_tilt_remote))+"\n");
        for(int i=0;i<writedata.size();i++)
            tofile.write(writedata.at(i).toAscii());
//        tofile.write(brf.toAscii());
        tofile.close();
        return true;
    }
    return false;
}
