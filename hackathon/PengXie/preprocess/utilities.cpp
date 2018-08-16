#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"
using namespace std;

#include "pre_processing_main.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neuron_connector_func.h"


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

bool my_saveANO(QString fileNameHeader, bool swc=true, bool apo=true){
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

    if(swc){fprintf(fp, "SWCFILE=%s\n", qPrintable(fileNameHeader+QString(".swc")));}
    if(apo){fprintf(fp, "APOFILE=%s\n", qPrintable(fileNameHeader+QString(".apo")));}
    if(fp){fclose(fp);}
    return true;
}

NeuronTree rm_nodes(NeuronTree nt, QList<int> list){
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

    return nt;
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
            if(soma_ct>1){return -1;}
        }
    }
    if(soma_ct==0){return -1;}
    return soma;
}

NeuronTree single_tree(NeuronTree nt, int soma){
    QList<int> pList;
    const int N=nt.listNeuron.size();
    int subtree_ct=0;
    // check whether this is a single tree
    for(int i=0; i<N; i++){
        // root check
        if(nt.listNeuron.at(i).pn<0){
            pList.append(-1);
            subtree_ct++;
        }
        else{
            int pid = nt.listNeuron.at(i).pn-1;
            pList.append(pid);
        }
    }

    // Case 1
    if(subtree_ct==1){return nt;}

    // Case 2
    if(subtree_ct==0){
        printf("No root found in the swc.\n");
        return nt;
    }

    // Case 3
    if(subtree_ct>1){
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

        // DFS using stack
        while(!pstack.isEmpty()){
            is_push = false;
            pid = pstack.top();
            // whether exist unvisited children of pid
            // if yes, push child to stack;
            for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
                if((nt.listNeuron.at(i).pn-1)==pid && visited.at(i)==0){
                    pstack.push(i);
                    visited[i]=1;
                    new_tree.listNeuron.append(nt.listNeuron.at(i));
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

NeuronTree get_subtree_by_id(NeuronTree nt, QList <int> idlist){
    NeuronTree new_tree;
    for(int i=0; i<idlist.size();i++){
        new_tree.listNeuron.append(nt.listNeuron.at(idlist.at(i)));
    }
    return new_tree;
}

NeuronTree color_subtree_by_id(NeuronTree nt, QList <int> idlist, int new_type=0){
    for(int i=0; i<idlist.size();i++){
        nt.listNeuron[idlist.at(i)].type = new_type;
    }
    return nt;
}

NeuronTree get_subtree_by_type(NeuronTree nt, int type, bool keep_soma=1){
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

