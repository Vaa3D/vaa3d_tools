
#include "Node_type_correction_func.h"

QVector< QVector<V3DLONG> > get_children(NeuronTree nt){
    QVector< QVector<V3DLONG> > children;
    QList<NeuronSWC> n_list = nt.listNeuron;
    for(int i=0; i<n_list.size(); i++){
        QVector<V3DLONG> empty_vector;
        empty_vector.clear();
        children.append(empty_vector);
    }
    for(int i=0; i<n_list.size(); i++){
        int parent = n_list.at(i).pn;
        if(parent>=0){
            children[nt.hashNeuron.value(parent)].append(i);
        }
    }
    return children;
}

int get_root(NeuronTree nt){
    QList<NeuronSWC> n_list = nt.listNeuron;
    int root;
    for(int i=0; i<n_list.size(); i++){
        if(n_list.at(i).pn == -1){
            root = i;
            return root;
        }
    }
    v3d_msg("No root node found!");
    return -1;
}

void node_type_correction(QString input_swc, QString output_swc)
{
    // This plugin does the following:
    // 1. Find swc nodes whose types are invalid (valid types: 1 (soma), 2 (axon), 3 (basal dendrite), 4 (apical dendrite) )
    // 2. Make corrections when possible, otherwise label them
    // 3. Generate a report of node type counts.

    QList<int> valid_types;
    valid_types << 2 << 3 << 4;

    NeuronTree nt = readSWC_file(input_swc);
    QVector< QVector<V3DLONG> > children = get_children(nt);
    int root = get_root(nt);
    if(root < 0){
        return;
    }

    // DFS
    QStack<int> pstack;
    QList<bool> visited;
    for(int i=0; i<nt.listNeuron.size(); i++){visited.append(false);}

    // Begin
    pstack.push(root);
    visited[root] = true;
    if(nt.listNeuron.at(root).type!=1){nt.listNeuron[root].type = 0;}

    int cur_type = 0;

    // DFS search
    while(!pstack.empty()){
        bool new_push = false;
        int cur_node = pstack.top();
//        qDebug()<<"cur_node: "<<cur_node;
        QVector<V3DLONG>::iterator it;
        QVector<V3DLONG> cur_children = children.at(cur_node);
        for(it=cur_children.begin(); it!=cur_children.end(); ++it)
        {
            cur_type = nt.listNeuron.at(cur_node).type;
            if(!visited.at(*it)){
                pstack.push(*it);
                new_push = true;
                visited[*it] = true;
                if(!valid_types.contains(nt.listNeuron.at(*it).type)){nt.listNeuron[*it].type = cur_type;}
                else{
                    cur_type = nt.listNeuron.at(*it).type;
                }
                break;
            }
        }
        if(!new_push){
            int cur_node = pstack.pop();
            if((!valid_types.contains(nt.listNeuron.at(cur_node).type)) && (cur_node != root)){
                QVector<V3DLONG>::iterator it;
                QVector<V3DLONG> cur_children = children.at(cur_node);
                for(it=cur_children.begin(); it!=cur_children.end(); ++it)
                {
                    if(valid_types.contains(nt.listNeuron.at(*it).type)){
                        nt.listNeuron[cur_node].type = nt.listNeuron.at(*it).type;
                        break;
                    }
                }
            }
        }
    }

    // To be implemented: Display number of node types, e.g. types: 1(1), 2(500), 3(200), 4(50), others (0)
    writeSWC_file(output_swc, nt);
    return;
}
