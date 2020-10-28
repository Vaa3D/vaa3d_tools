#include "feature_analysis.h"
#include <vector>
#include <stack>
#include <QMessageBox>

#define FNUM 26
#ifndef VOID
#define VOID 1000000000
#endif
#define PI 3.14159265359
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)
#define MAX_DOUBLE 1.79769e+308


using namespace std;

//sort swc & split
void Process(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 & callback){
    vector<char*> in, inparas, outfiles;
    if(input.size() >= 1) in = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    bool hasOutput;
    if(output.size() >= 1) {outfiles = *((vector<char*> *)output.at(0).p);hasOutput=true;}
    else{hasOutput=false;}
    //cout<<in.size()<<endl;
    string indir_tmp;
    for (int i = 0;i<in.size();++i) {
      indir_tmp.append(in.at(i));
    }
    QString indir= QString::fromUtf8(indir_tmp.c_str(), indir_tmp.length());
    Analysis(indir,outfiles,hasOutput);
}

void Analysis(QString indir,vector<char*> outfiles, bool hasOutput){
    QDir dir(indir);
    //printf("%s\n",indir.toStdString().data());
    QStringList nameFilters;
    nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    //cout<<files.size()<<endl;

    for(int i=0;i<files.size();i++){
        QString swc_file = files.at(i);
        //printf("%s\n",swc_file.toStdString().data());
        QStringList list=swc_file.split(".");
        QString id=list.first();
        //QString output_branch = output_dir+"/"+flag1+".apo";
        QString swc_path=indir+"/"+swc_file;
        //printf("%s\n",swc_path.toStdString().data());
        NeuronTree nt_unsorted = readSWC_file(swc_path);
        //cout<<nt_unsorted.listNeuron.size()<<endl;

        //sort
        QString fileSaveName;
        QString saveFolder;
        QString file_den,file_local,file_proj,file_clu;
        if (hasOutput)
        {
            fileSaveName = QString(outfiles.at(0))+"/"+swc_file+"_sorted.swc";
            saveFolder = QString(outfiles.at(0));
            file_den = QString(outfiles.at(0))+"/"+swc_file+"_sorted.dendrite.swc";
            file_local = QString(outfiles.at(0))+"/"+swc_file+"_sorted.axon.proximal_axon.swc";
            file_proj = QString(outfiles.at(0))+"/"+swc_file+"_sorted.axon.long_projection.swc";
            file_clu = QString(outfiles.at(0))+"/"+swc_file+"_sorted.axon.distal_axon.swc";
        }
        else
        {
            fileSaveName = swc_path+"_sorted.swc";
            saveFolder = indir;
            file_den = swc_path+"_sorted.dendrite.swc";
            file_local = swc_path+"_sorted.proximal_axon.swc";
            file_proj = swc_path+"_sorted.long_projection.swc";
            file_clu = swc_path+"_sorted.distal_axon.swc";
        }
        QList<NeuronSWC> neuron_unsorted,result;
        neuron_unsorted=nt_unsorted.listNeuron;
        V3DLONG rootid = VOID;
        double thres = VOID;
        if (!SortSWC(neuron_unsorted, result , rootid, thres))
        {
            cout<<"Error in sorting swc"<<endl;
        }
        if (!export_list2file(result, fileSaveName, swc_path))
        {
            cout<<"Error in writing swc to file"<<endl;
        }


        //split
        split_neuron(fileSaveName, indir);

        //global_feature & Completeness
        //1. swc
        NeuronTree nt_sorted = readSWC_file(fileSaveName);
        double * swc_features = new double[FNUM];
        computeFeature(nt_sorted,swc_features);

        //2.dendrite
        NeuronTree nt_dendrite = readSWC_file(file_den);
        // determine whether apical/basal
        bool apical = false;
        for (int i=0;i<nt_dendrite.listNeuron.size();i++){
            if (nt_dendrite.listNeuron.at(i).type == 4){
                apical = true;
                break;
            }
        }
        double * den_features = new double[FNUM];
        computeFeature(nt_dendrite,den_features);

        //3.local axon
        NeuronTree nt_local = readSWC_file(file_local);
        double * local_features = new double[FNUM];
        computeFeature(nt_local,local_features);

        //4.long projection
        NeuronTree nt_proj = readSWC_file(file_proj);
        double * proj_features = new double[FNUM];
        computeFeature(nt_proj,proj_features);

        //5.axon cluster
        NeuronTree nt_clu = readSWC_file(file_clu);
        double * clu_features = new double[FNUM];
        computeFeature(nt_clu,clu_features);

        //analysis
        bool judgement = true;
        ofstream out;
        out.open("/home/penglab/Desktop/result.txt",ofstream::app);

        cout<<"--------------------Analytical Results---------------------------"<<endl;
        if(swc_features[4]/swc_features[5]>2.5 | swc_features[18]>1){
            //printf("%s is abnormal in branch/tip ratio!",id.toStdString().data());
            out<<id.toStdString().data()<<"'s swc is abnormal in branch/tip ratio"<<endl;
            judgement = false;
        }
        if(den_features[18]>1){
            //printf("%s is abnormal in branch/tip ratio!",id.toStdString().data());
            out<<id.toStdString().data()<<"'s dendrite is abnormal in branch/tip ratio"<<endl;
            judgement = false;
        }
        if(clu_features[18]>1){
            //printf("%s is abnormal in branch/tip ratio!",id.toStdString().data());
            out<<id.toStdString().data()<<"'s cluster is abnormal in branch/tip ratio"<<endl;
            judgement = false;
        }
        if(den_features[13]>1500){
            //printf("%s has too long dendrite branches! ",id.toStdString().data());
            out<<id.toStdString().data()<<"'s dendrite has too long dendrite branches!"<<endl;
            judgement = false;
        }
        if(den_features[20]>95){
            //printf("%s's average remote angle is too high!",id.toStdString().data());
            out<<id.toStdString().data()<<" dendrite's average remote angle is too high!"<<endl;
            judgement = false;
        }
        if (nt_local.listNeuron.size()>0){
            if(local_features[18]>1){
                //printf("%s is abnormal in branch/tip ratio!",id.toStdString().data());
                out<<id.toStdString().data()<<"'s local axon is abnormal in branch/tip ratio!"<<endl;
                judgement = false;
            }
            if(local_features[2]>1){
                //printf("%s has 2 long projections!",id.toStdString().data());
                out<<id.toStdString().data()<<"'s local axon has two long projections!"<<endl;
                judgement = false;
            }
        }
        if(nt_proj.listNeuron.size()>0){
            if(proj_features[18]>1){
                //printf("%s is abnormal in branch/tip ratio!",id.toStdString().data());
                out<<id.toStdString().data()<<"'s long projection is abnormal in branch/tip ratio!!"<<endl;
                judgement = false;
            }
        }
        if(judgement){
        //printf("%s currently has no obvious problems!",id.toStdString().data());
        out<<id.toStdString().data()<<" currently has no obvious problems!"<<endl;
        }
        out.close();

    }



}

//bool QC_domenu(V3DPluginCallback2 &callback, QWidget *parent)
//{
//    //choose a directory that contain swc files
//    QString qs_dir_swc;
//    qs_dir_swc=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the directory that contains files to be processed")));
//    vector<char*> outfiles;
//    bool hasOutput = false;
//    Analysis(qs_dir_swc,outfiles,hasOutput);
//    //output part
//}

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<endl;
    myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
    {
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << " ";
        if(fileSaveName.endsWith(".eswc",Qt::CaseInsensitive))
        {
            myfile << lN.at(i).seg_id << " " << lN.at(i).level << " " << lN.at(i).creatmode << " " << lN.at(i).timestamp << " " << (long)lN.at(i).tfresindex;
        }
        myfile << "\n";
    }
    file.close();
    cout<<"swc/eswc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
}

bool fexists(QString filename)
{
  std::ifstream ifile(qPrintable(filename));
  return ifile.good();
}

int get_soma(NeuronTree nt){
    printf("Checking soma\n");
    const int N=nt.listNeuron.size();
    int soma;
    int soma_ct=0;
    // check whether unique soma
    for(int i=0; i<N; i++){
        // soma check
        if(nt.listNeuron.at(i).type==1){
            soma=i;
            soma_ct++;
            break;
//            if(soma_ct>1){return -1;}
        }
    }
    if(soma_ct==0){
        qDebug() << "Error: No soma found\n";
        return -1;
    }
    return soma;
}

NeuronTree get_subtree_by_type(NeuronTree nt, QList<int> type, bool keep_soma){
    NeuronTree new_tree;
    NeuronSWC node;
    for(int i=0;i<nt.listNeuron.size();i++){
        node = nt.listNeuron.at(i);
        if(keep_soma){
            if(type.indexOf(node.type)!=(-1) || node.type==1){
                new_tree.listNeuron.append(node);
            }
        }
        else{
            if(type.indexOf(node.type)!=(-1)){
                new_tree.listNeuron.append(node);
            }
        }
    }
    return new_tree;
}

NeuronTree single_tree(NeuronTree nt, int soma){
    QList<int> pList;
    QList<int> nList;
    const int N=nt.listNeuron.size();
    int subtree_ct=0;
    // check whether this is a single tree
    for(int i=0; i<N; i++){
        // root check
        if(nt.listNeuron.at(i).pn<0){
            subtree_ct++;
        }
        pList.append(nt.listNeuron.at(i).pn);
        nList.append(nt.listNeuron.at(i).n);
    }

//    // Case 1
    if(subtree_ct==1){return nt;}

    // Case 2
    if(subtree_ct==0){
        printf("No root found in the swc.\n");
        return nt;
    }

//    list<int> children[N];
//    list<int> children[MAXSIZE];
//    for(int i=0; i<N; i++){
//        NeuronSWC node = nt.listNeuron.at(i);
//        int pid = nList.lastIndexOf(node.pn);
//        if(pid<0){
//            continue;
//        }
//        children[pid].push_back(i);
//    }
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = nList.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    // Case 3
    if(subtree_ct>0){
        printf("More than one single trees in the input swc. Only the first connected with soma will be reported.\n");
        NeuronTree new_tree;
        // DFS to extract the first soma-connected tree;

        // Initialization
        int pid=soma;
        QList<int> visited;
        for(int i=0;i<N; i++){visited.append(0);}
        QStack<int> pstack;
        pstack.push(pid);
        bool is_push = false;
        visited[pid]=1;
        new_tree.listNeuron.append(nt.listNeuron.at(pid));
//        cout<<nt.listNeuron.at(pid).n<<"\t"<<nt.listNeuron.at(pid).x<<"\t"<<nt.listNeuron.at(pid).y<<"\t"<<nt.listNeuron.at(pid).z<<endl;

        // DFS using stack
        while(!pstack.isEmpty()){
            is_push = false;
            pid = pstack.top();
            // whether exist unvisited children of pid
            // if yes, push child to stack;
            for (V3DLONG *i = children[pid].begin(); i != children[pid].end(); ++i)
            {
                NeuronSWC node = nt.listNeuron.at(*i);
                if(nList.lastIndexOf(node.pn)==pid && visited.at(*i)==0){
                    pstack.push(*i);
                    visited[*i]=1;
                    new_tree.listNeuron.append(node);
//                    cout<<node.n<<"\t"<<node.x<<"\t"<<node.y<<"\t"<<node.z<<endl;
                    is_push=true;
                    break;
                }
            }
            // else, pop pid
            if(!is_push){
                pstack.pop();
            }
        }
        return new_tree;
    }
}

NeuronTree return_dendrite(NeuronTree nt, int soma){
    printf("welcome to use return_dendrite\n");
    QList<int> type;
    type<<3<<4;
    nt = get_subtree_by_type(nt, type,true);
    nt = single_tree(nt, soma);
    return nt;
}

NeuronTree return_axon(NeuronTree nt, int soma){

    printf("Welcome to use return_axon.\n");
    // 1. Get axon subtrees starting from soma
    // 1.1 extract axon nodes
    QList<int> type;
    type<<2;
    NeuronTree new_tree = get_subtree_by_type(nt, type,true);
    QList<int> name_list;
    for(int i=0; i<nt.listNeuron.size();i++){
        NeuronSWC node = nt.listNeuron.at(i);
        name_list.append(node.n);
    }
    // 1.2 Add axon-connected dendrite segments to axon subtrees
    // Find out whether any axon node connected to dendrite
    QList<int> a2d_ind;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        int pn_ind = name_list.indexOf(node.pn);
        if(pn_ind<0){continue;}
        if(((nt.listNeuron.at(pn_ind).type==3) || (nt.listNeuron.at(pn_ind).type==4)) && (node.type==2)){
            a2d_ind.append(i);
            printf("Node %d is connected to dendrite.\n", i);
        }
    }
    // If any, trace back to soma;
    if(a2d_ind.size()>0){
        if(a2d_ind.size()>1){printf("Warning: %d axon nodes connected to dendrite!\n", a2d_ind.size());}
        for(int i=0; i<a2d_ind.size(); i++){
            bool soma_reached=0;
            NeuronSWC node = nt.listNeuron.at(a2d_ind.at(i));
            while(node.pn != -1){
                int pn_ind = name_list.indexOf(node.pn);
                node = nt.listNeuron.at(pn_ind);
                if((node.type==3) || (node.type==4)){new_tree.listNeuron.append(node);}
                if(node.type==1){soma_reached==1;}
            }
            if(!soma_reached){
                printf("Warning: axon nodes connected to dendrite but cannot trace back to soma!\n");
            }
        }
    }

    nt = single_tree(new_tree, soma);
    return nt;
}

double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
    double xx = (s1.x-s2.x);
    double yy = (s1.y-s2.y);
    double zz = (s1.z-s2.z);
    return sqrt(xx*xx+yy*yy+zz*zz);
}

QList <int> find_long_axon(NeuronTree nt, int soma){
    printf("welcome to use find_long_axon\n");
    const int N = nt.listNeuron.size();
    QList<int> name_list;
    QList<int> parent_list;
    for(int i=0; i<N; i++){
        name_list.append(nt.listNeuron.at(i).n);
        parent_list.append(nt.listNeuron.at(i).pn);
    }

    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = name_list.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    // 1. find longest path from soma
    // DFS
    // Initialization
    int pid=soma;
    QList<double> distance;
    for(int i=0;i<N; i++){distance.append(-1);}
    QStack<int> pstack;
    pstack.push(pid);
    bool is_push = false;
    distance[pid]=0;

    // DFS search
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        // whether exist unvisited children of pid
        // if yes, push child to stack;
//        for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
        for(V3DLONG *i=children[pid].begin(); i!=children[pid].end(); ++i)
        {
            if((nt.listNeuron.at(*i).pn)==nt.listNeuron.at(pid).n && distance.at(*i)==(-1.0)){
                pstack.push(*i);
                distance[*i]=distance.at(pid)+computeDist2(nt.listNeuron.at(pid), nt.listNeuron.at(*i));
                is_push=true;
                break;
            }
        }
        // else, pop pid
        if(!is_push){
            pstack.pop();
        }
    }
    QList<double> sort_distance = distance;
    qSort(sort_distance);
    double longest_distance = sort_distance.last();
    int endpoint = distance.lastIndexOf(longest_distance);
    cout<<"Longest distance:\t"<<distance.at(endpoint)<<"\t"<<"node "<<nt.listNeuron.at(endpoint).n <<endl;

    // 2. Return a list of node ids of the long projection axon
    QList <int> lpa;
    int cur_id = endpoint;
    lpa.prepend(cur_id);
    // Peng Xie 2019-03-20
    while(nt.listNeuron.at(cur_id).pn!=(-1)){
        cur_id = name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn); // Move to the parent node
        if(cur_id==name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn)){break;} // check self loop; should not exist;
        lpa.prepend(cur_id);
    }
    cout<<"finished"<<endl;
    return lpa;
}

NeuronTree return_long_axon(NeuronTree nt, int soma){
    QList<int> idlist = find_long_axon(nt, soma);
    cout<<"check return_long_axon"<<endl;
    cout<<idlist.size()<<endl;
    NeuronTree new_tree;
    for(int i=0; i<idlist.size();i++){
        new_tree.listNeuron.append(nt.listNeuron.at(idlist.at(i)));
    }
    return new_tree;
    cout<<"check finish long_axon_return"<<endl;
}

QList<int> match_axon(NeuronTree axon, NeuronTree lpa){
    QList<int> lpa_id;
    for(int i=0; i<lpa.listNeuron.size();i++){
        NeuronSWC node_lpa=lpa.listNeuron.at(i);
        for(int j=0; j<axon.listNeuron.size();j++){
            NeuronSWC node_axon=axon.listNeuron.at(j);
            if((node_axon.x==node_lpa.x) && (node_axon.y==node_lpa.y) && (node_axon.z==node_lpa.z)){
                lpa_id.push_back(node_axon.n);
            }
        }
    }
    return lpa_id;
}

QList<int> get_subtree(NeuronTree nt, int id){
    bool *visited = new bool [nt.listNeuron.size()];
    QList<int> names;
    for(int m=0; m<nt.listNeuron.size(); m++){
        visited[m]=false;
        names.push_back(nt.listNeuron.at(m).n);
    }

    QVector<QList<int> > children;
    children = QVector<QList<int> >(nt.listNeuron.size(), QList<int>() );
    for (V3DLONG k=0;k<nt.listNeuron.size();k++)
    {
        int pid = names.indexOf(nt.listNeuron.at(k).pn);
        if (pid<0) continue;
        children[pid].push_back(k);
    }

//    QList<int> neworder;

//    // DFS to sort current component;

//    // Initialization
//    QStack<int> pstack;
//    QList<int> visited;
//    for(int i=0;i<nt.listNeuron.size(); i++){visited.append(0);}
//    visited[id]=1;
//    pstack.push(id);
//    neworder.append(id);

//    // Tree traverse
//    bool is_push;
//    int p;
//    while(!pstack.isEmpty()){
//        is_push = false;
//        p = pstack.top();
//        // whether exist unvisited neighbors of pid
//        // if yes, push neighbor to stack;
//        QList<int>::iterator it;
//        //QVector<V3DLONG> cur_neighbors = neighbors.at(pid);
//        for(it = children[p].begin();it!=children[p].end(); ++it)
//        {
//            if(visited.at(*it)==0)
//            {
//                pstack.push(*it);
//                is_push=true;
//                visited[*it]=1;
//                neworder.append(*it);
//                break;
//            }
//        }
//        // else, pop pid
//        if(!is_push){
//            pstack.pop();
//        }
//    }
//    return neworder;


    QList<int> queue,treelist;
    visited[id]=true;
    queue.push_back(id);
    treelist.push_back(id);

    int id_queue;
    cout<<"----------------------------------------------------"<<endl;
    while(!queue.empty()){
        id_queue = queue.front();
        queue.pop_front();

        for(QList<int>::iterator i = children[id_queue].begin();i!=children[id_queue].end();++i){
            if(!visited[*i]){
                visited[*i] = true;
                queue.push_back(*i);
                treelist.push_back(*i);
            }
        }
    }
    cout<<"finish"<<endl;
    return treelist;
}

NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        S.level = node.level;
        S.fea_val = node.fea_val;
        S.seg_id = node.seg_id;
        S.nodeinseg_id = node.nodeinseg_id;
        S.creatmode = node.creatmode;
        S.timestamp = node.timestamp;
        S.tfresindex = node.tfresindex;

        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}

NeuronTree missing_parent(NeuronTree nt){
    // traverse every node of a tree, if a node's parent node is not in the tree, set this node as root.
    QList <int> nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        nlist.append(nt.listNeuron.at(i).n);
    }
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(nlist.lastIndexOf(nt.listNeuron.at(i).pn)<0){
            nt.listNeuron[i].pn = -1;
        }
    }
    return neuronlist_2_neurontree(nt.listNeuron);
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons, QHash<V3DLONG, NeuronSWC> & LUT_newid_to_node)
{
    // Range of LUT values: [0, # deduplicated neuron list)
    QHash<V3DLONG,V3DLONG> LUT;
    V3DLONG cur_id=0;
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        V3DLONG j=0;
        for (j=0;j<i;j++) // Check whether this node is a duplicated with the previous ones
        {
            if (neurons.at(i).x==neurons.at(j).x
                    && neurons.at(i).y==neurons.at(j).y
                    && neurons.at(i).z==neurons.at(j).z)
            {
                break;
            }
        }
        if(i==j){  // not a duplicate
            LUT.insertMulti(neurons.at(i).n, cur_id);
            LUT_newid_to_node.insertMulti(cur_id, neurons.at(j));
            cur_id++;
        }
        else{  // is a duplicate
            LUT.insertMulti(neurons.at(i).n, LUT.value(neurons.at(j).n));
        }
    }
    return (LUT);
}

QVector< QVector<V3DLONG> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<V3DLONG,V3DLONG> & LUT)
{
    // generate neighbor lists for each node, using new ids.
    // LUT (look-up table): old name -> new ids
    // ids are the line numbers
    // names are the node names (neurons.name)
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
    int siz = idlist.size();
    QList<int> nlist;
    for(V3DLONG i=0; i<neurons.size(); i++){nlist.append(neurons.at(i).n);}

//    qDebug()<<"Before defining qvector";
    QVector< QVector<V3DLONG> > neighbors = QVector< QVector<V3DLONG> >(siz, QVector<V3DLONG>() );
//    qDebug()<<"After defining qvector";
//    system("pause");
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        // Find parent node
//        qDebug()<<i;
        int pid_old = nlist.lastIndexOf(neurons.at(i).pn);
        if(pid_old<0){
            continue;  // Skip root nodes
        }
        else{
            int pname_old = neurons.at(pid_old).n;
            int cname_old = neurons.at(i).n;
            int pid_new = LUT.value(pname_old);
            int cid_new = LUT.value(cname_old);
            if((pid_new>=siz) || (cid_new>=siz)){
                v3d_msg(QString("Out of range [0, %1]: pid:%2; cid:%3").arg(siz).arg(pid_new).arg(cid_new));
            }
            // add a new neighbor for the child node
            if(!neighbors.at(cid_new).contains(pid_new)){
//                qDebug()<<QString("Adding edge between %1 and %2").arg(cid_new).arg(pid_new);
                neighbors[cid_new].push_back(pid_new);
            }
            // add a new neighbor for the parent node
            if(!neighbors.at(pid_new).contains(cid_new)){
//                qDebug()<<QString("Adding edge between %1 and %2").arg(pid_new).arg(cid_new);
                neighbors[pid_new].push_back(cid_new);
            }
        }
    }
    return neighbors;
}

QList<V3DLONG> DFS(QVector< QVector<V3DLONG> > neighbors, V3DLONG newrootid, V3DLONG siz)
{
    // siz: size of the whole neuronlist
    // The neuronlist may include multiple components
    // A component is a connected tree
    // Sorted components: other components that have already been sorted.
    // Current component: the component where newroot resides. We will sort it and append it to the sorted components

    // sorted_size: size of sorted components
    // neworder: new order of the sored components
    // *group: id of the current component

    QList<V3DLONG> neworder;

    // DFS to sort current component;

    // Initialization
    QStack<int> pstack;
    QList<int> visited;
    for(int i=0;i<siz; i++){visited.append(0);}
    visited[newrootid]=1;
    pstack.push(newrootid);
    neworder.append(newrootid);

    // Tree traverse
    bool is_push;
    int pid;
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        // whether exist unvisited neighbors of pid
        // if yes, push neighbor to stack;
        QVector<V3DLONG>::iterator it;
        QVector<V3DLONG> cur_neighbors = neighbors.at(pid);
        for(it=cur_neighbors.begin(); it!=cur_neighbors.end(); ++it)
        {
            if(visited.at(*it)==0)
            {
                pstack.push(*it);
                is_push=true;
                visited[*it]=1;
                neworder.append(*it);
                break;
            }
        }
        // else, pop pid
        if(!is_push){
            pstack.pop();
        }
    }
    return neworder;
}

bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres)
{
    // node name list of
    QList<V3DLONG> nlist;
    for(int i=0; i<neurons.size(); i++){
        nlist.append(neurons.at(i).n);
    }

    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, NeuronSWC> LUT_newid_to_node;
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons, LUT_newid_to_node);

    //create a new id list to give every different neuron a new id
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
    V3DLONG siz = idlist.size();

//    // create a LUT from new id to neuron node
//    QHash<V3DLONG, NeuronSWC> LUT_newid_to_node;
//    for(int i=0; i<idlist.size(); i++){
//        NeuronSWC node = neurons.at(nlist.indexOf(LUT.key(idlist.at(i))));
//        LUT_newid_to_node.insertMulti(idlist.at(i), node);
//    }


    // create a vector to keep neighbors of each node
    QVector< QVector<V3DLONG> > neighbors = get_neighbors(neurons, LUT);

    // Find the new id of the new root
    V3DLONG root = 0;
    if (newrootid==VOID)  // If unspecified, use the 1st root as new root.
    {
        for (V3DLONG i=0;i<neurons.size();i++)
            if (neurons.at(i).pn==-1){
                root = idlist.indexOf(LUT.value(neurons.at(i).n));
                break;
            }
    }
    else{
        root = idlist.indexOf(LUT.value(newrootid));
        if (LUT.keys().indexOf(newrootid)==-1)
        {
            v3d_msg("The new root id you have chosen does not exist in the SWC file.");
            return(false);
        }
    }

    //Major steps
    //do a DFS for the the matrix and re-allocate ids for all the nodes
    QList<V3DLONG> neworder;
    QList<V3DLONG> cur_neworder;
    QList<V3DLONG> component_id;
    for(int i=0; i<siz; i++){
        component_id.append(0);
    }
    V3DLONG sorted_size = 0;
    int cur_group = 1;

    // Begin with the new root node and
    // generate the 1st sorted tree.
    cur_neworder= DFS(neighbors, root, siz);
    qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
    sorted_size += cur_neworder.size();
    neworder.append(cur_neworder);
    for(int i=0; i<cur_neworder.size(); i++){
        component_id[cur_neworder.at(i)] = cur_group;
    }
    cout<<"Done 1st DFS"<<endl;

    // Continue to sort the rest of the tree
    while (sorted_size <siz)
    {
        V3DLONG new_root;
        cur_group++;
        for (V3DLONG iter=0;iter<siz;iter++)
        {
            if (!neworder.contains(iter))
            {
                new_root = iter;
                break;
            }
        }
        cur_neworder = DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id[cur_neworder.at(i)] = cur_group;
        }
    }
    qDebug()<<"Number of components before making connections"<<cur_group;

    QList<V3DLONG> output_newroot_list;
//    if((thres != 1000000000) && (thres>0)){  // If distance threshold > 0: make new connections
    if(thres>=0){  // If distance threshold > 0: make new connections
        qDebug()<<"find the point in non-group 1 that is nearest to group 1";
        //find the point in non-group 1 that is nearest to group 1,
        //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
        output_newroot_list.append(root);
        while(cur_group>1)
        {
            qDebug()<<"Remaining components: "<<cur_group;
            double min = VOID;
            double dist2 = 0;
            int mingroup = 1;

            // Find the closest pair of nodes between group 1 and the rest.
            V3DLONG m1,m2;
            for (V3DLONG ii=0;ii<siz;ii++)
            {
                if (component_id[ii]==1)
                {
//                    qDebug()<<QString("Distance check: %1").arg(ii);
                    for (V3DLONG jj=0;jj<siz;jj++)
                        if (component_id[jj]!=1)
                        {
//                            dist2 = computeDist2(neurons.at(nlist.indexOf(LUT.key(ii))),
//                                                 neurons.at(nlist.indexOf(LUT.key(jj))));
                            dist2 = computeDist2(LUT_newid_to_node.value(ii),
                                                 LUT_newid_to_node.value(jj));
                            if (dist2<min)
                            {
                                min = dist2;
                                mingroup = component_id[jj];
                                m1 = ii;
                                m2 = jj;
                            }
                        }
                }
            }
            for (V3DLONG i=0;i<siz;i++)
            {
                if (component_id[i]==mingroup)
                {
                    component_id[i] = 1;
                }
            }
            if (min<=thres)
            {
                qDebug()<<QString("New connection is made between %1 and %2").arg(m1).arg(m2);
                qDebug()<<QString("Original node name: %1 and %2")
                          .arg(LUT_newid_to_node.value(m1).n)
                          .arg(LUT_newid_to_node.value(m2).n);
                if(!neighbors.at(m1).contains(m2)){neighbors[m1].push_back(m2);}
                if(!neighbors.at(m2).contains(m1)){neighbors[m2].push_back(m1);}
            }
            else{  // set component the node closest to group 1 is root
                output_newroot_list.append(m2);
            }
            cur_group--;
        }
        qDebug()<<"Number of components after making connections"<<output_newroot_list.size();
    }
    else{
        int tp_group = 0;
        for(int i=0; i<siz; i++){
            if(component_id.at(i) != tp_group){
                output_newroot_list.append(neworder.at(i));
                tp_group = component_id.at(i);
            }
        }
    }

    // DFS sort of the neuronlist after new connections
    for (int i=0;i<siz;i++)
    {
        component_id[i] = 0;
        neworder[i]= VOID;
    }
    component_id.clear();
    neworder.clear();
    sorted_size = 0;
    cur_group = 1;

    V3DLONG offset=0;
    for(V3DLONG i=0; i<output_newroot_list.size(); i++)
    {
        V3DLONG new_root = output_newroot_list.at(i);
//        qDebug()<<QString("Output component %1, root id is %2").arg(i).arg(new_root);
        V3DLONG cnt = 0;
        // Sort current component;
        cur_neworder= DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id.append(cur_group);
        }
        NeuronSWC S;
        S = LUT_newid_to_node.value(new_root);
        S.n = offset+1;
        S.pn = -1;
        result.append(S);
        cnt++;
        qDebug()<<QString("New root %1:").arg(i)<<S.x<<S.y<<S.z;

        for (V3DLONG ii=offset+1;ii<(sorted_size);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                V3DLONG cid = neworder[ii];
                V3DLONG pid = neworder[jj];
                // If there's an edge between the two nodes, then pid is the parent of cid
                if (pid!=VOID && cid!=VOID && neighbors.at(pid).contains(cid))
                {
                        NeuronSWC S;
                        S = LUT_newid_to_node.value(cid);
                        S.n = ii+1;
                        S.pn = jj+1;

                        result.append(S);
                        cnt++;
                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        offset += cnt;
    }

    if ((sorted_size)<siz) {
        v3d_msg(QString("Error!\nsorted_size:%1\nsize:%2").arg(sorted_size).arg(siz));
        return false;
    }

    // free space.
    neighbors.clear();
    return(true);
}

NeuronTree my_SortSWC(NeuronTree nt, V3DLONG newrootid, double thres){
    NeuronTree new_tree;
    QList<NeuronSWC> neuronlist;
    SortSWC(nt.listNeuron, neuronlist, newrootid, thres);
    new_tree.deepCopy(neuronlist_2_neurontree(neuronlist));
    return new_tree;
}

QList<int> match_axon_and_lpa(NeuronTree axon, NeuronTree lpa){

    QList<int> lpa_id;

    // 1.1 Load whole axon data
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }

    // 2 Check whether match
    bool matched = 1; // Whether the id's of axon and lpa matches
    for(int i=0; i<lpa.listNeuron.size();i++){
        NeuronSWC node=lpa.listNeuron.at(i);
        int i_in_axon = nlist.indexOf(node.n);
        if(i_in_axon == -1){  // lpa id doesn't exsit in axon: break the loop.
            matched = 0;
            break;
        }
        NeuronSWC node_axon = axon.listNeuron.at(i_in_axon);  // The matching node in axon
        if(node.x != node_axon.x || node.y != node_axon.y || node.z != node_axon.z){
            matched = 0;
            break;
        }
        lpa_id.append(node.n);
    }

    if(matched){
        return lpa_id;
    }
    else{
        printf("ID's of axon and lpa do not match!\n");
        int soma = get_soma(axon);
        lpa_id.clear();
        QList<int> lpa = find_long_axon(axon, soma);
        for(int i=0; i<lpa.size(); i++){
            lpa_id.append(axon.listNeuron.at(lpa.at(i)).n);
        }
        return lpa_id;
    }
}

NeuronTree get_subtree_by_name(NeuronTree nt, QList <int> nlist){
    NeuronTree new_tree;
    QList<int> name_list;
    for(int i=0; i<nt.listNeuron.size(); i++){
        name_list.append(nt.listNeuron.at(i).n);
    }
    for(int i=0; i<nlist.size();i++){
        new_tree.listNeuron.append(nt.listNeuron.at(name_list.indexOf(nlist.at(i))));
    }
    return new_tree;
}

bool split_neuron(QString qs_input, QString qs_outdir){
    // Split neurons into different components
    QString qs_tag;
    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){qs_tag = qs_input.left(qs_input.length()-4);}
    if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){qs_tag = qs_input.left(qs_input.length()-5);}

    if(!fexists(qs_input)){
        return 0;
    }
    if(qs_outdir.size()>0){
        if(!qs_outdir.endsWith("/")){qs_outdir=qs_outdir+"/";}
        qs_tag = qs_tag.right(qs_tag.size()-qs_tag.lastIndexOf(("/"))-1);
        qs_tag = qs_outdir+qs_tag;
    }

    // 1. Load data
    NeuronTree nt;
    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC") || qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC"))
    {
        nt = readSWC_file(qs_input);
    }

    // 2. Examine input
    // 2.1 Check soma
    int soma=get_soma(nt);
    cout<<soma<<endl;
    if(soma<0){return 0;}
    if(nt.listNeuron.at(soma).pn != (-1)){
        printf("Exit: soma is not root!\n");
        return 0;
    }

    //2.2 dendrite
    NeuronTree nt_dendrite = return_dendrite(nt,soma);
    nt_dendrite = my_SortSWC(nt_dendrite,VOID,VOID);
    //2.3 axon
    NeuronTree nt_axon = return_axon(nt,soma);
    nt_axon = my_SortSWC(nt_axon,VOID,VOID);
    //2.4 long_projection without branches
    int soma_axon = get_soma(nt_axon);
    if(soma_axon<0){return 0;}
    if(nt_axon.listNeuron.at(soma_axon).pn != (-1)){
        printf("Exit: soma is not root!\n");
        return 0;
    }
    NeuronTree lpa = return_long_axon(nt_axon,soma_axon);
    cout<<"check0"<<endl;
    //lpa = my_SortSWC(lpa,1,0);
    //cout<<"!!!!!!!!!!!!!!"<<endl;
    //2.5 3 parts
    QList<int> l=match_axon_and_lpa(nt_axon,lpa);
    NeuronTree nt_long=get_subtree_by_name(nt_axon,l);
    //nt_long = my_SortSWC(nt_long,1,0);
    QList<int> long_in_axon = match_axon(nt_axon,nt_long);
    cout<<"check1"<<endl;
    //longest path length
    QList<double> path_distance;
    path_distance.append(0);
    double cur_distance = 0;
    double max_dis=0;
    int n_long = nt_long.listNeuron.size();
    for(int i=1; i<n_long; i++){
        cur_distance += computeDist2(nt_long.listNeuron.at(i-1), nt_long.listNeuron.at(i));
        path_distance.append(cur_distance);
        double d=computeDist2(nt_long.listNeuron.at(i),nt_long.listNeuron.at(0));
        max_dis = max(d,max_dis);
    }
    double total_path_distance = cur_distance;
    cout<<"total_path "<<total_path_distance<<endl;
    //dendrite maximal radius
    double den_r=0;
    int n_den = nt_dendrite.listNeuron.size();  
    for(int i=0; i<n_den;i++){
        double dis = computeDist2(nt_dendrite.listNeuron.at(i),nt.listNeuron.at(soma));
        den_r = max(den_r,dis);
    }
    cout<<"check2"<<endl;
    //axon: children_parent list
    QVector<QVector<V3DLONG> > children;
    int n_axon =nt_axon.listNeuron.size();
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<nt_axon.listNeuron.size();i++){
        NeuronSWC node=nt_axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    children = QVector< QVector<V3DLONG> >(n_axon, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<n_axon;i++)
    {
        int pid = nlist.lastIndexOf(nt_axon.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }
    cout<<"check3"<<endl;
    // branches on the longest path & classification for distal/proximal
    QList<int> branch_list;
    map<int,int > child;
    for(int i=0; i<n_long;i++){
        if(children[i].size()>1){
            branch_list.push_back(i);
            int idc1 = children[i].at(0);
            int idc2 = children[i].at(1);
            //cout<<idc1<<"------"<<idc2<<endl;
            if(idc1 != idc2){
            if(long_in_axon.indexOf(nt_axon.listNeuron.at(idc1).n)!= -1){
                child.insert(pair<int,int>(i,idc2));
            }
            else{
                child.insert(pair<int,int>(i,idc1));
            }
            }
            else continue;
        }
    }
    cout<<"check4"<<endl;
    QList<int> b_proximal, b_distal;
    b_proximal.append(1);
    b_distal.append(0);
    int n_branch = branch_list.size();
    for(int i=0;i<n_branch;i++){
        double bd = computeDist2(nt_dendrite.listNeuron.at(0),nt_long.listNeuron.at(branch_list.at(i)));
        //proximal
        if(bd<2*den_r){
            b_proximal.append(1);
        }
        else{b_proximal.append(0);}
        //distal
        if(path_distance.at(branch_list.at(i))/total_path_distance >0.7){
            b_distal.append(1);
        }
        else{
            b_distal.append(0);
        }
    }
    b_distal.append(1);
    b_proximal.append(0);
    cout<<"check5"<<endl;
    bool *indicator = new bool [n_axon];
    for(int i=0;i<n_axon;i++){
        indicator[i]=false;
    }
    cout<<"check5.5"<<endl;
    NeuronTree proximal_axon,distal_axon;
    //proximal
    int cb=0;
    int end_bid,end_id;
    for(int i=0; i<n_long;i++){
        NeuronSWC node=nt_long.listNeuron.at(i);
        //proximal
        if(b_proximal.at(cb)==1 && b_proximal.at(cb+1)==1){
            proximal_axon.listNeuron.append(node);
            indicator[nlist.indexOf(long_in_axon.at(i))]=true;
            //cout<<"^^^^^"<<nlist.indexOf(long_in_axon.at(i))<<endl;
            //cout<<branch_list.at(cb)<<"...."<<i<<endl;
            if(i==branch_list.at(cb)){
                cb=cb+1;
                int ch_id= child.at(i);
                //cout<<i<<"!!!!!!!!!!!!!!!!"<<child.at(i)<<endl;
                QList<int> ch_list=get_subtree(nt_axon,ch_id);
                //cout<<i<<"!!!!!!!!!!!!!!!!"<<ch_id<<endl;
                for(int j=0; j<ch_list.size();j++){
                    NeuronSWC b_node =nt_axon.listNeuron.at(ch_list.at(j));
                    proximal_axon.listNeuron.append(b_node);
                    indicator[ch_list.at(j)]=true;
                }
            }
        }
        else if(b_proximal.at(cb)==1 && b_proximal.at(cb+1)==0){
            end_id=i;
            end_bid = cb;
            break;
            //cout<<end_id<<"......"<<end_bid<<endl;
        }
    }
    cout<<"check6"<<endl;
    //cout<<cb<<"***********"<<branch_list.size()<<endl;
    //distal
    int s=b_distal.indexOf(1);
    int distal_id= nlist.indexOf(long_in_axon.at(branch_list.at(s-1)));
    //cout<<b_distal.size()<<"****************"<<s<<endl;

    if(distal_id>-1){
    QList<int> distal_list=get_subtree(nt_axon,distal_id);
    for(int j=0; j<distal_list.size();j++){
        NeuronSWC d_node =nt_axon.listNeuron.at(distal_list.at(j));
        distal_axon.listNeuron.append(d_node);
        indicator[distal_list.at(j)]=true;
    }
}
    cout<<"check7"<<endl;
    //add a distance to the proximal
    if(end_bid !=0){
    for(int j=0;j<10;j++){
        NeuronSWC node = nt_long.listNeuron.at(end_id+j);
        proximal_axon.listNeuron.append(node);
        indicator[nlist.indexOf(long_in_axon.at(end_id+j))]=true;
    }
    }
    cout<<"check8"<<endl;
    //long projection
    NeuronTree long_projection;
    for(int k=0;k<n_axon;k++){
        if(!indicator[k]){
            NeuronSWC l_node =nt_axon.listNeuron.at(k);
            long_projection.listNeuron.append(l_node);
        }
    }
    cout<<"check9"<<endl;
    //save files
    QString proximal_swc,distal_swc,den_swc,long_swc;
    proximal_swc = qs_tag+".proximal_axon.swc";
    distal_swc = qs_tag+".distal_axon.swc";
    den_swc = qs_tag+".dendrite.swc";
    long_swc = qs_tag+".long_projection.swc";
    writeSWC_file(den_swc, nt_dendrite);
    if(proximal_axon.listNeuron.size()!=0){
    proximal_axon = missing_parent(proximal_axon);
    proximal_axon = my_SortSWC(proximal_axon, proximal_axon.listNeuron.at(0).n, VOID);
    writeSWC_file(proximal_swc, proximal_axon);}
    cout<<"check save proximal"<<endl;
    if(long_projection.listNeuron.size()!=0){
    long_projection = missing_parent(long_projection);
    long_projection = my_SortSWC(long_projection, long_projection.listNeuron.at(0).n, VOID);
    writeSWC_file(long_swc, long_projection);}
    cout<<"check save long"<<endl;
    distal_axon = missing_parent(distal_axon);
    distal_axon = my_SortSWC(distal_axon, distal_axon.listNeuron.at(0).n, VOID);
    writeSWC_file(distal_swc, distal_axon);
    return 1;
}

//analytical part

double Width=0, Height=0, Depth=0, Diameter=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
int N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0, Max_Order=0;
double Pd_ratio=0, Contraction=0, Max_Eux=0, Max_Path=0, BifA_local=0, BifA_remote=0, Soma_surface=0, Fragmentation=0;
int rootidx=0;
double Max_local=0, Min_local=0, Max_remote=0, Min_remote=0;

QVector<QVector<V3DLONG> > childs;

void computeFeature(const NeuronTree & nt, double * features)
{
    Width=0, Height=0, Depth=0, Diameter=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
    N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0, Max_Order=0;
    Pd_ratio=0, Contraction=0, Max_Eux=0, Max_Path=0, BifA_local=0, BifA_remote=0, Soma_surface=0, Fragmentation=0;
    rootidx=0;

    //for debug
    printf( "%s  \n", "compute Feature");

    V3DLONG neuronNum = nt.listNeuron.size();
    if(neuronNum>0){

    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    //find the root
    rootidx = VOID;
    QList<NeuronSWC> list = nt.listNeuron;
        for (int i=0;i<list.size();i++)
        {
          if (list.at(i).pn==-1){
              //compute the first tree in the forest
              rootidx = i;
              break;
           }
        }
    if (rootidx==VOID){
        cerr<<"the input neuron tree does not have a root, please check your data"<<endl;
        return;
    }

    N_node = list.size();
    N_stem = childs[rootidx].size();
    Soma_surface = 4*PI*(list.at(rootidx).r)*(list.at(rootidx).r);

    printf( "%s : %d \n", "N_node", N_node);
    printf( "%s : %f \n", "Soma_surface", Soma_surface);
    printf( "%s : %d \n", "N_stem", N_stem);

    computeLinear(nt);
    computeTree(nt);
    Hausdorff = computeHausdorff(nt);

    //feature # 0: Number of Nodes
    features[0] = N_node;
    //feature #1: Soma Surface
    features[1] = Soma_surface;
    //feature # 2: Number of Stems
    features[2] = N_stem;
    //feature # 3: Number of Bifurcations
    features[3] = N_bifs;
    //feature # 4: Number of Branches
    features[4] = N_branch;
    //feature # 5: Number of Tips
    features[5] = N_tips;
    //feature # 6: Overall Width
    features[6] = Width;
    //feature # 7: Overall Height
    features[7] = Height;
    //feature # 8: Overall Depth
    features[8] = Depth;
    //feature # 9: Average Diameter
    features[9] = Diameter;
    //feature # 10: Total Length
    features[10] = Length;
    //feature # 11: Total Surface
    features[11] = Surface;
    //feature # 12: Total Volume
    features[12] = Volume;
    //feature # 13: Max Euclidean Distance
    features[13] = Max_Eux;
    //feature # 14: Max Path Distance
    features[14] = Max_Path;
    //feature # 15: Max Branch Order
    features[15] = Max_Order;
    //feature # 16: Average Contraction
    features[16] = Contraction;
    //feature # 17: Average Fragmentation
    features[17] = Fragmentation;
    //feature # 18: Average Parent-daughter Ratio
    features[18] = Pd_ratio;
    //feature # 19: Average Bifurcation Angle Local
    features[19] = BifA_local;
    //feature # 20: Average Bifurcation Angle Remote
    features[20] = BifA_remote;
    //feature # 21: Hausdorr Dimension
    features[21] = Hausdorff;		//Hausdorff program crash when running on complex neuron data, we don't use it
    //feature # 22: Max Local Angle
    features[22] = Max_local;
    //feature # 23: Min Local Angle
    features[23] = Min_local;
    //feature # 24: Max Remote Angle
    features[24] = Max_remote;
    //feature # 22: Min Remote Angle
    features[25] = Min_remote;
    }
}

QVector<V3DLONG> getRemoteChild(int t)
{
    QVector<V3DLONG> rchildlist;
    rchildlist.clear();
    int tmp;
    for (int i=0;i<childs[t].size();i++)
    {
        tmp = childs[t].at(i);
        while (childs[tmp].size()==1)
            tmp = childs[tmp].at(0);
        rchildlist.append(tmp);
    }
    return rchildlist;
}

void computeLinear(const NeuronTree & nt)
{
    double xmin,ymin,zmin;
    xmin = ymin = zmin = VOID;
    double xmax,ymax,zmax;
    xmax = ymax = zmax = -VOID;
    QList<NeuronSWC> list = nt.listNeuron;
    NeuronSWC soma = list.at(rootidx);

    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        xmin = min(xmin,curr.x); ymin = min(ymin,curr.y); zmin = min(zmin,curr.z);
        xmax = max(xmax,curr.x); ymax = max(ymax,curr.y); zmax = max(zmax,curr.z);
        if (childs[i].size()==0)
            N_tips++;
        else if (childs[i].size()>1)
            N_bifs++;
        int parent = getParent(i,nt);
        if (parent==VOID) continue;
        double l = dist(curr,list.at(parent));
        Diameter += 2*curr.r;
        Length += l;
        Surface += 2*PI*curr.r*l;
        Volume += PI*curr.r*curr.r*l;
        double lsoma = dist(curr,soma);
        Max_Eux = max(Max_Eux,lsoma);
    }
    Width = xmax-xmin;
    Height = ymax-ymin;
    Depth = zmax-zmin;
    Diameter /= list.size();
}

void computeTree(const NeuronTree & nt)
{
    QList<NeuronSWC> list = nt.listNeuron;
    NeuronSWC soma = nt.listNeuron.at(rootidx);
    double max_l=0, max_r=0, min_l=360, min_r=360;

    double * pathTotal = new double[list.size()];
    int * depth = new int[list.size()];
    for (int i=0;i<list.size();i++)
    {
        pathTotal[i] = 0;
        depth[i] = 0;
    }

    QStack<int> stack = QStack<int>();
    stack.push(rootidx);
    double pathlength,eudist,max_local_ang,max_remote_ang;
    V3DLONG N_ratio = 0, N_Contraction = 0;

    if (childs[rootidx].size()>1)
    {
        double local_ang,remote_ang;
        max_local_ang = 0;
        max_remote_ang = 0;
        int ch_local1 = childs[rootidx][0];
        int ch_local2 = childs[rootidx][1];
        local_ang = angle(list.at(rootidx),list.at(ch_local1),list.at(ch_local2));

        int ch_remote1 = getRemoteChild(rootidx).at(0);
        int ch_remote2 = getRemoteChild(rootidx).at(1);
        remote_ang = angle(list.at(rootidx),list.at(ch_remote1),list.at(ch_remote2));
        if (local_ang==local_ang)
            max_local_ang = max(max_local_ang,local_ang);
        if (remote_ang==remote_ang)
            max_remote_ang = max(max_remote_ang,remote_ang);

        if(local_ang>max_l){max_l=local_ang;}
        if(remote_ang>max_l){max_r=remote_ang;}
        if(local_ang<min_l){min_l=local_ang;}
        if(remote_ang<min_r){min_r=remote_ang;}

        BifA_local += max_local_ang;
        BifA_remote += max_remote_ang;
    }

    int t,tmp,fragment;
    while (!stack.isEmpty())
    {
        t = stack.pop();
        QVector<V3DLONG> child = childs[t];
        for (int i=0;i<child.size();i++)
        {
            N_branch++;
            tmp = child[i];
            if (list[t].r > 0)
            {
                N_ratio ++;
                Pd_ratio += list.at(tmp).r/list.at(t).r;
            }
            pathlength = dist(list.at(tmp),list.at(t));

            fragment = 0;
            while (childs[tmp].size()==1)
            {
                int ch = childs[tmp].at(0);
                pathlength += dist(list.at(ch),list.at(tmp));
                fragment++;
                tmp = ch;
            }
            eudist = dist(list.at(tmp),list.at(t));
            Fragmentation += fragment;
            if (pathlength>0)
            {
                Contraction += eudist/pathlength;
                N_Contraction++;
            }

            //we are reaching a tip point or another branch point, computation for this branch is over
            int chsz = childs[tmp].size();
            if (chsz>1)  //another branch
            {
                stack.push(tmp);

                //compute local bif angle and remote bif angle
                double local_ang,remote_ang;
                max_local_ang = 0;
                max_remote_ang = 0;
                int ch_local1 = childs[tmp][0];
                int ch_local2 = childs[tmp][1];
                local_ang = angle(list.at(tmp),list.at(ch_local1),list.at(ch_local2));

                int ch_remote1 = getRemoteChild(tmp).at(0);
                int ch_remote2 = getRemoteChild(tmp).at(1);
                remote_ang = angle(list.at(tmp),list.at(ch_remote1),list.at(ch_remote2));
                if (local_ang==local_ang)
                    max_local_ang = max(max_local_ang,local_ang);
                if (remote_ang==remote_ang)
                    max_remote_ang = max(max_remote_ang,remote_ang);

                if(local_ang>max_l){max_l=local_ang;}
                if(remote_ang>max_l){max_r=remote_ang;}
                if(local_ang<min_l){min_l=local_ang;}
                if(remote_ang<min_r){min_r=remote_ang;}

                BifA_local += max_local_ang;
                BifA_remote += max_remote_ang;

            }
            pathTotal[tmp] = pathTotal[t] + pathlength;
            depth[tmp] = depth[t] + 1;
        }
    }

    Pd_ratio /= N_ratio;
    Fragmentation /= N_branch;
    Contraction /= N_Contraction;

    BifA_local /= N_bifs;
    BifA_remote /= N_bifs;
    Max_local=max_l;
    Min_local=min_l;
    Max_remote=max_r;
    Min_remote=min_r;


    for (int i=0;i<list.size();i++)
    {
        Max_Path = max(Max_Path,pathTotal[i]);
        Max_Order = max(Max_Order,depth[i]);
    }
    delete pathTotal; pathTotal = NULL;
    delete depth; depth = NULL;
}

void free_matrix(short **mat,int n,int m)

    /* Free a float matrix allocated by matrix(). */
{
    int i;

    for (i = 0; i<n ; i++)
    {
        delete(mat[i]);
        mat[i] = NULL;
    }
    delete(mat);
    mat = NULL;
}

int mark(int m, short r[3], short ** c)
{
    int i, j, k;
    if (m<=0)
        for (k=0; k<3; k++) c[0][k]=r[k]; /*--initialize the pool of marked cells--*/
    else
    {
        for (i=0; i<m; i++)
        {
            if (c[i][0]==r[0] &&
                    c[i][1]==r[1] &&
                    c[i][2]==r[2]) return(m); /*--- already marked ---*/
            if (c[i][0]>=r[0] &&
                    c[i][1]>=r[1] &&
                    c[i][2]>r[2]) break; /*--- insert into ordered set ---*/
        }
        if (i<m) for (j=m; j>i; j--) for (k=0; k<3; k++) c[j][k]=c[j-1][k];
        for (k=0; k<3; k++) c[i][k]=r[k];
    }

    return(m+1);

}

int fillArray(const NeuronTree & nt, short** r1, short** r2)
{
    QList<NeuronSWC> list = nt.listNeuron;

    int siz = list.size();
    for (int t=0;t<siz;t++)
    {
        int s = getParent(t, nt);
        if(s==VOID) s = t;
        int cst=1;
        r2[0][t]=(short)list.at(s).x+cst;
        r2[1][t]=(short)list.at(s).y+cst;
        r2[2][t]=(short)list.at(s).z+1;
        r1[0][t]=(short)list.at(t).x+cst;
        r1[1][t]=(short)list.at(t).y+cst;
        r1[2][t]=(short)list.at(t).z+1;

    }
    return siz;
}

short **matrix(int n,int m)
{
    int i;
    short **mat = new short*[n];
    for (int i=0;i<n;i++)
    {
        mat[i] = new short[m];
        for (int j=0;j<m;j++)
            mat[i][j] = 0;
    }
    /* Return pointer to array of pointers to rows. */
    return mat;

}

double computeHausdorff(const NeuronTree & nt)
{
#define LMINMAX 2
#define LMAXMIN 1
#define NCELL 500000        // max# nonempty lattice cells in Hausdorff analysis

    int n;

    short **r1, **r2;

    n = nt.listNeuron.size();
    r1 = matrix(3,n);
    r2 = matrix(3,n);

    n = fillArray(nt, r1,  r2);

    int i,  k, k1, l, m, cnt, dl, lmin , lmax;
    short r[3], rr[3], **cell;

    int scale;
    float dr[3], rt[3], total;
    float hd, measure[25], length;

    length=0;
    lmin=0;
    lmax=0;
    for (i=1; i<n; i++)
        for (k=0; k<3; k++)
        {
            lmin += abs(r1[k][i]-r2[k][i]);
            if (lmax<abs(r2[k][i]-r1[k][1])) lmax=abs(r2[k][i]-r1[k][1]);
        }
    lmin /= LMINMAX*n;
    lmax /= 2;
    /*------------start with lattice cell >= lmin ------------*/
    if (lmin<1) lmin=1;
    else if (lmin>1)
    {
        lmax /= lmin;
        for (i=1; i<n; i++) for (k=0; k<3; k++)
        {
            r1[k][i] /= lmin;
            r2[k][i] /= lmin;
        }
    }
    if (lmax<=1) return(0.0);
    scale=lmin;
    cnt=0;

    cell = matrix(NCELL,3);
    /*-----------------------------------------------------main loop begin----------------------*/
    while (lmax>LMAXMIN)
    {
        for (k=0; k<3; k++) r[k]=r1[k][1];
        m = mark(0, r, cell);
        for (i=1; i<n; i++) if ((r1[0][i]!=r2[0][i]) ||
                (r1[1][i]!=r2[1][i]) ||
                (r1[2][i]!=r2[2][i]))
        {
            /*-------------------------tracing link-------*/
            total=0.0;
            for (k=0; k<3; k++) total += abs(r2[k][i]-r1[k][i]);
            for (k=0; k<3; k++)
            {
                r[k]=r1[k][i];
                dr[k]=(r2[k][i]-r[k])/total;
                rt[k]=dr[k];
            }
            m=mark(m, r, cell);
            while((r[0]!=r2[0][i]) ||
                    (r[1]!=r2[1][i]) ||
                    (r[2]!=r2[2][i]))
            {
                l=0;
                k1=-1;
                for (k=0; k<3; k++) rr[k]=r2[k][i]-r[k];
                for (k=0; k<3; k++)
                {
                    if ((rt[k]*rr[k]>0) && (abs(l)<abs(rr[k])))
                    {
                        l=rr[k];
                        k1=k;
                    }
                }
                if (l>0)
                {
                    r[k1]++;
                    rt[k1]--;
                }
                else
                {
                    r[k1]--;
                    rt[k1]++;
                }
                for (k=0; k<3; k++) rt[k] += dr[k];
                m=mark(m, r, cell);
                if (m>=NCELL) cerr<<"maximal cell number reached"<<endl;;
                if (m>=NCELL) exit(1);
            }

        }
        measure[cnt]=m;
        cnt++;

        for (i=1; i<n; i++) for (k=0; k<3; k++)
        {
            r1[k][i] /= 2;
            r2[k][i] /= 2;
        }
        lmax /= 2;
        scale *=2;
    }
    /*-----------------------------main loop end-------------------------*/
    free_matrix(r1,3,n);
    free_matrix(r2,3,n);
    free_matrix(cell,NCELL,3);
    /*-----------------------------computing Hausdorff dimension---------*/
    hd=0;
    for (i=0; i<cnt; i++) hd += (i-0.5*(cnt-1))*log(measure[i]);
    hd *= -12.0/(cnt*(cnt*cnt-1.0))/log(2.0);
    return(hd);
}

QList<int> loop_detection(const NeuronTree & nt){
    int siz = nt.listNeuron.size();
    //cout<<siz<<endl;
    QList<int> nlist,plist,soma_candidate,trifur;
    nlist.clear();
    plist.clear();
    QVector< QVector<int> > neighbor;
    QVector< QVector<int> > same_coord;
    neighbor = QVector< QVector<int> >(siz, QVector<int>() );
    same_coord = QVector< QVector<int> >(siz, QVector<int>() );
    bool *visited = new bool [nt.listNeuron.size()];;
    //count xyz pair, record corresponding ids
    cout<<"check2"<<endl;
    cout<<siz<<endl;
    QHash<QString, QList<int> > id_hash;
    for(int i=0; i<siz; i++){
        QString coordi;
        coordi = QString("%1").arg(nt.listNeuron.at(i).x)+"_"+QString("%1").arg(nt.listNeuron.at(i).y)+"_"+QString("%1").arg(nt.listNeuron.at(i).z);
        //qDebug()<<coordi;
        if(!id_hash.contains(coordi)){
            QList<int> tmp;
            tmp.append(i);
            id_hash.insert(coordi,tmp);
        }
        else{
            //cout << "***********"<<id_hash.value(coordi).size()<<endl;
            for(int j=0; j<id_hash.value(coordi).size();j++){
                same_coord[i].push_back(id_hash.value(coordi).at(j));
                same_coord[id_hash.value(coordi).at(j)].push_back(i);
            }
            QList<int> t=id_hash.value(coordi);
            t.append(i);
            id_hash[coordi] = t;
            //cout << "***********"<<id_hash.value(coordi).size()<<endl;
        }
        nlist.append(nt.listNeuron[i].n);
        plist.append(nt.listNeuron[i].pn);
        visited[i]=false;
        //cout << "***********"<<id_hash.value(coordi).size()<<endl;
    }
    //children-parent relation
//    QVector<QVector<V3DLONG> > children;
//    children = QVector< QVector<V3DLONG> >(siz, QVector<V3DLONG>() );
//    for (int i=0;i<siz;i++)
//    {
//        int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
//        if (pid<0) continue;
//        children[pid].push_back(i);
//    }
    cout<<"check3"<<endl;
    //neighbor matrix
    for(int i=0; i<siz; i++){
        QString indicator_codi;
        indicator_codi = QString("%1").arg(nt.listNeuron.at(i).x)+"_"+QString("%1").arg(nt.listNeuron.at(i).y)+"_"+QString("%1").arg(nt.listNeuron.at(i).z);
        //qDebug()<<indicator_codi.toStdString().data();
        int child_id = plist.indexOf(nt.listNeuron.at(i).n);
        int parent_id = nlist.indexOf(nt.listNeuron.at(i).pn);
        //cout<<nt.listNeuron[i].n<<"*************"<<child_id<<endl;
        if(child_id != -1){
            //cout<<"!!!!!!!!!!!!!!!!!!!"<<endl;
            neighbor[i].push_back(child_id);
            neighbor[child_id].push_back(i);
            //cout<<child_id<<endl;
        }
        if(plist.at(i) != -1 && parent_id != -1){
            neighbor[i].push_back(parent_id);
            neighbor[parent_id].push_back(i);
            //cout<<parent_id<<endl;
        }
    }
    cout<<"check4"<<endl;
    // starting points of tree structure
    for(int i=0; i<siz; i++){
        if(nt.listNeuron.at(i).type == 1){
            soma_candidate.append(i);
        }
        QString indi_codi;
        indi_codi = QString("%1").arg(nt.listNeuron.at(i).x)+"_"+QString("%1").arg(nt.listNeuron.at(i).y)+"_"+QString("%1").arg(nt.listNeuron.at(i).z);
        if(nt.listNeuron.at(i).pn==-1 && id_hash.value(indi_codi).size()==1){
            soma_candidate.append(i);
            //cout<<"***************"<<i<<endl;
        }
    }
    cout<<"check5"<<endl;
    //DFS
    QList<int> loop_mark_candidate;
//    QHash <QString, int> cnt;
    for(int j=0;j<soma_candidate.size();j++){
        QList<int> queue;
        queue.push_back(soma_candidate.at(j));
        visited[soma_candidate.at(j)]=true;
        //cout<<"***************"<<soma_candidate.at(j)<<endl;
//        QString indicator=QString("%1").arg(nt.listNeuron.at(soma_candidate.at(j)).x)+"_"+QString("%1").arg(nt.listNeuron.at(soma_candidate.at(j)).y)+"_"+QString("%1").arg(nt.listNeuron.at(soma_candidate.at(j)).z);
//       cnt[indicator]=1;
        int id_queue;
        while(!queue.empty()){
            id_queue = queue.front();
            queue.pop_front();
            if(same_coord[id_queue].size()>0){
                for(QVector<int>::iterator it=same_coord[id_queue].begin();it!=same_coord[id_queue].end();++it){
                    if(!visited[*it]){
                        visited[*it]=true;
                        queue.push_back(*it);
                    }
//                    else{
//                        QString indicator=QString("%1").arg(nt.listNeuron.at(*iter).x)+"_"+QString("%1").arg(nt.listNeuron.at(*iter).y)+"_"+QString("%1").arg(nt.listNeuron.at(*iter).z);
//                        cnt[indicator]=1;
//                    }
            }
            }
                for(QVector<int>::iterator iter = neighbor[id_queue].begin(); iter!=neighbor[id_queue].end(); ++iter){
                    if(!visited[*iter]){
                        visited[*iter]=true;
                        queue.push_back(*iter);
                        //cout<<*iter<<endl;
//                        if(!cnt.count(indicator)){
//                            cnt[indicator]=1;
//                        }
//                        else{
//                            cnt[indicator]++;
//                            cout<<cnt[indicator]<<"***************"<<endl;
//                        }
                    }
                    else{
                        if(!loop_mark_candidate.contains(*iter) && !loop_mark_candidate.contains(id_queue)){
                            int k=0;
                            for(int m=0; m<same_coord[*iter].size(); m++){
                                if(visited[same_coord[*iter].at(m)]!=true){
                                    k=k+1;
                                }
                            }
                            if(k == 0 && same_coord[*iter].size()>0){
                               loop_mark_candidate.append(id_queue);
                               loop_mark_candidate.append(*iter);
                               break;
                            }
                            //cout<<"**********"<<id_queue<<endl;
                        }
//                        if(!cnt.count(indicator)){
//                            cnt[indicator]=1;
                        }
//                        else{
//                            cnt[indicator]++;
//                            cout<<cnt[indicator]<<"***************"<<endl;
//                        }
//                    }
                }
    }
    }
    cout<<"check6"<<endl;
    return loop_mark_candidate;
}

//double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2)
//{
//    double dis=MAX_DOUBLE;
//    for(int i=0;i<neuron1.size();i++)
//    {
//        for(int j=0;j<neuron2.size();j++)
//        {
//            if(dist(neuron1[i],neuron2[j])<dis)
//                dis = dist(neuron1[i],neuron2[j]);
//        }
//    }
//    return dis;
//}

//bool TC_segments(NeuronTree nt, QMultiMap<int, QList<NeuronSWC>> multi_tree, QList<double> percent, QList<double> gap){
//    int siz=nt.listNeuron.size();
//    int count_tree =0;
//    QList<NeuronSWC> nt_list;
//    for(int i=0; i<siz; i++){
//        nt_list.append(nt.listNeuron.at(i));
//    }
//    // count tree number
//    int start=0;
//    for(int i=0; i<siz; i++){
//        if(nt.listNeuron.at(i).pn == -1){
//            count_tree = count_tree+1;
//            // store segments
//            if(count_tree != 1){
//                multi_tree.insert(count_tree-1,nt_list.mid(start,i-1));
//                start = i;
//            }
//        }
//    }
//    multi_tree.insert(count_tree,nt_list.mid(start,siz-1));
//    // calculate min distance between each tree
//    for(int i=0; i<count_tree; i++){
//        double d=MAX_DOUBLE;
//        for(int j=0; j<count_tree; j++){
//            if(i!=j){
//                int count_id1=i+1;
//                int count_id2=j+1;
//                QList<NeuronSWC> neuron1, neuron2;
//                neuron1 = multi_tree.value(count_id1);
//                neuron2 = multi_tree.value(count_id2);
//                double dis = minDist(neuron1,neuron2);
//                if(dis<d){
//                    d=dis;
//                }
//            }
//        }
//        gap.append(d);
//    }
//    //percentage
//    for(int i=0; i<count_tree; i++){
//        QList<NeuronSWC> neuron;
//        neuron = multi_tree.value(i+1);
//        double p;
//        p=100*(double)neuron.size()/(double)nt_list.size();
//        percent.append(p);
//    }
//    return;
//}

QList<TreeNode> tree(NeuronTree nt, int n){
    QList<TreeNode> tree;
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<n;i++){
        NeuronSWC node=nt.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(n, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<n;i++)
    {
        int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }
    QList<int> branch;
    QList<int> tip;
    for(int i=0; i<n; i++){
        if(children[i].size()>1){
            branch.append(i);
        }
        if(children[i].size()==0){
            tip.append(i);
        }
    }
    for(int i=0; i<branch.size(); i++){
        int child1=children[branch.at(i)].at(0);
        int child2=children[branch.at(i)].at(1);
        int tmp=1;
        TreeNode p;
        p.val=branch.at(i);
        while (tmp ==1){
            int b_tmp=branch.lastIndexOf(child1);
            int t_tmp=tip.lastIndexOf(child1);
            if(b_tmp>=0 || t_tmp >=0){
                p.left=child1;
                tmp=0;
            }
            else{
                child1=children[child1].at(0);
            }
        }
        tmp=1;
        while (tmp ==1){
            int b_tmp=branch.lastIndexOf(child2);
            int t_tmp=tip.lastIndexOf(child2);
            if(b_tmp>=0 || t_tmp >=0){
                p.right=child2;
                tmp=0;
            }
            else{
                child2=children[child2].at(0);
            }
        }
        tree.append(p);
    }
    return tree;
}


void tree_structure(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 & callback)
{
    vector<char*> in, inparas, outfiles;
    if(input.size() >= 1) in = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    bool hasOutput;
    if(output.size() >= 1) {outfiles = *((vector<char*> *)output.at(0).p);hasOutput=true;} else {hasOutput=false;}
    QString swc_file = in.at(0);
    NeuronTree nt = readSWC_file(swc_file);
    //V3DLONG newrootid = VOID;
    //double thres = VOID;
    //NeuronTree nt_sort = my_SortSWC(nt, newrootid, thres);
    NeuronTree nt_sort=nt;
    int n=nt_sort.listNeuron.size();
    //cout<<"swc size "<<n<<endl;
    QList<TreeNode> simple_tree;
    simple_tree= tree(nt_sort, n);
    QList<int> nlist;
    for(int i=0; i<n;i++){
        NeuronSWC node=nt_sort.listNeuron.at(i);
        nlist.append(node.n);
    }
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(n, QVector<V3DLONG>() );
    QHash<int, int> parent;
    parent.clear();
    for (V3DLONG i=0;i<n;i++)
    {
        int pid = nlist.lastIndexOf(nt_sort.listNeuron.at(i).pn);
        parent.insert(i,pid);
        if (pid<0) continue;
        children[pid].push_back(i);
    }
    QList<int> tip;
    QList<int> branch;
    for (int i=0; i<n; i++){
        if (children[i].size()==0){
            tip.append(i);
        }
        if (children[i].size()>1){
            branch.append(i);
        }
    }
    QHash<int, int> child_parent;
    child_parent.clear();
    QHash< int, QList<int> > parent_child;
    parent_child.clear();
    //Bifurcation ampl
    double bif_avg_angle;
    double bif_max_angle=0;
    //cout<<"simple tree size="<<simple_tree.size()<<endl;
    for (int i=0; i<simple_tree.size(); i++){
        int a= simple_tree.at(i).val;
        int b1=simple_tree.at(i).left;
        int b2=simple_tree.at(i).right;
        child_parent.insert(b1, a);
        child_parent.insert(b2, a);
        QList<int> tmp;
        tmp.clear();
        tmp.push_back(b1);
        tmp.push_back(b2);
        parent_child.insert(a,tmp);
        double ang=angle(nt_sort.listNeuron.at(a),nt_sort.listNeuron.at(b1),nt_sort.listNeuron.at(b2));
        bif_avg_angle=bif_avg_angle+ang;
        if(ang>bif_max_angle){
            bif_max_angle=ang;
        }
    }
    int maxdepth=0;
    int mindepth=1000;
    //cout<<"tip size ="<<tip.size()<<endl;
    for(int i=0; i<tip.size();i++){
        int tmp=1;
        int id=tip.at(i);
        while (child_parent.contains(id)){
            id=child_parent.value(id);
            tmp=tmp+1;
        }
        if(maxdepth<tmp){
            maxdepth=tmp+1;
        }
        if(mindepth>tmp){
            mindepth=tmp+1;
        }
    }
    //cout<<maxdepth<<" "<<mindepth<<endl;

    //branch_pathlength
    double sum_path=0;
    //cout<<"branch size "<<branch.size()<<endl;
    //cout<<"p size "<<parent.size()<<endl;
    for(int i=0; i<branch.size();i++){
        int tmp_index=branch.at(i);
        if (nt_sort.listNeuron.at(tmp_index).pn == -1){continue;}
        int p_index=parent[tmp_index];
        //cout<<"pindex "<<parent[18]<<endl;
        double pathL=dist(nt_sort.listNeuron.at(tmp_index),nt_sort.listNeuron.at(p_index));
        sum_path=sum_path+pathL;
        //int countloop=1;
        while (!branch.contains(p_index)){
            if(nt_sort.listNeuron.at(p_index).pn == -1){break;}
            sum_path=sum_path+dist(nt_sort.listNeuron.at(p_index),nt_sort.listNeuron.at(parent[p_index]));
            p_index=parent[p_index];
            //cout<<"parent "<<p_index<<endl;
            //countloop++;
        }
    }
    double avg_path=sum_path/branch.size();

    //Terminal degree
    QHash<int, int> tip_count;
    tip_count.clear();
    int terminal_deg=0;
    //cout<<"tip size ="<<tip.size()<<endl;
    for (int i=0; i<tip.size();i++){
        int p_index=parent[tip.at(i)];
        while(p_index != -1){
            if(branch.contains(p_index)){
                terminal_deg=terminal_deg+1;
                if(!tip_count.contains(p_index)){
                    tip_count.insert(p_index,1);
                }
                else{
                    tip_count[p_index]=tip_count[p_index]+1;
                }
            }
            p_index=parent[p_index];
        }
    }
    double degree= terminal_deg/branch.size();

    //cout<<"asymmetry"<<endl;
    //Partition Asymmetry
    double asymmetry=0;
    QHash< int,QList<int> >::const_iterator it;
    for (it = parent_child.constBegin(); it!=parent_child.constEnd(); ++it){
        int left, right;
        int left_id=it.value().at(0);
        int right_id=it.value().at(1);
        if(tip.contains(left_id)){
            left=1;
        }
        else{
            left=tip_count[left_id];
        }
        if(tip.contains(right_id)){
            right=1;
        }
        else{
            right=tip_count[right_id];
        }
        if(left+right == 2){
            continue;
        }
        else{
            asymmetry=(abs(left-right)/(left+right-2))+asymmetry;
        }
        //cout<<"asy "<<asymmetry<<endl;
    }
    double asy=asymmetry/parent_child.size();

    QString neuron_id_split=QString(swc_file).split("/").last();
    QList<QString> path_split=neuron_id_split.split(".");
    QString neuron_split1=path_split.first();
    ofstream myfile;
    myfile.open ("/home/penglab/Desktop/sujun/analysis/arbor/LM.txt",ios::out | ios::app );
    myfile <<neuron_split1.toStdString().c_str()<<" "<<mindepth<<" "<<bif_avg_angle<<" "<<bif_max_angle<<" "<<avg_path<<" "<<degree<<
          " "<<asy<<endl;
    myfile.close();
}


void arbor_main(V3DPluginCallback2 &callback, QWidget *parent){
    QString InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all arbor swcs."),
                                          QDir::currentPath(),
                                          QFileDialog::ShowDirsOnly);
    QString CsvName;
    CsvName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.csv)"
                ));
    if(CsvName.isEmpty())
        return;

    bool flag_sort=TRUE;
    QString title="Sort Function";
    QString content="Do you want to sort swc files";
    QMessageBox::StandardButton result=QMessageBox::information(0,title,content,QMessageBox::Yes|QMessageBox::No);
    switch (result) {
    case QMessageBox::Yes:
        flag_sort=TRUE;
        break;
    case QMessageBox::No:
        flag_sort=FALSE;
        break;
    }

    QString OutputFolder = QFileDialog::getExistingDirectory(parent,
                                                              QString(QObject::tr("Choose the output directory."))
                                                              );
    if(OutputFolder.size()==0){OutputFolder = InputfolderName;}
    arbor_qc(InputfolderName, CsvName, flag_sort, OutputFolder);
}


// arbor analysis
void arbor_qc(QString SwcFolder, QString CsvFile, bool flag_sort, QString OutputFolder){
    QHash <QString, QString>  swc_celltype;
    QHash <QString, QString> swc_region;
    QFile f_type(CsvFile);
    if(!f_type.open(QIODevice::ReadOnly)){
        cout<<"cannot open swc!"<<endl;
    }
    else{
        QString first_line=QString(f_type.readLine()).trimmed();
        QStringList first_sp=first_line.split(",");
        int reg_index=first_sp.indexOf("CellType_Rough");
        int r_index=first_sp.indexOf("region");
        while(!f_type.atEnd()){
            QString line=QString(f_type.readLine()).trimmed();
            QStringList line_sp=line.split(",");
            swc_celltype.insert(line_sp.at(0),line_sp.at(r_index));
            swc_region.insert(line_sp.at(0),line_sp.at(reg_index));
        }
    }

    QString out, outgf;

    outgf = OutputFolder+"/gf.csv";
    out = OutputFolder+"/result.csv";

    ofstream csvOutFile;
    //cout<<outgf.toStdString()<<endl;
    csvOutFile.open(outgf.toStdString().c_str(),ios::out | ios::app);
    csvOutFile<<"Name,Region,Celltype_Rough,Nodes,SomaSurface,Stems,Bifurcations,Branches,Tips,OverallWidth,OverallHeight,OverallDepth";
    csvOutFile<<",AverageDiameter,Length,Surface,Volume,MaxEuclideanDistance,MaxPathDistance,MaxBranchOrder";
    csvOutFile<<",AverageContraction,AverageFragmentation,AverageParent-daughterRatio,AverageBifurcationAngleLocal,AverageBifurcationAngleRemote,HausdorffDimension"<<endl;
    csvOutFile.close();
    ofstream resultcsv;
    resultcsv.open(out.toStdString().c_str(),ios::out | ios::app);
    resultcsv<<"Name,Region,Celltype_Rough,Tips,Width,Height,Depth,Length,MaxBranchOrder,QC_result"<<endl;
    resultcsv.close();

    QDir dir(SwcFolder);
    QStringList nameFilters;
    nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);

    for(int i=0;i<files.size();i++){
        QString swc_file = files.at(i);
        QString swc_path=SwcFolder+"/"+swc_file;
        if(!swc_region.contains(swc_file)){
            v3d_msg(QString("File \"%1\" are not in the csv table. Please double check.").arg(swc_file));
            continue;
        }
        arbor_analysis(swc_path,outgf, swc_celltype,out, swc_region, flag_sort);
    }
    swc_celltype.clear();
    swc_region.clear();
    ifstream readcsvOut, readresult;
    int n_csvOut=0, n_result=0;
    string tmp1, tmp2;
    readcsvOut.open(outgf.toStdString().c_str(),ios::in);
    while(getline(readcsvOut,tmp1)){
        n_csvOut++;
        if(n_csvOut>1){
            break;
        }
    }
    if(n_csvOut<2){
        QFile fileTemp1(outgf.toStdString().c_str());
        fileTemp1.remove();
    }
    readresult.open(out.toStdString().c_str(),ios::in);
    while(getline(readresult,tmp2)){
        n_result++;
        if(n_result>1){
            break;
        }
    }
    if(n_csvOut<2){
        QFile fileTemp2(out.toStdString().c_str());
        fileTemp2.remove();
    }
}

void arbor_analysis(QString swc, QString outgf, QHash <QString, QString> swc_celltype,
                    QString out, QHash <QString, QString> swc_region, bool flag_sort){
    //output
    QFileInfo f(swc);
    QString id=f.fileName();
    QFileInfo outf(out);
    QString outpath=outf.path();
    QString fileSaveName;

    ofstream csvOutFile, result;
    cout<<outgf.toStdString()<<endl;
    csvOutFile.open(outgf.toStdString().c_str(),ios::out | ios::app);
    result.open(out.toStdString().c_str(),ios::out | ios::app);
    if(!csvOutFile.is_open()){
         cerr<<"out Error: cannot open file to save"<<endl;
         return;
    }
    if(!result.is_open()){
        cerr<<"out Error: cannot open file to save final results"<<endl;
        return;
    }

    //sort
    NeuronTree nt_sort;
    if(flag_sort){
        QDir sort_folder;
        QString spath=outpath+"/sort";
        sort_folder.mkdir(spath);
        fileSaveName = spath+"/"+id+"_sorted.swc";
        NeuronTree nt_unsorted = readSWC_file(swc);
        QList<NeuronSWC> neuron_unsorted,sort_result;
        neuron_unsorted=nt_unsorted.listNeuron;
        if (!SortSWC(neuron_unsorted, sort_result , VOID, VOID))
        {
            cout<<"Error in sorting swc"<<endl;
        }
        if (!export_list2file(sort_result, fileSaveName, swc))
        {
            cout<<"Error in writing swc to file"<<endl;
        }
        nt_sort = readSWC_file(fileSaveName);
    }
    else{
        nt_sort = readSWC_file(swc);
    }

    //compute features
    double * swc_features = new double[FNUM];
    computeFeature(nt_sort,swc_features);

    QString celltype;
    QString region;
    QHash<QString, QString>::iterator it;
    for(it=swc_region.begin();it!=swc_region.end();++it){
        if(it.key()==id){
            region=it.value();
            csvOutFile<<it.key().toStdString().c_str()<<","<<it.value().toStdString().c_str();
            result<<it.key().toStdString().c_str()<<","<<it.value().toStdString().c_str();
            //cout<<"**********"<<it.key().toStdString()<<","<<it.value().toStdString()<<endl;
            break;
        }
    }
    for(it=swc_celltype.begin();it!=swc_celltype.end();++it){
        if(it.key()==id){
            celltype=it.value();
            csvOutFile<<","<<it.value().toStdString().c_str();
            result<<","<<it.value().toStdString().c_str();
            break;
        }
    }

    QList<int> feas_id;
    QList<float> feas;
    feas_id<<5<<6<<6<<8<<10<<15;
    for (int i=0;i<feas_id.size();i++)
    {
        feas.append(swc_features[feas_id.at(i)]);
        result<<","<<swc_features[feas_id.at(i)];
    }
    for(int i=0; i<FNUM; i++){
        csvOutFile<<","<<swc_features[i];
    }
    csvOutFile<<endl;
    csvOutFile.close();
    //cout<<"################## "<<id.toStdString()<<" "<<region.toStdString()<<" "<<celltype.toStdString()<<endl;
    //qc
    if(region=="TH"){
        if(celltype=="VPM"){
            if((feas[0]>=29)&&(feas[0]<=93)&&(feas[1]>=5.48)&&(feas[1]<=13.759)&&(feas[2]>=7.189)&&(feas[2]<=19.258)&&(feas[3]>=5.153)&&(feas[3]<=15.497)
                    &&(feas[4]>=93.281)&&(feas[4]<=402.169)&&(feas[5]>=5)&&(feas[5]<=14)){
                result<<","<<1<<endl;
            }
            else{
                result<<","<<0<<endl;
            }
        }
        else if(celltype=="VPL"){
            if((feas[0]>=31)&&(feas[0]<=108)&&(feas[1]>=5.916)&&(feas[1]<=14.946)&&(feas[2]>=7.662)&&(feas[2]<=17.703)&&(feas[3]>=4.611)&&(feas[3]<=15.628)
                    &&(feas[4]>=95.924)&&(feas[4]<=369.248)&&(feas[5]>=6)&&(feas[5]<=15)){
                result<<","<<1<<endl;
            }
            else{
                result<<","<<0<<endl;
            }
        }
        else if(celltype=="LGd"){
            if((feas[0]>=47)&&(feas[0]<=82)&&(feas[1]>=6.816)&&(feas[1]<=14.477)&&(feas[2]>=3.949)&&(feas[2]<=18.641)&&(feas[3]>=5.125)&&(feas[3]<=16.95)
                    &&(feas[4]>=126.229)&&(feas[4]<=358.143)&&(feas[5]>=7)&&(feas[5]<=11)){
                result<<","<<1<<endl;
            }
            else{
                result<<","<<0<<endl;
            }
        }
        else if(celltype=="MG"){
            if((feas[0]>=34)&&(feas[0]<=79)&&(feas[1]>=6.149)&&(feas[1]<=14.498)&&(feas[2]>=7.059)&&(feas[2]<=22.429)&&(feas[3]>=6.26)&&(feas[3]<=14.093)
                    &&(feas[4]>=120.773)&&(feas[4]<=318.078)&&(feas[5]>=6)&&(feas[5]<=16)){
                result<<","<<1<<endl;
            }
            else{
                result<<","<<0<<endl;
            }
        }
        else{
            if((feas[0]>=29)&&(feas[0]<=111)&&(feas[1]>=6.132)&&(feas[1]<=19.839)&&(feas[2]>=7.010)&&(feas[2]<=21.136)&&(feas[3]>=6.29)&&(feas[3]<=21.51)
                    &&(feas[4]>=118.877)&&(feas[4]<=499.613)&&(feas[5]>=5)&&(feas[5]<=20)){
                result<<","<<1<<endl;
            }
            else{
                result<<","<<0<<endl;
            }
        }
    }
    else if((region=="CNU")){
        if(celltype=="CP"){
            if(feas[4]<350){
                if((feas[0]>=26)&&(feas[0]<=109)&&(feas[1]>=5.942)&&(feas[1]<=21.221)&&(feas[2]>=3.806)&&(feas[2]<=22.034)&&(feas[3]>=5.486)&&(feas[3]<=18.734)
                        &&(feas[4]>=81.289)&&(feas[4]<=344.946)&&(feas[5]>=4)&&(feas[5]<=18)){
                    result<<","<<1<<endl;
                }
                else{
                    result<<","<<0<<endl;
                }
            }
            else {
                if((feas[0]>=40)&&(feas[0]<=208)&&(feas[1]>=13.271)&&(feas[1]<=66.578)&&(feas[2]>=12.03)&&(feas[2]<=72.919)&&(feas[3]>=11.288)&&(feas[3]<=54.891)
                        &&(feas[4]>=263.618)&&(feas[4]<=1312.98)&&(feas[5]>=8)&&(feas[5]<=29)){
                    result<<","<<1<<endl;
                }
                else{
                    result<<","<<0<<endl;
                }
            }
        }
        else{
            if((feas[0]>=1)&&(feas[0]<=67)&&(feas[1]>=2.665)&&(feas[1]<=20.478)&&(feas[2]>=2.687)&&(feas[2]<=32.034)&&(feas[3]>=0.956)&&(feas[3]<=20.952)
                    &&(feas[4]>=6.028)&&(feas[4]<=263.696)&&(feas[5]>=1)&&(feas[5]<=12)){
                result<<","<<1<<endl;
            }
            else{
                result<<","<<0<<endl;
            }
        }

    }
    else if(region=="CTX"){
        if((feas[0]>=22)&&(feas[0]<=121)&&(feas[1]>=6.77)&&(feas[1]<=31.354)&&(feas[2]>=6.683)&&(feas[2]<=46.143)&&(feas[3]>=6.36)&&(feas[3]<=35.99)
                &&(feas[4]>=71.317)&&(feas[4]<=585.043)&&(feas[5]>=5)&&(feas[5]<=30)){
            result<<","<<1<<endl;
        }
        else{
            result<<","<<0<<endl;
        }
    }
    else{
        if((feas[0]>=21)&&(feas[0]<=125)&&(feas[1]>=6.132)&&(feas[1]<=38.328)&&(feas[2]>=6.353)&&(feas[2]<=46.256)&&(feas[3]>=5.592)&&(feas[3]<=39.227)
                &&(feas[4]>=68.541)&&(feas[4]<=662.401)&&(feas[5]>=5)&&(feas[5]<=29)){
            result<<","<<1<<endl;
        }
        else{
            result<<","<<0<<endl;
        }
    }
    result.close();
}


