#include "retracefunction.h"

#include "app2.h"
#include "branchtree.h"
#include "../../../../v3d_main/neuron_editing/neuron_sim_scores.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
//#include "../../../../released_plugins/v3d_plugins/ada_threshold/ada_threshold.cpp"

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
bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result){
    return SortSWC(neurons,result,VOID,1);
}

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
        qDebug()<<"i: "<<i<<" type: "<<type;
        if (type%2 == 1){
            vector<LocationSimple> group = vector<LocationSimple>();
            LocationSimple l = LocationSimple(markers[i].x,markers[i].y,markers[i].z);
            group.push_back(l);
            result.push_back(group);
            typeMap[type] = result.size() - 1;
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
//            v.x = listNeuron[j].x;
//            v.y = listNeuron[j].y;
//            v.z = listNeuron[j].z;
//            v.radius = listNeuron[j].radius;
//            v.type = listNeuron[j].type;
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

void deleteSameBranch(NeuronTree& target, const NeuronTree& ori, double thres){
    BranchTree tb = BranchTree();
    tb.initialize(target);
    qDebug()<<"---------get level0Index------";
    vector<int> level0Index = vector<int>();
    tb.get_level_index(level0Index,0);

    qDebug()<<"level0Index size: "<<level0Index.size();
    if(level0Index.size() != 2){
        if(level0Index.size() == 1){
            level0Index.clear();
            tb.get_level_index(level0Index,1);
            if(level0Index.size() != 2)
                return;
        }else {
            return;
        }
    }

    vector<bool> branchDFlag = vector<bool>(tb.branchs.size(),false);
    double d[2];

    for(int i=0; i<2; i++){

        qDebug()<<"i: "<<i<<" index: "<<level0Index[i];

        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        qDebug()<<"---------------aaaaa----------------";
        tb.branchs[level0Index[i]].get_pointsIndex_of_branch(pointsIndex,target);

        qDebug()<<"pointsIndex size: "<<pointsIndex.size();
        d[i] = 0;
        int count = 0;
        for(int j=1; j<pointsIndex.size(); j++){
            const NeuronSWC& p = target.listNeuron[pointsIndex[j]];
            double pToSwc = dist_pt_to_swc(XYZ(p.x,p.y,p.z),&ori);
            d[i] += pToSwc;
            count++;
            if(j > 3)
                break;
        }
        d[i] /= count;


        qDebug()<<"count: "<<count<<" d: "<<d[i];

//        if(d<thres){
//            branchDFlag[level0Index[i]] = true;
//        }
    }
    int si = d[0]>d[1]?1:0;
    branchDFlag[level0Index[si]] = true;


    qDebug()<<"---------------start to flag dbranch---------";

    int maxLevel = tb.get_max_level();
    qDebug()<<"maxLevel: "<<maxLevel;

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
                qDebug()<<"delete branch: "<<levelIndex[j];
            }
        }
    }

    qDebug()<<"------------start to delete point--------------";

    int pointDNum = 0;
    vector<bool> pointDIndex = vector<bool>(target.listNeuron.size(),false);
    for(int i=0; i<branchDFlag.size(); i++){
        if(branchDFlag[i]){
            vector<V3DLONG> pointIndex = vector<V3DLONG>();
            pointIndex.clear();

            tb.branchs[i].get_pointsIndex_of_branch(pointIndex,target);
            qDebug()<<"point size: "<<pointIndex.size();
            for(int j=1; j<pointIndex.size(); j++){
                pointDIndex[pointIndex[j]] = true;
                pointDNum++;
            }
        }
    }

    qDebug()<<"delete point number: "<<pointDNum;

    qDebug()<<"---------start to copy------------------";

    QList<NeuronSWC> listNeuron = QList<NeuronSWC>();
    listNeuron.clear();
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

    const double th = 10.0;

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
        QList<NeuronSWC> sortListNeuron;

        sortSWC(result.listNeuron,sortListNeuron);
        result.listNeuron.clear();
        result.listNeuron = sortListNeuron;
        result.hashNeuron.clear();
        for(int j=0; j<result.listNeuron.size(); j++){
            result.hashNeuron.insert(result.listNeuron[j].n,j);
        }
        NeuronTree app2NeuronTree;
        app2NeuronTree.deepCopy(result);
        app2NeuronTrees.push_back(app2NeuronTree);
        if(imgApp2)
            delete imgApp2;
//        if(pdata1d)
//            delete[] pdata1d;
    }

    NeuronTree merge = mergeNeuronTrees(app2NeuronTrees);

    writeESWC_file(eswcPath,merge);

    return merge;
}

void app2Terafly(int type, bool threshold, V3DPluginCallback2 &callback){
    const Image4DSimple* image = callback.getImageTeraFly();
    qDebug()<<"x: "<<image->getXDim()<<" y: "<<image->getYDim()<<" z: "<<image->getZDim();

    QString fileName = image->getFileName();
    qDebug()<<"file name: "<<fileName;
    QStringList ls = fileName.split(',');
    qDebug()<<ls[2].split('[')[1]<<" "<<ls[3].split(']')[0]<<" "<<ls[4].split('[')[1]<<" "<<ls[5].split(']')[0]<<" "<<ls[6].split('[')[1]<<" "<<ls[7].split(']')[0];


    int dim = ls[1].split('x')[1].toInt();
    qDebug()<<dim;

    int x0 = ls[2].split('[')[1].toInt();
    int x1 = ls[3].split(']')[0].toInt();
    int y0 = ls[4].split('[')[1].toInt();
    int y1 = ls[5].split(']')[0].toInt();
    int z0 = ls[6].split('[')[1].toInt();
    int z1 = ls[7].split(']')[0].toInt();

    LandmarkList app2Markers;
    LandmarkList markers = callback.getLandmarkTeraFly();
    for(int i=0; i<markers.size(); i++){
        qDebug()<<"i "<<i<<" x: "<<markers[i].x<<" y: "<<markers[i].y<<" z: "<<markers[i].z;

        if(markers[i].color.r == 255 && markers[i].color.g == 255 && markers[i].color.b == 255){
            app2Markers.append(markers[i]);
        }
    }

    if(app2Markers.isEmpty()){
        return;
    }




    QString path =callback.getPathTeraFly();
    qDebug()<<"path: "<<path.toStdString().c_str();

    V3DLONG* sz = 0;
    callback.getDimTeraFly(path.toStdString(),sz);
    for(int i=0; i<3; i++){
        qDebug()<<i<<": "<<sz[i];
    }

    double times = sz[1]/(double) dim;
    NeuronTree nt = callback.getSWCTeraFly();

    double minD = INT_MAX;
    int minIndex = -1;
    LocationSimple& s = app2Markers[0];
    for(int i=0; i<nt.listNeuron.size(); i++){
        double d = dist_L2(XYZ(s.x-1, s.y-1 , s.z-1),XYZ(nt.listNeuron[i].x, nt.listNeuron[i].y, nt.listNeuron[i].z));
        if(d<minD){
            minD = d;
            minIndex = i;
        }
    }

    if(minD<20 && minIndex != -1){
        s.x = nt.listNeuron[minIndex].x + 1;
        s.y = nt.listNeuron[minIndex].y + 1;
        s.z = nt.listNeuron[minIndex].z + 1;
    }

    LocationSimple m;
    m.x = s.x/times - x0;
    m.y = s.y/times - y0;
    m.z = s.z/times - z0;

    qDebug()<<"x: "<<m.x<<" y: "<<m.y<<" z: "<<m.z;

    paraApp2 p = paraApp2();
    const unsigned char* imageData = image->getRawData();
    Image4DSimple* imageCopy = new Image4DSimple();
    V3DLONG szNum = image->getTotalUnitNumber();
    unsigned char* pdata = new unsigned char[szNum];
    for(int i=0; i<szNum; i++){
        pdata[i] = imageData[i];
    }
    if(threshold){
        unsigned char* pdataThres = new unsigned char[szNum];
        BinaryProcess(pdata,pdataThres,image->getXDim(),image->getYDim(),image->getZDim(),5,3);
        for(int i=0; i<szNum; i++){
            pdata[i] = pdataThres[i];
        }
        delete[] pdataThres;
    }

    imageCopy->setData(pdata,image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim(),image->getDatatype());
    p.p4dImage = imageCopy;
    p.bkg_thresh = -1;
    p.landmarks.append(m);

    for(int i=1; i<app2Markers.size(); i++){
        LocationSimple mm;
        mm.x = app2Markers[i].x/times - x0;
        mm.y = app2Markers[i].y/times - y0;
        mm.z = app2Markers[i].z/times - z0;
        p.landmarks.append(mm);
    }

    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = p.p4dImage->getXDim()-1;
    p.yc1 = p.p4dImage->getYDim()-1;
    p.zc1 = p.p4dImage->getZDim()-1;
    proc_app2(p);

    NeuronTree& result = p.result;
    for(int j=0; j<result.listNeuron.size(); j++){
        result.listNeuron[j].x = (result.listNeuron[j].x + x0)*times;
        result.listNeuron[j].y = (result.listNeuron[j].y + y0)*times;
        result.listNeuron[j].z = (result.listNeuron[j].z + z0)*times;
        result.listNeuron[j].type = type;
    }

    qDebug()<<"NeuronTree size: "<<result.listNeuron.size();
    result.hashNeuron.clear();
    for(int j=0; j<result.listNeuron.size(); j++){
        result.hashNeuron.insert(result.listNeuron[j].n,j);
    }
    QList<NeuronSWC> sortListNeuron;
    sortSWC(result.listNeuron,sortListNeuron);
    result.listNeuron.clear();
    result.listNeuron = sortListNeuron;
    result.hashNeuron.clear();
    for(int j=0; j<result.listNeuron.size(); j++){
        result.hashNeuron.insert(result.listNeuron[j].n,j);
    }

    deleteSameBranch(result,nt,5);

    sortListNeuron.clear();
    sortSWC(result.listNeuron,sortListNeuron);
    result.listNeuron.clear();
    result.listNeuron = sortListNeuron;
    result.hashNeuron.clear();
    for(int j=0; j<result.listNeuron.size(); j++){
        result.hashNeuron.insert(result.listNeuron[j].n,j);
    }

    vector<NeuronTree> app2NeuronTrees;

    NeuronTree app2NeuronTree;
    app2NeuronTree.deepCopy(result);
    app2NeuronTrees.push_back(nt);
    app2NeuronTrees.push_back(app2NeuronTree);

    NeuronTree merge = mergeNeuronTrees(app2NeuronTrees);

    qDebug()<<"merge end...";


    callback.setSWCTeraFly(merge);

    qDebug()<<"set end...";

    nt = callback.getSWCTeraFly();
    qDebug()<<"NeuronTree size: "<<nt.listNeuron.size();
    if(imageCopy)
        delete imageCopy;

}
