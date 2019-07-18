/* calculate_reliability_score_plugin.cpp
 * This plugin will generate the reliability score of neuron reconstruction.
 * 2015-4-27 : by Hanbo Chen, Hang Xiao, Hanchuan Peng
 * 2018-10-09: by Peng Xie
 */

#include "calculate_reliability_score_main.h"
#include "src/topology_analysis.h"
//#include "src/my_surf_objs.h"

typedef vector<MyMarker*> Segment;
typedef vector<MyMarker*> Tree;

void doCalculateScore(V3DPluginCallback2 &callback, QString fname_img, QString fname_swc, QString fname_output, int score_type=1, float radius_factor=2, bool is_gui=0)
{
    //load image
    unsigned char * p_img1d = 0;
    int type_img;
    V3DLONG sz_img[4];
    if(!simple_loadimage_wrapper(callback, fname_img.toStdString().c_str(), p_img1d, sz_img, type_img)){
        if(is_gui){
            v3d_msg("ERROR: failed to load image file "+fname_img);
        }else{
            qDebug()<<"ERROR: failed to load image file "<<fname_img;
        }
        exit(1);
    }
    if(sz_img[3]>1){
        if(is_gui){
            v3d_msg("WARNING: image has more than 1 color channel. Only the first channel will be used.");
        }else{
            qDebug()<<"WARNING: image has more than 1 color channel. Only the first channel will be used.";
        }
    }

    //load swc
    vector<MyMarker *> neuronTree = readSWC_file(fname_swc.toStdString());
    if(neuronTree.size()==0){
        if(is_gui){
            v3d_msg("ERROR: failed to load swc file "+fname_swc);
        }else{
            qDebug()<<"ERROR: failed to load swc file "<<fname_swc;
        }
        exit(1);
    }

    //calcluate
    map<MyMarker*, double> score_map;
    if(score_type <2) //topology segment
        if(type_img==1)
            topology_analysis_perturb_intense(p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], score_type);
        else if(type_img==2)
            topology_analysis_perturb_intense((unsigned short *)p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], score_type);
        else if(type_img==4)
            topology_analysis_perturb_intense((float *)p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], score_type);
        else{
            if(is_gui){
                v3d_msg("ERROR: image formate not supported. Only support: UINT8, UINT16, and FLOAT32.");
            }else{
                qDebug()<<"ERROR: image formate not supported. Only support: UINT8, UINT16, and FLOAT32";
            }
            exit(1);
        }
    else
        if(type_img==1)
            path_analysis_perturb_intense(p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], 0.9, score_type-2);
        else if(type_img==2)
            path_analysis_perturb_intense((unsigned short *)p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], 0.9, score_type-2);
        else if(type_img==4)
            path_analysis_perturb_intense((float *)p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], 0.9, score_type-2);
        else{
            if(is_gui){
                v3d_msg("ERROR: image formate not supported. Only support: UINT8, UINT16, and FLOAT32.");
            }else{
                qDebug()<<"ERROR: image formate not supported. Only support: UINT8, UINT16, and FLOAT32";
            }
            exit(1);
        }

    //output
    QString fname_outscore = fname_output + "_score.txt";
    ofstream fp(fname_outscore.toStdString().c_str());
    if(!fp.is_open()){
        if(is_gui){
            v3d_msg("ERROR: cannot open file to save: "+fname_outscore);
        }else{
            qDebug()<<"ERROR: cannot open file to save: "<<fname_outscore;
        }
        exit(1);
    }
    for(long i=0; i<neuronTree.size(); i++){
        fp<<i+1<<"\t"<<score_map[neuronTree[i]]<<endl;
    }
    fp.close();
    for(V3DLONG i = 0; i<neuronTree.size(); i++){
        MyMarker * marker = neuronTree[i];
        double tmp = score_map[marker] * 80 +19;
        marker->type = tmp > 255 ? 255 : tmp;
    }
    QString fname_outswc = fname_output+"_scored.swc";
    saveSWC_file(fname_outswc.toStdString(), neuronTree);

    qDebug()<<"Done!";
}

NeuronTree calculateScoreTerafly(V3DPluginCallback2 &callback,QString fname_img, NeuronTree nt, int score_type,
                                 float radius_factor, float output_thres, QString prefix)
{
    qDebug()<<"entering calculateScoreTerafly";
    NeuronTree result;
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        MyMarker* pt = new MyMarker;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->radius = s.r;
        pt->type = s.type;
        pt->parent = NULL;
        pt->level = 0;
        tree.push_back(pt);
        hashNeuron.insert(s.n, i);
    }
    nt.hashNeuron = hashNeuron;

    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        tree[i]->parent = tree[pid];
        tree[pid]->level++;
    }
        printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->level!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            MyMarker* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->parent;
            }
            while(cur && cur->level==1);
            seg_list.push_back(seg);
        }
    }


    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    for(V3DLONG i=0; i<seg_list.size();i++)
    {
        Segment* seg = seg_list[i];

        start_x = seg->at(0)->x;
        end_x = seg->at(0)->x;
        start_y = seg->at(0)->y;
        end_y = seg->at(0)->y;
        start_z = seg->at(0)->z;
        end_z = seg->at(0)->z;
        for(V3DLONG j=1; j<seg->size();j++)
        {
            if(start_x>seg->at(j)->x)  start_x = seg->at(j)->x;
            if(end_x<seg->at(j)->x)  end_x = seg->at(j)->x;
            if(start_y>seg->at(j)->y)  start_y = seg->at(j)->y;
            if(end_y<seg->at(j)->y)  end_y = seg->at(j)->y;
            if(start_z>seg->at(j)->z)  start_z = seg->at(j)->z;
            if(end_z<seg->at(j)->z)  end_z = seg->at(j)->z;
        }

        start_x -= 50;
        end_x   +=50;
        start_y -= 50;
        end_y   +=50;
        start_z -= 50;
        end_z   +=50;

        unsigned char * total1dData = 0;
        total1dData = callback.getSubVolumeTeraFly(fname_img.toStdString(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
        V3DLONG mysz[4];
        mysz[0] = end_x-start_x+1;
        mysz[1] = end_y-start_y+1;
        mysz[2] = end_z-start_z+1;
        mysz[3] = 1;

        for(V3DLONG j=0; j<seg->size();j++)
        {
            seg->at(j)->x -= start_x;
            seg->at(j)->y -= start_y;
            seg->at(j)->z -= start_z;
        }

//        QString imageSaveString = fname_img + QString("/x_%1_y%2_z%3.v3draw").arg(seg->at(0)->x).arg(seg->at(0)->y).arg(seg->at(0)->z);
//        QString swcSaveString = QString("Segment_%1.swc").arg(i);
//        saveSWC_file(swcSaveString.toStdString(), *seg);

        // Calculate confidence scores
        double max_score = 0;
        map<MyMarker*, double> score_map;
        topology_analysis_perturb_intense(total1dData, *seg, score_map, radius_factor, mysz[0], mysz[1], mysz[2], 1);
        for(V3DLONG i = 0; i<seg->size(); i++){
            MyMarker * marker = seg->at(i);
            double tmp = score_map[marker] * 120 +19;
            marker->level = tmp > 255 ? 255 : tmp;
            if(marker->level>max_score){
                max_score = marker->level;
            }
//            marker->x += start_x;
//            marker->y += start_y;
//            marker->z += start_z;
        }
        // @ Peng Xie edited
        // Save image and corresponding SWC
        if(max_score>output_thres){
            QString imageSaveString = prefix+QString("Segment_%1.v3draw").arg(i);
            qDebug()<<"Save image"<<imageSaveString;
            simple_saveimage_wrapper(callback, qPrintable(imageSaveString),(unsigned char *)total1dData, mysz, 1);
            qDebug()<<"Save image done";
            QString swcSaveString = prefix+QString("Segment_%1.swc").arg(i);
            saveSWC_file(swcSaveString.toStdString(), *seg);
        }
        // Recover SWC coordinates
        for(V3DLONG i = 0; i<seg->size(); i++){
            MyMarker * marker = seg->at(i);
            marker->x += start_x;
            marker->y += start_y;
            marker->z += start_z;
        }

        if(total1dData) {delete [] total1dData; total1dData=0;}
//        QString fname_tmp = fname_img+"/scored.swc";
//        saveSWC_file(fname_tmp.toStdString(), *seg);


    }

    tree.clear();
    map<MyMarker*, V3DLONG> index_map;
    for (V3DLONG i=0;i<seg_list.size();i++)
        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            tree.push_back(seg_list[i]->at(j));
            index_map.insert(pair<MyMarker*, V3DLONG>(seg_list[i]->at(j), tree.size()-1));
        }

//    v3d_msg(QString("tree size is %1").arg(tree.size()));

    for (V3DLONG i=0;i<tree.size();i++)
    {
        NeuronSWC S;
        MyMarker* p = tree[i];
        S.n = i+1;
        if (p->parent==NULL) S.pn = -1;
        else
            S.pn = index_map[p->parent]+1;
        if (p->parent==p) printf("There is loop in the tree!\n");
        S.x = p->x;
        S.y = p->y;
        S.z = p->z;
        S.r = p->radius;
        S.type = p->type;
        S.level = p->level;
        result.listNeuron.push_back(S);
    }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        if (tree[i]) {delete tree[i]; tree[i]=NULL;}
    }
    for (V3DLONG j=0;j<seg_list.size();j++)
        if (seg_list[j]) {delete seg_list[j]; seg_list[j] = NULL;}
    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);

    return result;
}
