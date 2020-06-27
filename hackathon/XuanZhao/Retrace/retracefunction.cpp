#include "retracefunction.h"

#include "app2.h"
#include "branchtree.h"
#include "../../../../v3d_main/neuron_editing/neuron_sim_scores.h"

//bool operator<(const Triple& p1, const Triple& p2){
//    if(p1.r<p2.r){
//        return true;
//    }else if(p1.r>p2.r) {
//        return false;
//    }else{
//        if(p1.g<p2.g){
//            return true;
//        }else if(p1.g>p2.g) {
//            return false;
//        }else {
//            if(p1.b<p2.b){
//                return true;
//            }else{
//                return false;
//            }
//        }
//    }
//}

bool Triple::operator <(const Triple& other) const{
    if(this->r<other.r){
        return true;
    }else if(this->r>other.r) {
        return false;
    }else{
        if(this->g<other.g){
            return true;
        }else if(this->g>other.g) {
            return false;
        }else {
            if(this->b<other.b){
                return true;
            }else{
                return false;
            }
        }
    }
}


vector<vector<LocationSimple> > getApp2InMarkers(QList<CellAPO> markers){
    vector<vector<LocationSimple> > result;

    map<Triple,int> colorMap = map<Triple,int>();
    for(int i=0; i<colorNum; i++){
        Triple c= Triple{basicColorTable[i][0],basicColorTable[i][1],basicColorTable[i][2]};
//        c.r = basicColorTable[i][0];
//        c.g = basicColorTable[i][1];
//        c.b = basicColorTable[i][2];
//        c.a = 255;
        colorMap[c] = i+1;
    }

    map<int,int> typeMap = map<int,int>();
    for(int i=0; i<markers.size(); i++){
        RGBA8 cc = markers[i].color;
        Triple c = Triple(cc.r,cc.g,cc.b);
        int type = colorMap[c];
        if (type%2 == 1){
            vector<LocationSimple> group = vector<LocationSimple>();
            LocationSimple l = LocationSimple(markers[i].x,markers[i].y,markers[i].z);
            group.push_back(l);
            typeMap[type] = group.size()-1;
            result.push_back(group);
        }
    }

    for(int i=0; i<markers.size(); i++){
        RGBA8 cc = markers[i].color;
        Triple c = Triple(cc.r,cc.g,cc.b);
        int type = colorMap[c];
        if (type%2 == 0){
            int index = typeMap[type-1];
            LocationSimple l = LocationSimple(markers[i].x,markers[i].y,markers[i].z);
            result[index].push_back(l);
        }
    }

    return result;
}

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees){
    NeuronTree merge = NeuronTree();

    int n= 0;
    for(int i=0; i<neuronTrees.size(); i++){
        NeuronTree& nt = neuronTrees[i];
        QList<NeuronSWC>& listNeuron = nt.listNeuron;
        if(listNeuron.isEmpty()){
            continue;
        }
        int minInd = listNeuron[0].n;

        for(int j=1; j<listNeuron.size(); j++){
            if(listNeuron[j].n<minInd)
                minInd = (int) listNeuron[j].n;
            if(minInd<0)
                qDebug()<<"Found illegal neuron node index which is less than 0 in mergeNeuronTrees()!";
        }
        int n0 = n;
        for(int j=0; j<listNeuron.size(); j++){
            NeuronSWC v = listNeuron[j];
//            v.x = listNeuron[j].x;
//            v.y = listNeuron[j].y;
//            v.z = listNeuron[j].z;
//            v.radius = listNeuron[j].radius;
//            v.type = listNeuron[j].type;
            v.n = (n0+1) + listNeuron[j].n - minInd;
            v.parent = (listNeuron[j].parent<0) ? -1 : ((n0+1) + listNeuron[j].parent - minInd);

            merge.listNeuron.append(v);
            n++;
        }
    }

    for(int i=0; i<merge.listNeuron.size(); i++){
        merge.hashNeuron.insert((int) merge.listNeuron[i].n,i);
    }

    return merge;
}

void deleteSameBranch(NeuronTree& target, const NeuronTree& ori, double thres){
    BranchTree tb = BranchTree();
    tb.initialize(target);
    qDebug()<<"---------get level0Index------";
    vector<int> level0Index = vector<int>();
    tb.get_level_index(level0Index,0);

    qDebug()<<"level0Index size: "<<level0Index.size();

    vector<bool> branchDFlag = vector<bool>(tb.branchs.size(),false);
    for(int i=0; i<level0Index.size(); i++){

        qDebug()<<"i: "<<i<<" index: "<<level0Index[i];

        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        qDebug()<<"---------------aaaaa----------------";
        tb.branchs[level0Index[i]].get_pointsIndex_of_branch(pointsIndex,target);

        qDebug()<<"pointsIndex size: "<<pointsIndex.size();
        double d = 0;
        int count = 0;
        for(int j=0; j<pointsIndex.size(); j++){
            const NeuronSWC& p = target.listNeuron[pointsIndex[j]];
            double pToSwc = dist_pt_to_swc(XYZ(p.x,p.y,p.z),&ori);
            d += pToSwc;
            count++;
            if(j > 10)
                break;
        }
        d /= count;

        qDebug()<<"count: "<<count<<" d: "<<d;

        if(d<thres){
            branchDFlag[level0Index[i]] = true;
        }
    }

    qDebug()<<"---------------start to flag dbranch---------";

    int maxLevel = tb.get_max_level();

    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<tb.branchs.size(); i++){
        Branch* b = &(tb.branchs[i]);
        branchMap[b] = i;
    }

    for(int level = 1; level<=maxLevel; level++){
        vector<int> levelIndex = vector<int>();
        tb.get_level_index(levelIndex,level);
        for(int j=0; j<levelIndex.size(); j++){
            int prtIndex = branchMap[tb.branchs[levelIndex[j]].parent];
            if(branchDFlag[prtIndex]){
                branchDFlag[levelIndex[j]] = true;
            }
        }
    }

    qDebug()<<"------------start to delete point--------------";

    vector<bool> pointDIndex = vector<bool>(target.listNeuron.size(),false);
    for(int i=0; i<branchDFlag.size(); i++){
        if(branchDFlag[i]){
            vector<V3DLONG> pointIndex = vector<V3DLONG>();
            tb.branchs[branchDFlag[i]].get_pointsIndex_of_branch(pointIndex,target);
            for(int j=1; j<pointIndex.size(); j++){
                pointDIndex[pointIndex[j]] = true;
            }
        }
    }

    qDebug()<<"---------start to copy------------------";

    QList<NeuronSWC> listNeuron = QList<NeuronSWC>();
    for(int i=0; i<pointDIndex.size(); i++){
        if(!pointDIndex[i]){
            NeuronSWC p = target.listNeuron[i];
            listNeuron.append(p);
        }
    }

    target.listNeuron.clear();
    target.listNeuron = listNeuron;

}

NeuronTree retrace(QString apoPath, QString eswcPath, QString brainDir, int resolution, V3DPluginCallback2& callback){

    const double th = 5.0;

    int times = pow(2,resolution-1);
    qDebug()<<"times: "<<times;

    NeuronTree nt = readSWC_file(eswcPath);
    QList<CellAPO> markers = readAPO_file(apoPath);

    vector<vector<LocationSimple> > inMarkers = getApp2InMarkers(markers);

    qDebug()<<"inMarkers size: "<<inMarkers.size();

    vector<NeuronTree> app2NeuronTrees = vector<NeuronTree>();
    app2NeuronTrees.push_back(nt);

    for(int m=0; m<inMarkers.size(); m++){
        vector<LocationSimple>& inMarker = inMarkers[m];
        qDebug()<<"m: "<<m<<" inMarker size: "<<inMarker.size();
        LocationSimple& p = inMarker[0];
        double minD = INT_MAX;
        int minIndex = -1;
        for(int j=0; j<nt.listNeuron.size(); j++){
            double d = dist_L2(XYZ(p.x-1, p.y-1 , p.z-1),XYZ(nt.listNeuron[j].x, nt.listNeuron[j].y, nt.listNeuron[j].z));
            if(d<minD){
                minD = d;
                minIndex = j;
            }
        }
        if(minD<th && minIndex != -1){
            p.x = nt.listNeuron[minIndex].x + 1;
            p.y = nt.listNeuron[minIndex].y + 1;
            p.z = nt.listNeuron[minIndex].z + 1;
        }

        size_t x0 = INT_MAX, x1 = 0, y0 = INT_MAX, y1 = 0, z0 = INT_MAX, z1 = 0;

        for(int j=0; j<inMarker.size(); j++){
            if(inMarker[j].x<x0) x0 = inMarker[j].x;
            if(inMarker[j].x>x1) x1 = inMarker[j].x;
            if(inMarker[j].y<y0) y0 = inMarker[j].y;
            if(inMarker[j].y>y1) y1 = inMarker[j].y;
            if(inMarker[j].z<z0) z0 = inMarker[j].z;
            if(inMarker[j].z>z1) z1 = inMarker[j].z;
        }

        x0 -= 20;
        x1 += 20;
        y0 -= 20;
        y1 += 20;
        z0 -= 20;
        z1 += 20;
        x0 /= times, x1/= times, y0 /= times, y1 /= times, z0 /= times, z1 /= times;

        qDebug()<<"x0: "<<x0<<" x1: "<<x1<<" y0: "<<y0<<" y1: "<<y1<<" z0: "<<z0<<" z1: "<<z1;

        unsigned char* pdata1d = callback.getSubVolumeTeraFly(brainDir.toStdString(),x0,x1,y0,y1,z0,z1);
        V3DLONG sz0 = x1-x0;
        V3DLONG sz1 = y1-y0;
        V3DLONG sz2 = z1-z0;
        V3DLONG sz3 = 1;

        Image4DSimple* imgApp2 = new Image4DSimple();
        imgApp2->setData(pdata1d,sz0,sz1,sz2,sz3,V3D_UINT8);


        paraApp2 para = paraApp2();
        para.p4dImage = imgApp2;
        for(int j=0; j<inMarker.size(); j++){
            LocationSimple l = LocationSimple(inMarker[j].x/times - x0, inMarker[j].y/times - y0, inMarker[j].z/times - z0);
            para.landmarks.push_back(l);
        }
        para.xc0 = 0, para.xc1 = sz0-1, para.yc0 = 0, para.yc1 = sz1-1, para.zc0 = 0, para.zc1 = sz2-1;
        para.bkg_thresh = -1;
        proc_app2(para);

        NeuronTree& result = para.result;
        for(int j=0; j<result.listNeuron.size(); j++){
            result.listNeuron[j].x = (result.listNeuron[j].x + x0)*times;
            result.listNeuron[j].y = (result.listNeuron[j].y + y0)*times;
            result.listNeuron[j].z = (result.listNeuron[j].z + z0)*times;
            result.listNeuron[j].type = 2;
        }

        qDebug()<<"NeuronTree size: "<<result.listNeuron.size();
        result.hashNeuron.clear();
        for(int j=0; j<result.listNeuron.size(); j++){
            result.hashNeuron.insert(result.listNeuron[j].n,j);
        }

        deleteSameBranch(result,nt,th);
        NeuronTree app2NeuronTree;
        app2NeuronTree.deepCopy(result);
        app2NeuronTrees.push_back(app2NeuronTree);
    }

    NeuronTree merge = mergeNeuronTrees(app2NeuronTrees);

    writeESWC_file(eswcPath,merge);

    return merge;
}
