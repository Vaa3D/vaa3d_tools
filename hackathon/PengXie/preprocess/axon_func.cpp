//functions for axon analysis
//by Peng Xie
//2018-08-16

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "axon_func.h"


QList<double> branch_distribution(QString whole_axon_swc, QString lpa_swc){
    // Input is an swc of axon, with the long-projection axon labeled as 0
    // id's of whole_axon_swc and lpa_swc should match

    printf("axon_retype\n");

    // 1. Load data
    NeuronTree axon = readSWC_file(whole_axon_swc);
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }

    NeuronTree lpa = readSWC_file(lpa_swc);
    QList<double> lpa_id;
    QList<bool> is_proximal;
    for(int i=0; i<lpa.listNeuron.size();i++){
        NeuronSWC node=lpa.listNeuron.at(i);
        lpa_id.append(node.n);
        if((i*1.0/lpa.listNeuron.size())<0.2){
            is_proximal.append(1);
        }
        else{
            is_proximal.append(0);
        }
    }

    // 2. Branch size at every node of lpa
    QList<double> lpa_size;
    for(int i=0; i<lpa.listNeuron.size();i++){lpa_size.append(0);}
    QList<int> branch_ind;
    for(int i=1; i<axon.listNeuron.size(); i++){
        branch_ind.clear();
        NeuronSWC node = axon.listNeuron.at(i);
        int pn_ind = nlist.indexOf(node.pn);
        // find the start of a non-long-projection branch
        if(axon.listNeuron.at(pn_ind).type ==0 && node.type != 0){
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

        }
        lpa_size[i] += branch_ind.size();
    }

    // 3. Branch density at every node of lpa
    QList<double> lpa_density;
    for(int i=0; i<lpa.listNeuron.size();i++){
        lpa_density.append(0);
        for(int j=0; j<lpa.listNeuron.size();i++){
            lpa_density[i] += lpa_size.at(j) * exp(-pow(i-j, 2));
        }
    }

    return lpa_density;
}

bool export_branch_distribution(QList<double> lpa_density, QString output_fileName){
    FILE * fp=0;
    fp = fopen((char *)qPrintable(output_fileName), "wt");
    for(int i=0; i<lpa_density.size(); i++){
        fprintf(fp, "%f\n", lpa_density.at(i));
    }
    fclose(fp);
    return 1;
}

bool axon_retype(QString whole_axon_swc, QString lpa_swc, QString output_swc, bool proximal_distal){
    // Input is an swc of axon, with the long-projection axon labeled as 0
    // id's of whole_axon_swc and lpa_swc should match

    printf("Welcome to use axon_retype\n");

    // 1. Load data
    if(output_swc.size()==0){
        if (whole_axon_swc.endsWith(".swc") || whole_axon_swc.endsWith(".SWC")){output_swc = whole_axon_swc.left(whole_axon_swc.length()-4)+".retype.swc";}
        if (whole_axon_swc.endsWith(".eswc") || whole_axon_swc.endsWith(".ESWC")){output_swc = whole_axon_swc.left(whole_axon_swc.length()-5)+".retype.swc";}
    }
    NeuronTree axon = readSWC_file(whole_axon_swc);
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }

    NeuronTree lpa = readSWC_file(lpa_swc);
    QList<double> lpa_id;
    QList<bool> is_proximal;
    for(int i=0; i<lpa.listNeuron.size();i++){
        NeuronSWC node=lpa.listNeuron.at(i);
        lpa_id.append(node.n);
        if((i*1.0/lpa.listNeuron.size())<0.2){
            is_proximal.append(1);
        }
        else{
            is_proximal.append(0);
        }
    }

    // 2. label non-long-projection axon braches
    printf("\tFinding non-long-projection axons\n");
    // 2.1 default mode: retype axon clusters as proximal or distal
    int proximal_retype = 5;
    int distal_retype = 6;
    // 2.2 optional model: label branch with a different type if passes size_thres.
    int cur_retype = 2;
    int size_thres = 10;
    QList<int> branch_ind;
    for(int i=1; i<axon.listNeuron.size(); i++){
        branch_ind.clear();
        NeuronSWC node = axon.listNeuron.at(i);
        // find the start of a non-long-projection branch
        if(lpa_id.indexOf(node.n) == (-1) && lpa_id.indexOf(node.pn) != (-1)){

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

        }
        // retype the branch
        if(branch_ind.size()>0){
            // Mode 1
            if(proximal_distal){
                int branch_head = branch_ind.at(0);
                int branch_at_lpa = lpa_id.indexOf(plist.at(branch_head));
                // Proximal cluster
                if(is_proximal.at(branch_at_lpa)){
                    cur_retype = proximal_retype;
                }
                // Distal cluster
                else{
                    cur_retype = distal_retype;
                }
                for(int j=0; j<branch_ind.size(); j++){
                    axon.listNeuron[branch_ind.at(j)].type=cur_retype;
                }
            }
            // Mode 2
            else{
                if(branch_ind.size()>size_thres){
                    cur_retype++;
                    for(int j=0; j<branch_ind.size(); j++){
                        axon.listNeuron[branch_ind.at(j)].type=cur_retype;
                    }
                }
            }
        }
    }
    writeSWC_file(output_swc, axon);
    return 1;
}
