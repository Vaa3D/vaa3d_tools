//functions for axon analysis
//by Peng Xie
//2018-08-16

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "axon_func.h"

bool axon_retype(QString input_swc, QString output_swc){
    printf("axon_retype\n");
    if(output_swc.size()==0){
        if (input_swc.endsWith(".swc") || input_swc.endsWith(".SWC")){output_swc = input_swc.left(input_swc.length()-4)+".retype.swc";}
        if (input_swc.endsWith(".eswc") || input_swc.endsWith(".ESWC")){output_swc = input_swc.left(input_swc.length()-5)+".retype.swc";}
    }
    NeuronTree axon = readSWC_file(input_swc);
    NeuronTree lpa = get_subtree_by_type(axon, 0);
    lpa = single_tree(axon, 0);

    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    // label non-long-projection axon braches
    printf("\tFinding non-long-projection axons\n");
    int cur_retype = 2;
    int size_thres = 10; // Label branch with a different type if passes size_thres.
    QList<int> branch_ind;
    for(int i=0; i<axon.listNeuron.size(); i++){
        branch_ind.clear();
        NeuronSWC node = axon.listNeuron.at(i);
        int pn_ind = plist.indexOf(node.pn);
        if(axon.listNeuron.at(pn_ind).type ==0 && node.type != 0){  // start of a non-long-projection branch
            cout << i << "\t" << pn_ind << "\t";
            // label the branch by DFS
            // Initialization
            int pid=i;
            QList<int> visited;
            for(int j=0;j<axon.listNeuron.size(); j++){visited.append(0);}
            QStack<int> pstack;
            pstack.push(pid);
            bool is_push = false;
            visited[pid]=1;
            branch_ind.append(pid);

            // DFS using stack
            while(!pstack.isEmpty()){
                is_push = false;
                pid = pstack.top();
                // whether exist unvisited children of pid
                // if yes, push child to stack;
                for(int j=0; j<axon.listNeuron.size();j++){  // This loop can be more efficient, improve it later!
                    if((axon.listNeuron.at(j).pn)==nlist.at(pid) && visited.at(j)==0){
                        pstack.push(j);
                        visited[j]=1;
                        branch_ind.append(j);
                        is_push=true;
                        break;
                    }
                }
                // else, pop pid
                if(!is_push){
                    pstack.pop();
                }
            }
            if(branch_ind.size()>size_thres){
                cur_retype++;
                cout << cur_retype << endl;
                for(int j=0; j<branch_ind.size(); j++){
                    axon.listNeuron[branch_ind.at(j)].type=cur_retype;
                }
            }
        }
    }
    writeSWC_file(output_swc, axon);
    return 1;
}
