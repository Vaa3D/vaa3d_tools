#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"
#include "fstream"
using namespace std;

#include "pre_processing_main.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neuron_connector_func.h"
#include "utilities.h"

#define MAXSIZE 100000

bool isEqual(float x, float y)
{
  const float epsilon = 1e-5f;
  return (fabs(x - y) <= epsilon);
}

NeuronTree neuron_deepcopy(NeuronTree nt){
    QList<NeuronSWC> new_list;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        NeuronSWC S;
        S.x = node.x;
        S.y = node.y;
        S.z = node.z;
        S.r = node.r;
        S.type = node.type;
        S.n = node.n;
        S.pn = node.pn;
        new_list.append(S);
    }
    NeuronTree new_tree;
    new_tree.listNeuron = new_list;
    return new_tree;
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

bool whether_identical(NeuronTree nt1, NeuronTree nt2){
    printf("welcome to use whether_identical\n");
    if(nt1.listNeuron.size() != nt2.listNeuron.size()){
        printf("Unequal sizes!\n");
        return 0;
    }
    for(int i=0; i<nt1.listNeuron.size(); i++){
        NeuronSWC node1 = nt1.listNeuron.at(i);
        NeuronSWC node2 = nt2.listNeuron.at(i);
        if(node1.x != node2.x
                || node1.y != node2.y
                || node1.z != node2.z
                || node1.r != node2.r
                || node1.type != node2.type
                || node1.n != node2.n
                || node1.pn != node2.pn){
            printf("%d\t%d\t%f\t%f\t%f\t%f\t%d\n", node1.n, node1.type, node1.x, node1.y, node1.z, node1.r, node1.pn);
            printf("%d\t%d\t%f\t%f\t%f\t%f\t%d\n", node2.n, node2.type, node2.x, node2.y, node2.z, node2.r, node2.pn);
            return 0;
        }
    }
    return 1;
}
bool fexists(QString filename)
{
  std::ifstream ifile(qPrintable(filename));
  return ifile.good();
}

CellAPO get_marker(NeuronSWC node, double vol, double color_r, double color_g, double color_b){
    CellAPO marker;
    marker.x = node.x;
    marker.y = node.y;
    marker.z = node.z;
    marker.volsize = vol;
    marker.color.r = color_r;
    marker.color.g = color_g;
    marker.color.b = color_b;
    return marker;
}
NeuronSWC CellAPO_to_NeuronSWC(CellAPO marker){
    NeuronSWC node;
    node.x = marker.x;
    node.y = marker.y;
    node.z = marker.z;
    return node;
}
int count_root(NeuronTree nt){
    int ct = 0;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(node.pn == -1){
            ct++;
        }
    }
    return ct;
}

bool my_saveANO(QString fileNameHeader, bool swc, bool apo, QString swc_name){
    FILE * fp=0;
    fp = fopen((char *)qPrintable(fileNameHeader+QString(".ano")), "wt");
    if (!fp)
    {
        v3d_msg("Fail to open file to write.");
        return false;
    }
    if(fileNameHeader.count("/")>0){
        fileNameHeader = fileNameHeader.right(fileNameHeader.size()-fileNameHeader.lastIndexOf("/")-1);
    }

    if(swc){
        if(swc_name.length()==0){
            fprintf(fp, "SWCFILE=%s\n", qPrintable(fileNameHeader+QString(".swc")));
        }
        else{
            swc_name = swc_name.right(swc_name.size()-swc_name.lastIndexOf("/")-1);
            fprintf(fp, "SWCFILE=%s\n", qPrintable(swc_name));
        }
    }
    if(apo){fprintf(fp, "APOFILE=%s\n", qPrintable(fileNameHeader+QString(".apo")));}
    if(fp){fclose(fp);}
    return true;
}

NeuronTree rm_nodes(NeuronTree nt, QList<int> list){
    if(list.size()==0){
        return nt;
    }
    qSort(list);
    cout << "rm_nodes started" <<endl;
    cout << list.last() << "\t" << list.size() << "\t" << nt.listNeuron.size()<<endl;
    if(list.last()>=nt.listNeuron.size()){  // Index out of range
        cout << list.at(list.last())<<endl<<"rm_nodes Error: index out of range\n";
        return nt;
    }
    for(int i=(list.size()-1); i>=0; i--){
        nt.listNeuron.removeAt(list.at(i));
    }
    cout << "rm_nodes finished" <<endl;    
    return missing_parent(nt);
}

int get_soma(NeuronTree nt){
    printf("Checking soma\n");
    const int N=nt.listNeuron.size();
    int soma;
    int soma_ct=0;
    QList<int> candidate_list;

    for(int i=0; i<N; i++){
        if(nt.listNeuron.at(i).pn==(-1)){ // 2019-03-14: Changed for data release
            candidate_list.append(i);
        }
    }

    for(int i=0; i<candidate_list.size(); i++){
        int cur_candidate = candidate_list.at(i);
        if(nt.listNeuron.at(cur_candidate).type==1){ // 2019-03-14: Changed for data release
            soma = cur_candidate;
            soma_ct++;
            break;
        }
    }
    if(soma_ct==0){
        qDebug()<<"warning: Use 1st root as soma of swc.";
        soma = candidate_list.at(0);
    }
    return soma;
}

QList<int> getChild(int t, QList <NeuronSWC> & list, QHash<int,int> & LUT)
{
        QList<int> childlist = QList<int>();
        int pan;
        for (int i=0;i<list.size();i++)
        {
                pan = list.at(i).pn;
                if (pan==-1) continue;
                if (LUT.value(pan)==t)
                        childlist.append(i);
        }
        return childlist;
}

QList<int> get_components(NeuronTree nt){
    //connected component
    printf("welcom to use get_components\n");
    int N = nt.listNeuron.size();
    QList<int> nList;
    QList<int> pList;
    QList<int> components;
    int ncomponents = 0;
    for(V3DLONG i=0; i<N; i++){
        if(nt.listNeuron.at(i).pn<0){
            components.append(ncomponents);
            ncomponents++;
        }
        else{
            components.append(-1);
        }
        nList.append(nt.listNeuron.at(i).n);
        pList.append(nt.listNeuron.at(i).pn);
    }

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


    //assign nodes to components
    for(int cid=0; cid<ncomponents; cid++){
        QStack<int> pstack;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        int pid=components.indexOf(cid);
        pstack.push(pid);

        // DFS to mark the component of currunt root (cid);
        QList<int> visited;
        for(int i=0;i<N; i++){visited.append(0);}
        bool is_push = false;
        visited[pid]=1;
        while(!pstack.isEmpty()){
            is_push = false;
            pid = pstack.top();
            // whether exist unvisited children of pid
            // if yes, push child to stack;
            for(V3DLONG *i=children[pid].begin(); i!=children[pid].end(); ++i){  // This loop can be more efficient, improve it later!
                if(visited.at(*i)==0){
                    pstack.push(*i);
                    visited[*i]=1;
                    is_push=true;
                    components[*i]=cid;
                    break;
                }
            }
            // else, pop pid
            if(!is_push){
                pstack.pop();
            }
        }
    }
    QList<int> ucomponents = components.toSet().toList();
    if(ucomponents.lastIndexOf(-1)>=0){
        printf("Cannot find parents for some nodes!\n");
    }
    printf("\t%d components found.\n", ncomponents);
    return components;
}
NeuronTree get_ith_component(NeuronTree nt, QList<int> components, int ith){
    QList<int> list;
    NeuronTree new_tree;
    if(nt.listNeuron.size() != components.size()){
        printf("Error: sizes of neurontree and components must be the same!\n");
        return new_tree;
    }
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(components.at(i)==ith){
            list.append(i);
        }
    }
    new_tree = get_subtree_by_id(nt, list);
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

//NeuronTree single_tree(NeuronTree nt, int soma){
//    QList<int> pList;
//    QList<int> nList;
//    const int N=nt.listNeuron.size();
//    int subtree_ct=0;
//    // check whether this is a single tree
//    for(int i=0; i<N; i++){
//        // root check
//        if(nt.listNeuron.at(i).pn<0){
//            pList.append(-1);
//            subtree_ct++;
//        }
//        else{
//            pList.append(nt.listNeuron.at(i).pn);
//        }
//        nList.append(nt.listNeuron.at(i).n);
//    }

////    // Case 1
//    if(subtree_ct==1){return nt;}

//    // Case 2
//    if(subtree_ct==0){
//        printf("No root found in the swc.\n");
//        return nt;
//    }

//    // Case 3
//    if(subtree_ct>0){
//        printf("More than one single trees in the input swc. Only the first connected with soma will be reported.\n");
//        NeuronTree new_tree;
//        // DFS to extract the first soma-connected tree;

//        // Initialization
//        int pid=soma;
//        QList<int> visited;
//        for(int i=0;i<N; i++){visited.append(0);}
//        QStack<int> pstack;
//        pstack.push(pid);
//        bool is_push = false;
//        visited[pid]=1;
//        new_tree.listNeuron.append(nt.listNeuron.at(pid));
////        cout<<nt.listNeuron.at(pid).n<<"\t"<<nt.listNeuron.at(pid).x<<"\t"<<nt.listNeuron.at(pid).y<<"\t"<<nt.listNeuron.at(pid).z<<endl;

//        // DFS using stack
//        while(!pstack.isEmpty()){
//            is_push = false;
//            pid = pstack.top();
//            // whether exist unvisited children of pid
//            // if yes, push child to stack;
//            for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
//                NeuronSWC node = nt.listNeuron.at(i);
//                if(nList.lastIndexOf(node.pn)==pid && visited.at(i)==0){
//                    pstack.push(i);
//                    visited[i]=1;
//                    new_tree.listNeuron.append(node);
////                    cout<<node.n<<"\t"<<node.x<<"\t"<<node.y<<"\t"<<node.z<<endl;
//                    is_push=true;
//                    break;
//                }
//            }
//            // else, pop pid
//            if(!is_push){
//                pstack.pop();
//            }
//        }
//        return new_tree;
//    }
//}

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

NeuronTree get_subtree_by_id(NeuronTree nt, QList <int> idlist){
    NeuronTree new_tree;
    for(int i=0; i<idlist.size();i++){
        new_tree.listNeuron.append(nt.listNeuron.at(idlist.at(i)));
    }
    return new_tree;
}

NeuronTree color_subtree_by_id(NeuronTree nt, QList <int> idlist, int new_type){
    for(int i=0; i<idlist.size();i++){
        nt.listNeuron[idlist.at(i)].type = new_type;
    }
    return nt;
}
NeuronTree color_components(NeuronTree nt, QList<int> components){
    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].type=components.at(i)+1;
    }
    return nt;
}
NeuronTree get_subtree_by_type(NeuronTree nt, int type, bool keep_soma){
    NeuronTree new_tree;
    NeuronSWC node;
    for(int i=0;i<nt.listNeuron.size();i++){
        node = nt.listNeuron.at(i);
        if(keep_soma){
            if(node.type==type || node.type==1){
                new_tree.listNeuron.append(node);
            }
        }
        else{
            if(node.type==type){
                new_tree.listNeuron.append(node);
            }
        }
    }
    return new_tree;
}
QList<int> get_tips(NeuronTree nt, bool include_root){
    // whether a node is a tip;
    QList<int> tip_list;
    QList<int> plist;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        if(include_root & nt.listNeuron.at(i).pn == -1){
            tip_list.append(i);
        }
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)==0){tip_list.append(i);}
    }
    return(tip_list);
}
QList<NeuronSWC> neuronlist_cat(QList<NeuronSWC> nl1, QList<NeuronSWC> nl2){
    // Concatenate nl2 after nl1;
    // id's of nl1/2 should start from 1;
    if(nl1.size()==0){
        return nl2;
    }
    if(nl2.size()==0){
        return nl1;
    }
    QList<NeuronSWC> empty_list;
    if(nl1.at(0).n != 1 || nl2.at(0).n != 1){
        printf("names of neuron trees must begin with 1!\n");
        return empty_list;
    }
    int nl1_size = nl1.size();
    for(int i=0; i<nl2.size(); i++){
        nl2[i].n += nl1_size;
        if(nl2.at(i).pn != -1){  // Keep root node as it was.
            nl2[i].pn += nl1_size;
        }
    }
    nl1.append(nl2);
    return nl1;
}
int count_dup_for_node(NeuronTree nt, int node_id){
    NeuronSWC node = nt.listNeuron.at(node_id);
    int ct = 0;
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(((node.x==nt.listNeuron.at(i).x) && (node.y==nt.listNeuron.at(i).y) && (node.z==nt.listNeuron.at(i).z)))
        {
            ct++;
        }
    }
    return ct;
}

bool check_duplicate(NeuronTree nt){
    printf("welcome to use remove_duplicte\n");

    QList<int> plist;
    QList<int> nlist;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        NeuronSWC node = nt.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    QList<int> del_list;
    QList<int> case_ct;
    int total_ct=0;
    for(int i=0; i<4; i++){case_ct.append(0);}
    for(int i=0; i<N; i++){
        if(del_list.indexOf(i)>0){continue;}  // This node has already been deduplicated;
        NeuronSWC node1 = nt.listNeuron.at(i);
        for(int j=(i+1); j<N; j++){
            NeuronSWC node2 = nt.listNeuron.at(j);
            if(node1.x==node2.x && node1.y==node2.y && node1.z==node2.z){
                total_ct ++;

                // Case 1
                if(node1.pn == -1 && node2.pn != -1){
                    case_ct[0]++;
//                    // Delete node1;
//                    del_list.append(i);
//                    // Connect node1's children to node2
//                    while(plist.indexOf(node1.n) > 0){
//                        int k = plist.lastIndexOf(node1.n);
//                        nt.listNeuron[k].pn = node2.n;
//                        plist[k]=node2.n;
//                    }
                }
                // Case 2
                if(node1.pn != -1 && node2.pn == -1){
                    case_ct[1]++;
//                    // Delete node2;
//                    del_list.append(j);
//                    // Connect node2's children to node1
//                    while(plist.indexOf(node2.n) > 0){
//                        int k = plist.lastIndexOf(node2.n);
//                        nt.listNeuron[k].pn = node1.n;
//                        plist[k]=node1.n;
//                    }
                }
                // Case 3
                if(node1.pn == -1 && node2.pn == -1){
                    case_ct[2]++;
//                    // Delete node2;
//                    del_list.append(j);
//                    // Connect node2's children to node1
//                    while(plist.indexOf(node2.n) > 0){
//                        int k = plist.lastIndexOf(node2.n);
//                        nt.listNeuron[k].pn = node1.n;
//                        plist[k]=node1.n;
//                    }
                }
                // Case 4
                if(node1.pn != -1 && node2.pn != -1){
                    if(node1.pn != node2.n && node2.pn != node1.n){
                        case_ct[3]++;
//                        printf("%d\t%d\n", node1.n, node1.pn);
//                        printf("%d\t%d\n", node2.n, node2.pn);
                    }
                }
            }
        }
    }
    printf("Total duplicates:\t%d\n", total_ct);
    for(int i=0; i<4; i++){printf("Case %d:\t%d\n", (i+1), case_ct.at(i));}
    return (total_ct==0);
}

double dist_to_parent(NeuronTree nt, int i, double xscale, double yscale, double zscale){
    NeuronSWC node = nt.listNeuron.at(i);
    QList<int> nlist;
    for(int j=0; j<nt.listNeuron.size(); j++){
        nlist.append(nt.listNeuron.at(j).pn);
    }
    int pid = nlist.indexOf(node.pn);
    if(pid == -1){return 0;}  // this is a root
    return computeDist2(node, nt.listNeuron.at(pid), xscale, yscale, zscale);
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

QList<NeuronSWC> get_soma_from_APO(QList<CellAPO> markers){
    QList<NeuronSWC> S_list;
    NeuronSWC S;
    int soma_rootid=1;
    if(markers.size()>0)
    {
        if(markers.size()==1)
        {
            //markers.at(0).operator XYZ;
            S.x = markers.at(0).x;
            S.y = markers.at(0).y;
            S.z = markers.at(0).z;
            S.r = 100;
            S.type = 1;
            S.n = soma_rootid;
            S.pn = -1;
            S_list.append(S);
            qDebug()<< "Marker found. Using it as root."; //<< neuron.size()<< neuron.at(neuron.size()-1).pn;
        }
        else{
            for(int i=0;i<markers.size();i++)
            {
                if(markers.at(i).comment == "soma")//markers.at(i).color.r == 0 && markers.at(i).color.g == 0 && markers.at(i).color.b == 255)
                {
                    //markers.at(0).operator XYZ;
                    S.x = markers.at(i).x;
                    S.y = markers.at(i).y;
                    S.z = markers.at(i).z;
                    S.type = 1;
                    S.r = 100;
                    S.n = soma_rootid;
                    S.pn = -1;
                    S_list.append(S);
                    break;
                }
            }
            qDebug()<< "Warning: more than one marker in the file, taking one commented as 'soma'";
        }
    }
    return S_list;
}
