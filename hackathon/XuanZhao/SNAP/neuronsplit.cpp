#include "neuronsplit.h"

#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

mutex mtx;
condition_variable cv;

vector<NeuronTree> ntsG;


bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result, V3DLONG somaN){
    return SortSWC(neurons,result,somaN,0);
}

bool sortSWC(NeuronTree& nt, V3DLONG somaN){
    QList<NeuronSWC> sortListNeuron = QList<NeuronSWC>();
    if(!nt.listNeuron.isEmpty()){
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
        sortSWC(nt.listNeuron,sortListNeuron,somaN);
        nt.listNeuron.clear();
        nt.listNeuron = sortListNeuron;
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
    }
    return true;
}

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees){
    NeuronTree merge = NeuronTree();
    if(neuronTrees.empty()){
        return merge;
    }
    V3DLONG n= 0;
    for(int i=0; i<neuronTrees[0].listNeuron.size(); i++){
        merge.listNeuron.append(neuronTrees[0].listNeuron[i]);
        if(neuronTrees[0].listNeuron[i].n>n)
            n = neuronTrees[0].listNeuron[i].n;
    }
    if(neuronTrees.size() == 1){
        return merge;
    }

    qDebug()<<"max n: "<<n;
//    n++;
    for(int i=1; i<neuronTrees.size(); i++){
        NeuronTree& nt = neuronTrees[i];
        QList<NeuronSWC>& listNeuron = nt.listNeuron;
        if(listNeuron.isEmpty()){
            continue;
        }
        V3DLONG minInd = listNeuron[0].n;

        for(int j=1; j<listNeuron.size(); j++){
            if(listNeuron[j].n<minInd)
                minInd = listNeuron[j].n;
            if(minInd<0)
                qDebug()<<"Found illegal neuron node index which is less than 0 in mergeNeuronTrees()!";
        }
        qDebug()<<"minInd: "<<minInd;
        V3DLONG n0 = n;
        for(int j=0; j<listNeuron.size(); j++){
            NeuronSWC v = listNeuron[j];
            v.n = (n0+2) + listNeuron[j].n - minInd;
            v.parent = (listNeuron[j].parent<0) ? -1 : ((n0+2) + listNeuron[j].parent - minInd);

            merge.listNeuron.append(v);
            if(v.n>n){
                n = v.n;
            }
        }
        qDebug()<<"max n: "<<n;
    }

    for(int i=0; i<merge.listNeuron.size(); i++){
        merge.hashNeuron.insert((int) merge.listNeuron[i].n,i);
    }

    return merge;
}

vector<NeuronTree> splitNeuronTree(NeuronTree nt, const vector<long long> &somaIndexes, const LandmarkList& markers){
    vector<NeuronTree> nts;
    for(int i=0; i<somaIndexes.size(); ++i){
        NeuronTree tmpnt;
        tmpnt.deepCopy(nt);
        sortSWC(tmpnt,somaIndexes[i]);
        BranchTree bt;
        bt.initialize(tmpnt);
        vector<V3DLONG> somaiIndexes = convertMarkers2SomaNList(tmpnt,markers);;
        bt.pruningAdjacentSoma3(somaiIndexes,2);
        nts.push_back(bt.getPrunedNeuronTree());
    }
    return nts;
}

//vector<NeuronTree> splitNeuronTreeMultiThread(NeuronTree nt, const vector<long long> &somaIndexes, const LandmarkList& markers){
//    ntsG.clear();

//}

void splitNeuronTreeManu(V3DPluginCallback2 &callback){
    v3dhandle curwin = callback.currentImageWindow();
    QList<NeuronTree>* windowTrees = callback.getHandleNeuronTrees_3DGlobalViewer(curwin);
//    sortSWC(nt,VOID);
    if(windowTrees->size() != 1){
        v3d_msg("Please load one swc!");
        return;
    }
    NeuronTree nt = windowTrees->at(0);
    cout<<"nt size: "<<nt.listNeuron.size()<<endl;
    LandmarkList markers = callback.getLandmark(curwin);

    cout<<"marker size: "<<markers.size()<<endl;

    vector<V3DLONG> somaNList = convertMarkers2SomaNList(nt,markers);
    vector<NeuronTree> nts = splitNeuronTree(nt,somaNList,markers);

    windowTrees->clear();
    for(int i=0; i<nts.size(); ++i){
        for(int j=0; j<nts[i].listNeuron.size(); ++j){
            nts[i].listNeuron[j].type = i+2;
        }
        nts[i].color = RGBA8({hanchuan_colortable[i%hanchuan_colortable_size()][0],
                hanchuan_colortable[i%hanchuan_colortable_size()][1],
                hanchuan_colortable[i%hanchuan_colortable_size()][2],0});
        windowTrees->append(nts[i]);
    }
    qDebug()<<"render "<<nts.size()<<" swc end";

    callback.update_3DViewer((V3dR_MainWindow*)curwin);
}










