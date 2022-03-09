#include "retracefunction.h"

#include "app2.h"
#include "branchtree.h"
#include "../../../../../v3d_external/v3d_main/neuron_editing/neuron_sim_scores.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
#include "marker_radius.h"
#include "basic_memory.cpp"
#include "swc_convert.h"
#include "QMessageBox"
#include "judgebranch.h"

void changeContrast(unsigned char* input, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, double times){
    V3DLONG i, j, k;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                unsigned char tmp = (input[curpos]*times>255) ? 255 : input[curpos]*times;
                input[curpos] = tmp;
            }
        }
    }

}

void BinaryProcess(unsigned char* &apsInput, V3DLONG* in_sz){
    V3DLONG tolSZ = in_sz[0]*in_sz[1]*in_sz[2];
    unsigned char* bdata = new unsigned char[tolSZ];
    BinaryProcess(apsInput,bdata,in_sz[0],in_sz[1],in_sz[2],3,5);
    for(int i=0; i<tolSZ; i++){
        apsInput[i] = bdata[i];
    }
    if(bdata){
        delete[] bdata;
        bdata = 0;
    }
}

bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result){
    return SortSWC(neurons,result,VOID_VALUE,1);
}

bool sortSWC(NeuronTree& nt){
    QList<NeuronSWC> sortListNeuron = QList<NeuronSWC>();
    if(!nt.listNeuron.isEmpty()){
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
        sortSWC(nt.listNeuron,sortListNeuron);
        nt.listNeuron.clear();
        nt.listNeuron = sortListNeuron;
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
    }
    return true;
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

XYZ getLineDirection(const vector<NeuronSWC> &points){
    if(points.size() == 0 || points.size() == 1){
        return XYZ(-1,-1,-1);
    }

    XYZ p0(points[0].x,points[0].y,points[0].z);
    int count = 0;
    float x = 0, y = 0, z = 0;
    for(int i=1; i<points.size(); i++){
        x += points[i].x;
        y += points[i].y;
        z += points[i].z;
        count++;

        if(count>=5){
            break;
        }
    }
    XYZ p1(x/count,y/count,z/count);
    return p1-p0;
}

void deleteSameBranch(NeuronTree& target, const NeuronTree& ori){
    BranchTree tb = BranchTree();
    tb.initialize(target);
    qDebug()<<"---------get level0Index------";
    vector<int> level0Index = vector<int>();
    tb.get_level_index(level0Index,0);


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
    qDebug()<<"level0Index size: "<<level0Index.size();

    vector<NeuronSWC> oriLine;
    for(int i=0; i<ori.listNeuron.size(); i++){
        oriLine.push_back(ori.listNeuron[i]);
    }

    XYZ p = getLineDirection(oriLine);
    if(p == XYZ(-1,-1,-1))
        return;

    vector<bool> branchDFlag = vector<bool>(tb.branchs.size(),false);
    double d[2];
    double dire[2];

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
            if(j >= 5)
                break;
        }
        d[i] /= count;


        qDebug()<<"count: "<<count<<" d: "<<d[i];


        vector<NeuronSWC> points;
        for(int j=0; j<pointsIndex.size(); j++){
            points.push_back(target.listNeuron[pointsIndex[j]]);
        }
        XYZ p1 = getLineDirection(points);
        if(p1 == XYZ(-1,-1,-1)){
            dire[i] = -1;
        }else {
            dire[i] = dot(p,p1)/(norm(p)*norm(p1));
        }
        qDebug()<<"count: "<<count<<" dire: "<<dire[i];


//        if(d<thres){
//            branchDFlag[level0Index[i]] = true;
//        }
    }
//    int si = d[0]>d[1]?1:0;
    int si = dire[0]>dire[1] ? 0 : 1;
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

void deleteSameBranch(NeuronTree &target, const NeuronTree &ori, double dis){
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
        for(int j=1; j<pointsIndex.size(); j++){
            const NeuronSWC& p = target.listNeuron[pointsIndex[j]];
            double pToSwc = dist_pt_to_swc(XYZ(p.x,p.y,p.z),&ori);
            d += pToSwc;
            count++;
//            if(j >= 5)
//                break;
        }
        d /= count;

        if(d<dis){
            branchDFlag[level0Index[i]] = true;
        }
        qDebug()<<"count: "<<count<<" d: "<<d;

    }

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



        deleteSameBranch(result,nt);
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

void app2Terafly(int type, bool threshold, int app2Th, double contrastT, V3DPluginCallback2 &callback, QWidget *parent){



    const Image4DSimple* image = callback.getImageTeraFly();
    qDebug()<<"x: "<<image->getXDim()<<" y: "<<image->getYDim()<<" z: "<<image->getZDim();

    QString fileName = image->getFileName();
    qDebug()<<"file name: "<<fileName;
    QStringList ls = fileName.split(',');
    qDebug()<<ls[2].split('[')[1]<<" "<<ls[3].split(']')[0]<<" "<<ls[4].split('[')[1]<<" "<<ls[5].split(']')[0]<<" "<<ls[6].split('[')[1]<<" "<<ls[7].split(']')[0];

//    V3dR_MainWindow cur = callback.find3DViewerByName(fileName);



    int dimx = ls[1].split('x')[0].split('(')[1].toInt();
    int dimy = ls[1].split('x')[1].toInt();
    int dimz = ls[1].split('x')[2].split(')')[0].toInt();
    qDebug()<<"dimx: "<<dimx<<" dimy: "<<dimy<<" dimz: "<<dimz;

    int x0 = ls[2].split('[')[1].toInt() - 1;
    int x1 = ls[3].split(']')[0].toInt() - 1;
    int y0 = ls[4].split('[')[1].toInt() - 1;
    int y1 = ls[5].split(']')[0].toInt() - 1;
    int z0 = ls[6].split('[')[1].toInt() - 1;
    int z1 = ls[7].split(']')[0].toInt() - 1;

    LandmarkList app2Markers;
    LandmarkList markers = callback.getLandmarkTeraFly();
    for(int i=0; i<markers.size(); i++){
        qDebug()<<"i "<<i<<" x: "<<markers[i].x<<" y: "<<markers[i].y<<" z: "<<markers[i].z;

        if(markers[i].color.r == 255 && markers[i].color.g == 255 && markers[i].color.b == 255){
            app2Markers.append(markers[i]);
        }
    }

    if(app2Markers.isEmpty()){
        QMessageBox::information(parent,"info",QString("Please add white marker first!"));
        return;
    }




    QString path =callback.getPathTeraFly();
    qDebug()<<"path: "<<path.toStdString().c_str();

    V3DLONG* sz = 0;
    callback.getDimTeraFly(path.toStdString(),sz);
    for(int i=0; i<3; i++){
        qDebug()<<i<<": "<<sz[i];
    }

    double timesX = sz[0]/(double) dimx;
    double timesY = sz[1]/(double) dimy;
    double timesZ = sz[2]/(double) dimz;
    NeuronTree nt = callback.getSWCTeraFly();
    if(!nt.listNeuron.isEmpty()){
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
    }

    const unsigned char* imageData = image->getRawData();
    V3DLONG szNum = image->getTotalUnitNumber();

    double mean = 0, std = 0;
    for(int i=0; i<szNum; i++){
        mean += imageData[i];
    }
    if(szNum>0)
        mean /= szNum;
    for(int i=0; i<szNum; i++){
        std += (imageData[i] - mean)*(imageData[i] - mean);
    }
    if(szNum>0){
        std = sqrt(std/szNum);
    }
    int th = app2Th;
    if(th<0){
        th = mean + std;
    }



    double minD = INT_MAX;
    int minIndex = -1;
    LocationSimple& s = app2Markers[0];
    if(!nt.listNeuron.isEmpty()){
        for(int i=0; i<nt.listNeuron.size(); i++){
            XYZ tmp = XYZ(nt.listNeuron[i].x, nt.listNeuron[i].y, nt.listNeuron[i].z);
            int z = (int)(tmp.z/timesZ - z0 + 0.5);
            int y = (int)(tmp.y/timesY - y0 + 0.5);
            int x = (int)(tmp.x/timesX - x0 + 0.5);
            if(z > image->getZDim() -1) z = image->getZDim() -1;
            if(z < 0) z = 0;
            if(y > image->getYDim() -1) y = image->getYDim() -1;
            if(y < 0) y = 0;
            if(x > image->getXDim() -1) x = image->getXDim() -1;
            if(x < 0) x = 0;

            int index = z * image->getXDim()*image->getYDim()
                    + y * image->getXDim() + x;
            if(imageData[index] < th)
                continue;
            double d = dist_L2(XYZ(s.x-1, s.y-1 , s.z-1),tmp);
            if(d<minD){
                minD = d;
                minIndex = i;
            }
        }
    }




    NeuronTree nt1;
    bool isDelete = false;


    if(minD<20 && minIndex != -1){
        s.x = nt.listNeuron[minIndex].x + 1;
        s.y = nt.listNeuron[minIndex].y + 1;
        s.z = nt.listNeuron[minIndex].z + 1;
        isDelete = true;
        NeuronSWC tmp = nt.listNeuron[minIndex];
        int count = 0;
        while(tmp.parent != -1){
            nt1.listNeuron.append(tmp);
            int prtIndex = nt.hashNeuron.value(tmp.parent);
            tmp = nt.listNeuron[prtIndex];
            count++;
            if(count>10)
                break;
        }
    }
    qDebug()<<"nt1 size:"<<nt1.listNeuron.size();

    LocationSimple m;
    m.x = s.x/timesX - x0;
    m.y = s.y/timesY - y0;
    m.z = s.z/timesZ - z0;

    qDebug()<<"x: "<<m.x<<" y: "<<m.y<<" z: "<<m.z;

    paraApp2 p = paraApp2();

    Image4DSimple* imageCopy = new Image4DSimple();

    unsigned char* pdata = new unsigned char[szNum];
    for(int i=0; i<szNum; i++){
        pdata[i] = imageData[i];
    }
    if(contrastT>0){
        changeContrast(pdata,image->getXDim(),image->getYDim(),image->getZDim(),contrastT);
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
    p.bkg_thresh = app2Th;
    p.landmarks.append(m);

    for(int i=1; i<app2Markers.size(); i++){
        LocationSimple mm;
        mm.x = app2Markers[i].x/timesX - x0;
        mm.y = app2Markers[i].y/timesY - y0;
        mm.z = app2Markers[i].z/timesZ - z0;
        p.landmarks.append(mm);
    }


    v3dhandle curwin = callback.currentImageWindow();
    View3DControl * view3d = callback.getView3DControl(curwin);
    V3DLONG in_sz0[4] = {p.p4dImage->getXDim(),p.p4dImage->getYDim(),p.p4dImage->getZDim(),p.p4dImage->getCDim()};
    if(!view3d){
        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;
        qDebug()<<"no view 3d";
    }else{
        qDebug()<<" xc0: "<<view3d->xCut0()<<" xc1: "<<view3d->xCut1()
               <<" yc0: "<<view3d->yCut0()<<" yc1: "<<view3d->yCut1()
              <<" zc0: "<<view3d->zCut0()<<" zc1: "<<view3d->zCut1();
        V3DLONG view3d_datasz0 = view3d->dataDim1();
        V3DLONG view3d_datasz1 = view3d->dataDim2();
        V3DLONG view3d_datasz2 = view3d->dataDim3();

        qDebug()<<"view3d_datasz0： "<<view3d_datasz0<<" view3d_datasz1： "<<view3d_datasz1<<" view3d_datasz2： "<<view3d_datasz2;

        p.xc0 = int(double(view3d->xCut0()) * in_sz0[0] / view3d_datasz0 + 0.5);
        p.xc1 = int(double(view3d->xCut1()) * in_sz0[0] / view3d_datasz0 + 0.5);
        if (p.xc1>in_sz0[0]-1) p.xc1 = in_sz0[0]-1;
        //xc1 = in_sz0[0]-1;//for debug purpose. 130102

        p.yc0 = int(double(view3d->yCut0()) * in_sz0[1] / view3d_datasz1 + 0.5);
        p.yc1 = int(double(view3d->yCut1()) * in_sz0[1] / view3d_datasz1 + 0.5);
        if (p.yc1>in_sz0[1]-1) p.yc1 = in_sz0[1]-1;

        p.zc0 = int(double(view3d->zCut0()) * in_sz0[2] / view3d_datasz2 + 0.5);
        p.zc1 = int(double(view3d->zCut1()) * in_sz0[2] / view3d_datasz2 + 0.5);
        if (p.zc1>in_sz0[2]-1) p.zc1 = in_sz0[2]-1;

    }

    NeuronTree maskTree = NeuronTree();
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC tmp = nt.listNeuron[i];
        tmp.x = nt.listNeuron[i].x/timesX - x0;
        tmp.y = nt.listNeuron[i].y/timesY - y0;
        tmp.z = nt.listNeuron[i].z/timesZ - z0;
        if(tmp.x >= p.xc0 && tmp.x <= p.xc1
                && tmp.y >= p.yc0 && tmp.y <= p.yc1
                && tmp.z >= p.zc0 && tmp.z <= p.zc1){
            maskTree.listNeuron.push_back(tmp);
        }
    }
    unsigned char* mask = 0;
    vector<MyMarker*> maskMarkers = swc_convert(maskTree);
    V3DLONG imageSZ[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};
    double radiusThres = th > 40 ? th : 40;
    for(int i=0 ; i<maskMarkers.size(); i++){
        maskMarkers[i]->radius = markerRadius(imageData,imageSZ,maskMarkers[i]->x,maskMarkers[i]->y,maskMarkers[i]->z,radiusThres);
    }
    swc2mask(mask,maskMarkers,imageSZ[0],imageSZ[1],imageSZ[2]);
    for(int i=0; i<szNum; i++){
        int x = i%imageSZ[0];
        int y = (i/imageSZ[0])%imageSZ[1];
        int z = i/(imageSZ[0]*imageSZ[1]);
        if(mask[i] == 255 && (x>m.x+2 || x<m.x-3)
                && (y>m.y+2 || y<m.y-3)
                && (z>m.y+2 || z<m.z-3)){
            pdata[i] = mean < 10 ? mean : 10;
//            pdata[i] = 0;
        }
    }
    simple_saveimage_wrapper(callback,"D:\\mask.v3draw",pdata,imageSZ,1);


    for(int i=0; i<p.landmarks.size(); i++){
        const LocationSimple& mm = p.landmarks.at(i);
        if(mm.x<p.xc0 || mm.x>p.xc1 || mm.y<p.yc0 || mm.y>p.yc1 || mm.z<p.zc0 || mm.z>p.zc1){
            QMessageBox::information(parent,"info",QString("Please add white marker in the box, not in anywhere!"));
            return;
        }
    }

    proc_app2(p);

    NeuronTree& result = p.result;
    for(int j=0; j<result.listNeuron.size(); j++){
        result.listNeuron[j].x = (result.listNeuron[j].x + x0)*timesX;
        result.listNeuron[j].y = (result.listNeuron[j].y + y0)*timesY;
        result.listNeuron[j].z = (result.listNeuron[j].z + z0)*timesZ;
        result.listNeuron[j].type = type;
    }

    qDebug()<<"NeuronTree size: "<<result.listNeuron.size();
    if(result.listNeuron.isEmpty()){
        QMessageBox::information(parent,"info",QString("tracing is ended! tracing listNeuron size is 0"));
        return;
    }
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

    if(isDelete){
        deleteSameBranch(result,nt1);
    }

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

    int lastSize = app2NeuronTree.listNeuron.size();

    if(minD<30 && minIndex != -1){
        for(int j=0; j<app2NeuronTree.listNeuron.size(); j++){
            if(app2NeuronTree.listNeuron[j].parent == -1){
                app2NeuronTree.listNeuron[j].x = nt.listNeuron[minIndex].x;
                app2NeuronTree.listNeuron[j].y = nt.listNeuron[minIndex].y;
                app2NeuronTree.listNeuron[j].z = nt.listNeuron[minIndex].z;
            }
        }
    }

    app2NeuronTrees.push_back(nt);
    app2NeuronTrees.push_back(app2NeuronTree);

    NeuronTree merge = mergeNeuronTrees(app2NeuronTrees);

    qDebug()<<"merge end...";


    callback.setSWCTeraFly(merge);

    qDebug()<<"set end...";

//    nt = callback.getSWCTeraFly();
//    qDebug()<<"NeuronTree size: "<<nt.listNeuron.size();
    if(imageCopy)
        delete imageCopy;

    QMessageBox::information(parent,"info",QString("tracing is ended! tracing listNeuron size is ")+QString::number(lastSize));

}

void app2MultiTerafly(int type, bool threshold, int app2Th, double contrastT, V3DPluginCallback2& callback, QWidget *parent){
    const Image4DSimple* image = callback.getImageTeraFly();
    qDebug()<<"x: "<<image->getXDim()<<" y: "<<image->getYDim()<<" z: "<<image->getZDim();

    QString fileName = image->getFileName();
    qDebug()<<"file name: "<<fileName;
    QStringList ls = fileName.split(',');
    qDebug()<<ls[2].split('[')[1]<<" "<<ls[3].split(']')[0]<<" "<<ls[4].split('[')[1]<<" "<<ls[5].split(']')[0]<<" "<<ls[6].split('[')[1]<<" "<<ls[7].split(']')[0];

//    V3dR_MainWindow cur = callback.find3DViewerByName(fileName);



    int dimx = ls[1].split('x')[0].split('(')[1].toInt();
    int dimy = ls[1].split('x')[1].toInt();
    int dimz = ls[1].split('x')[2].split(')')[0].toInt();
    qDebug()<<"dimx: "<<dimx<<" dimy: "<<dimy<<" dimz: "<<dimz;

    int x0 = ls[2].split('[')[1].toInt() - 1;
    int x1 = ls[3].split(']')[0].toInt() - 1;
    int y0 = ls[4].split('[')[1].toInt() - 1;
    int y1 = ls[5].split(']')[0].toInt() - 1;
    int z0 = ls[6].split('[')[1].toInt() - 1;
    int z1 = ls[7].split(']')[0].toInt() - 1;

    LandmarkList app2Markers;
    LandmarkList markers = callback.getLandmarkTeraFly();
    for(int i=0; i<markers.size(); i++){
        qDebug()<<"i "<<i<<" x: "<<markers[i].x<<" y: "<<markers[i].y<<" z: "<<markers[i].z;

        if(markers[i].color.r == 255 && markers[i].color.g == 255 && markers[i].color.b == 255){
            app2Markers.append(markers[i]);
        }
    }

    if(app2Markers.isEmpty()){
        QMessageBox::information(parent,"info",QString("Please add white marker first!"));
        return;
    }




    QString path =callback.getPathTeraFly();
    qDebug()<<"path: "<<path.toStdString().c_str();

    V3DLONG* sz = 0;
    callback.getDimTeraFly(path.toStdString(),sz);
    for(int i=0; i<3; i++){
        qDebug()<<i<<": "<<sz[i];
    }

    double timesX = sz[0]/(double) dimx;
    double timesY = sz[1]/(double) dimy;
    double timesZ = sz[2]/(double) dimz;
    NeuronTree nt = callback.getSWCTeraFly();
    if(!nt.listNeuron.isEmpty()){
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
    }

    const unsigned char* imageData = image->getRawData();
    Image4DSimple* imageCopy = new Image4DSimple();
    V3DLONG szNum = image->getTotalUnitNumber();

    int th = app2Th;
    if(th<0){
        double mean = 0, std = 0;
        for(int i=0; i<szNum; i++){
            mean += imageData[i];
        }
        if(szNum>0)
            mean /= szNum;
        for(int i=0; i<szNum; i++){
            std += (imageData[i] - mean)*(imageData[i] - mean);
        }
        if(szNum>0){
            std = sqrt(std/szNum);
        }
        th = mean + std;
    }

    unsigned char* pdata = new unsigned char[szNum];
    for(int i=0; i<szNum; i++){
        pdata[i] = imageData[i];
    }

    if(contrastT>0){
        changeContrast(pdata,image->getXDim(),image->getYDim(),image->getZDim(),contrastT);
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
    paraApp2 p = paraApp2();
    p.p4dImage = imageCopy;
    p.bkg_thresh = app2Th;

    v3dhandle curwin = callback.currentImageWindow();
    View3DControl * view3d = callback.getView3DControl(curwin);
    V3DLONG in_sz0[4] = {p.p4dImage->getXDim(),p.p4dImage->getYDim(),p.p4dImage->getZDim(),p.p4dImage->getCDim()};
    if(!view3d){
        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;
        qDebug()<<"no view 3d";
    }else{
        qDebug()<<" xc0: "<<view3d->xCut0()<<" xc1: "<<view3d->xCut1()
               <<" yc0: "<<view3d->yCut0()<<" yc1: "<<view3d->yCut1()
              <<" zc0: "<<view3d->zCut0()<<" zc1: "<<view3d->zCut1();
        V3DLONG view3d_datasz0 = view3d->dataDim1();
        V3DLONG view3d_datasz1 = view3d->dataDim2();
        V3DLONG view3d_datasz2 = view3d->dataDim3();

        qDebug()<<"view3d_datasz0: "<<view3d_datasz0<<" view3d_datasz1: "<<view3d_datasz1<<" view3d_datasz2: "<<view3d_datasz2;

        p.xc0 = int(double(view3d->xCut0()) * in_sz0[0] / view3d_datasz0 + 0.5);
        p.xc1 = int(double(view3d->xCut1()) * in_sz0[0] / view3d_datasz0 + 0.5);
        if (p.xc1>in_sz0[0]-1) p.xc1 = in_sz0[0]-1;
        //xc1 = in_sz0[0]-1;//for debug purpose. 130102

        p.yc0 = int(double(view3d->yCut0()) * in_sz0[1] / view3d_datasz1 + 0.5);
        p.yc1 = int(double(view3d->yCut1()) * in_sz0[1] / view3d_datasz1 + 0.5);
        if (p.yc1>in_sz0[1]-1) p.yc1 = in_sz0[1]-1;

        p.zc0 = int(double(view3d->zCut0()) * in_sz0[2] / view3d_datasz2 + 0.5);
        p.zc1 = int(double(view3d->zCut1()) * in_sz0[2] / view3d_datasz2 + 0.5);
        if (p.zc1>in_sz0[2]-1) p.zc1 = in_sz0[2]-1;

    }


    vector<NeuronTree> app2NeuronTrees;
    app2NeuronTrees.push_back(nt);
    NeuronTree nt1 = NeuronTree();

    double minD = INT_MAX;
    int minIndex = -1;

    for(int i=0; i<app2Markers.size(); i++){
        minD = INT_MAX;
        minIndex = -1;
        LocationSimple& s = app2Markers[i];
        if(!nt.listNeuron.isEmpty()){
            for(int j=0; j<nt.listNeuron.size(); j++){
                XYZ tmp = XYZ(nt.listNeuron[j].x, nt.listNeuron[j].y, nt.listNeuron[j].z);
                int z = (int)(tmp.z/timesZ - z0 + 0.5);
                int y = (int)(tmp.y/timesY - y0 + 0.5);
                int x = (int)(tmp.x/timesX - x0 + 0.5);
                if(z > image->getZDim() -1) z = image->getZDim() -1;
                if(z < 0) z = 0;
                if(y > image->getYDim() -1) y = image->getYDim() -1;
                if(y < 0) y = 0;
                if(x > image->getXDim() -1) x = image->getXDim() -1;
                if(x < 0) x = 0;

                int index = z * image->getXDim()*image->getYDim()
                        + y * image->getXDim() + x;
                if(imageData[index] < th)
                    continue;
                double d = dist_L2(XYZ(s.x-1, s.y-1 , s.z-1),tmp);
                if(d<minD){
                    minD = d;
                    minIndex = j;
                }
            }
        }


        bool isDelete = false;
        nt1.listNeuron.clear();


        if(minD<30 && minIndex != -1){
            s.x = nt.listNeuron[minIndex].x + 1;
            s.y = nt.listNeuron[minIndex].y + 1;
            s.z = nt.listNeuron[minIndex].z + 1;
            isDelete = true;
            NeuronSWC tmp = nt.listNeuron[minIndex];
            int count = 0;
            while(tmp.parent != -1){
                nt1.listNeuron.append(tmp);
                int prtIndex = nt.hashNeuron.value(tmp.parent);
                tmp = nt.listNeuron[prtIndex];
                count++;
                if(count>10)
                    break;
            }
        }
        qDebug()<<"i: "<<i<<" nt1 size"<<nt1.listNeuron.size();

        LocationSimple m;
        m.x = s.x/timesX - x0;
        m.y = s.y/timesY - y0;
        m.z = s.z/timesZ - z0;

        qDebug()<<"x: "<<m.x<<" y: "<<m.y<<" z: "<<m.z;

        if(m.x<p.xc0 || m.x>p.xc1 || m.y<p.yc0 || m.y>p.yc1 || m.z<p.zc0 || m.z>p.zc1){
            continue;
        }

        if(!p.landmarks.isEmpty()){
            p.landmarks.clear();
        }

        p.landmarks.append(m);

        proc_app2(p);

        NeuronTree& result = p.result;
        for(int j=0; j<result.listNeuron.size(); j++){
            result.listNeuron[j].x = (result.listNeuron[j].x + x0)*timesX;
            result.listNeuron[j].y = (result.listNeuron[j].y + y0)*timesY;
            result.listNeuron[j].z = (result.listNeuron[j].z + z0)*timesZ;
            result.listNeuron[j].type = type;
        }

        qDebug()<<"i: "<<i<<" NeuronTree size "<<result.listNeuron.size();
        if(result.listNeuron.isEmpty()){
            continue;
        }
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

        if(isDelete){
            deleteSameBranch(result,nt1);
        }

        sortListNeuron.clear();
        sortSWC(result.listNeuron,sortListNeuron);
        result.listNeuron.clear();
        result.listNeuron = sortListNeuron;
        result.hashNeuron.clear();
        for(int j=0; j<result.listNeuron.size(); j++){
            result.hashNeuron.insert(result.listNeuron[j].n,j);
        }



        NeuronTree app2NeuronTree;
        app2NeuronTree.deepCopy(result);

        if(minD<30 && minIndex != -1){
            for(int j=0; j<app2NeuronTree.listNeuron.size(); j++){
                if(app2NeuronTree.listNeuron[j].parent == -1){
                    app2NeuronTree.listNeuron[j].x = nt.listNeuron[minIndex].x;
                    app2NeuronTree.listNeuron[j].y = nt.listNeuron[minIndex].y;
                    app2NeuronTree.listNeuron[j].z = nt.listNeuron[minIndex].z;
                }
            }
        }
        app2NeuronTrees.push_back(app2NeuronTree);
    }

    if(app2NeuronTrees.size() == 1){
        QMessageBox::information(parent,"info",QString("tracing is ended! tracing listNeuron size is 0"));
        return;
    }

    int lastSize = 0;

    for(int i=1; i<app2NeuronTrees.size(); i++){
        lastSize += app2NeuronTrees[i].listNeuron.size();
    }

    NeuronTree merge = mergeNeuronTrees(app2NeuronTrees);

    qDebug()<<"merge end...";


    callback.setSWCTeraFly(merge);

    qDebug()<<"set end...";

//    nt = callback.getSWCTeraFly();
//    qDebug()<<"NeuronTree size: "<<nt.listNeuron.size();
    if(imageCopy)
        delete imageCopy;
    QMessageBox::information(parent,"info",QString("tracing is ended! tracing listNeuron size is ")+QString::number(lastSize));
}



void normalImage(unsigned char* pdata, V3DLONG* sz){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double mean = 0;
    for(int i=0; i<tolSZ; i++){
        mean += pdata[i];
    }

    if(tolSZ>0)
        mean /= tolSZ;

    int max = 80;
    int min = INT_MAX;
    if(mean>0.5*max){
        max = mean*2;
    }

    for(int i=0; i<tolSZ; i++){
        if(pdata[i]<min)
            min = pdata[i];
        if(pdata[i]>max)
            pdata[i] = max;
    }

    for(int i=0; i<tolSZ; i++){
        int tmp = (unsigned char)(((pdata[i] - min)/(double) (max-min)) * 255 + 0.5);
        if(tmp>255) tmp = 255;
        if(tmp<0) tmp = 0;
        pdata[i] = tmp;
    }
}

int* getThresholdByKmeans(unsigned char* pdata, V3DLONG* sz, int k){

    if(k<2){
        qDebug()<<"please select number of 2 up as k";
    }

    V3DLONG totalSZ = sz[0]*sz[1]*sz[2];

    double t = 255/(double) (k+1);

    vector<vector<unsigned char> > groups = vector<vector<unsigned char> >(k,vector<unsigned char>());
    vector<double> groupCenter = vector<double>();
    for(int i=1; i<=k; i++){
        groupCenter.push_back(i*t);
    }
    int iteration = 0;
    while (iteration < 30) {
        for(int i=0; i<totalSZ; i++){
            double minDistance = INT_MAX;
            int minIndex = -1;
            for(int j=0; j<k; j++){
                double d = abs(groupCenter[j]-pdata[i]);
                if(d<minDistance){
                    minDistance = d;
                    minIndex = j;
                }
            }
            if(minIndex != -1){
                groups[minIndex].push_back(pdata[i]);
            }
        }

        vector<double> groupNewCenter = vector<double>();

        for(int i=0; i<k; i++){
            double mean = 0;
            int gs = groups[i].size();
            for(int j=0; j<gs; j++){
                mean += groups[i][j];
            }
            if(gs>0){
                mean /= gs;
            }
            groupNewCenter.push_back(mean);
        }

        double J = 0;

        for(int i=0; i<k; i++){
            J += (groupCenter[i] - groupNewCenter[i])*(groupCenter[i] - groupNewCenter[i]);
        }

        J = sqrt(J/k);
        if(J<2/(double)k){
            break;
        }else {
            for(int i=0; i<k; i++){
                groupCenter[i] = groupNewCenter[i];
            }
        }

        iteration++;
    }

    qDebug()<<"iteration: "<<iteration;

    for(int i=0; i<k; i++){
        qDebug()<<"i size: "<<groups[i].size();
    }

    int* result = new int[k];

    for(int i=0; i<k; i++){
        int tmp = INT_MAX;
        for(int j=0; j<groups[i].size(); j++){
            if(groups[i][j] < tmp)
                tmp = groups[i][j];
        }
        result[i] = tmp;
    }


    return result;

}

int* getThresholdByKmeans(Image4DSimple *image, int k){
    unsigned char* pdata = image->getRawData();
    V3DLONG sz[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};
    return getThresholdByKmeans(pdata,sz,k);
}

int getPercentTh(unsigned char* pdata, V3DLONG* sz, double ratio){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    vector<int> intensityNum = vector<int>(256,0);
    for(int i=0; i<tolSZ; i++){
        intensityNum[pdata[i]]++;
    }
    int count = 0;
    for(int i=255; i>=0; i--){
       count += intensityNum[i];
       if(count/(double)tolSZ>ratio){
           return i;
       }
    }
}

QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction)
{
    qDebug()<<"------------in group tips---------------";

    QList<LandmarkList> groupTips;

   //bubble sort
   if(direction == 1 || direction == 2 || direction == 5 || direction == 6)
   {
       sort(tips.begin(), tips.end(), [](LocationSimple a, LocationSimple b){return a.y > b.y;});

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).y - tips.at(d).y < block_size)
           {
               eachGroupList.push_back(tips.at(d+1));
           }
           else
           {
               groupTips.push_back(eachGroupList);
               eachGroupList.erase(eachGroupList.begin(),eachGroupList.end());
               eachGroupList.push_back(tips.at(d+1));
           }
       }
       groupTips.push_back(eachGroupList);
   }else
   {
       sort(tips.begin(), tips.end(), [](LocationSimple a, LocationSimple b){return a.x > b.x;});

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).x - tips.at(d).x < block_size)
           {
               eachGroupList.push_back(tips.at(d+1));
           }
           else
           {
               groupTips.push_back(eachGroupList);
               eachGroupList.erase(eachGroupList.begin(),eachGroupList.end());
               eachGroupList.push_back(tips.at(d+1));

           }
       }
       groupTips.push_back(eachGroupList);
   }
   return groupTips;
}

void getImageBlockByTip(LocationSimple tip, vector<imageBlock> &blockList, int block_size, int direction, BoundingBox box){
    int lap = 10;

    imageBlock b = imageBlock();
    b.direction = direction;
    b.startMarkers.push_back(tip);

    if(direction == 1)
    {
        b.end_x = tip.x + lap;
        b.start_x = b.end_x - block_size;
        b.start_y = tip.y - block_size/2;
        b.end_y = b.start_y + block_size;
        b.start_z = tip.z - block_size/2;
        b.end_z = b.start_z + block_size;
    }else if(direction == 2)
    {
        b.start_x = tip.x - lap;
        b.end_x = b.start_x + block_size;
        b.start_y = tip.y - block_size/2;
        b.end_y = b.start_y + block_size;
        b.start_z = tip.z - block_size/2;
        b.end_z = b.start_z + block_size;
    }else if(direction == 3)
    {
        b.start_x = tip.x - block_size/2;
        b.end_x = b.start_x + block_size;
        b.end_y = tip.y + lap;
        b.start_y = b.end_y - block_size;
        b.start_z = tip.z - block_size/2;
        b.end_z = b.start_z + block_size;
    }else if(direction == 4)
    {
        b.start_x = tip.x - block_size/2;
        b.end_x = b.start_x + block_size;
        b.start_y = tip.y - lap;
        b.end_y = b.start_y + block_size;
        b.start_z = tip.z - block_size/2;
        b.end_z = b.start_z + block_size;
    }else if(direction == 5)
    {
        b.start_x = tip.x - block_size/2;
        b.end_x = b.start_x + block_size;
        b.start_y = tip.y - block_size/2;
        b.end_y = b.start_y + block_size;
        b.end_z = tip.z + lap;
        b.start_z = b.end_z + block_size;
    }else if(direction == 6)
    {
        b.start_x = tip.x - block_size/2;
        b.end_x = b.start_x + block_size;
        b.start_y = tip.y - block_size/2;
        b.end_y = b.start_y + block_size;
        b.end_z = tip.z + lap;
        b.start_z = b.end_z + block_size;
    }


    qDebug()<<"before block: "<<b.start_x<<" "<<b.end_x<<" "<<b.start_y<<" "<<b.end_y<<" "<<b.start_z<<" "<<b.end_z;

    if(b.start_x<box.x0) b.start_x = box.x0;
    if(b.end_x>box.x1)   b.end_x = box.x1;
    if(b.start_y<box.y0) b.start_y = box.y0;
    if(b.end_y>box.y1)   b.end_y = box.y1;
    if(b.start_z<box.z0) b.start_z = box.z0;
    if(b.end_z>box.z1)   b.end_z = box.z1;

    qDebug()<<"block: "<<b.start_x<<" "<<b.end_x<<" "<<b.start_y<<" "<<b.end_y<<" "<<b.start_z<<" "<<b.end_z;
    if(b.end_x - b.start_x > 20 && b.end_y - b.start_y > 20 && b.end_z - b.start_z > 20){
        blockList.push_back(b);
    }

}

int getDirection(XYZ p){
    int direction = 0;
    float maxXYZ = ABSMAX(p.x,ABSMAX(p.y,p.z));
    if(maxXYZ == p.x){
        if(p.x>0){
            direction = 1;
        }else {
            direction = 2;
        }
    }else if (maxXYZ == p.y) {
        if(p.y>0){
            direction = 3;
        }else {
            direction = 4;
        }
    }else {
        if(p.z>0){
            direction = 5;
        }else {
            direction = 6;
        }
    }
    qDebug()<<"direction: "<<direction;
    return direction;
}

void getImageBlockByTips(LandmarkList tips, vector<imageBlock> &blockList, int block_size, int direction, BoundingBox box){

    qDebug()<<"--------------------in getImageBlockBytips-------------------------";

    int lap = 20;

    imageBlock b = imageBlock();
    b.direction = direction;
    b.startMarkers = tips;
    float min_x = INF, max_x = -INF;
    float min_y = INF, max_y = -INF;
    float min_z = INF, max_z = -INF;

    double adaptive_size_x,adaptive_size_y,adaptive_size_z;
//    double max_r = -INF;

    for(int i = 0; i<tips.size();i++)
    {
        if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
        if(tips.at(i).x >= max_x) max_x = tips.at(i).x;

        if(tips.at(i).y <= min_y) min_y = tips.at(i).y;
        if(tips.at(i).y >= max_y) max_y = tips.at(i).y;

        if(tips.at(i).z <= min_z) min_z = tips.at(i).z;
        if(tips.at(i).z >= max_z) max_z = tips.at(i).z;

//        if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
    }

    if(direction == 1 || direction == 2)
    {
        adaptive_size_y = (max_y - min_y)*1.2;
        adaptive_size_z = (max_z - min_z)*1.2;
        adaptive_size_x = adaptive_size_y;

    }else if(direction == 3 || direction == 4)
    {
        adaptive_size_x = (max_x - min_x)*1.2;
        adaptive_size_z = (max_z - min_z)*1.2;
        adaptive_size_y = adaptive_size_x;
    }else if(direction == 5 || direction == 6)
    {
        adaptive_size_x = (max_x - min_x)*1.2;
        adaptive_size_y = (max_y - min_y)*1.2;
        adaptive_size_z = adaptive_size_x;
    }

    adaptive_size_x = (adaptive_size_x <= 128) ? 128 : adaptive_size_x;
    adaptive_size_y = (adaptive_size_y <= 128) ? 128 : adaptive_size_y;
    adaptive_size_z = (adaptive_size_z <= 128) ? 128 : adaptive_size_z;


    adaptive_size_x = (adaptive_size_x >= block_size) ? block_size : adaptive_size_x;
    adaptive_size_y = (adaptive_size_y >= block_size) ? block_size : adaptive_size_y;
    adaptive_size_z = (adaptive_size_z >= block_size) ? block_size : adaptive_size_z;

    if(direction == 1)
    {
        b.end_x = floor(max_x + lap);
        b.start_x = b.end_x - adaptive_size_x;
        b.start_y = floor((min_y + max_y - adaptive_size_y)/2);
        b.end_y = b.start_y + adaptive_size_y;
        b.start_z = floor((min_z + max_z - adaptive_size_z)/2);
        b.end_z = b.start_z + adaptive_size_z;
    }else if(direction == 2)
    {
        b.start_x = floor(min_x - lap);
        b.end_x = b.start_x + adaptive_size_x;
        b.start_y = floor((min_y + max_y - adaptive_size_y)/2);
        b.end_y = b.start_y + adaptive_size_y;
        b.start_z = floor((min_z + max_z - adaptive_size_z)/2);
        b.end_z = b.start_z + adaptive_size_z;

    }else if(direction == 3)
    {
        b.start_x = floor((min_x + max_x - adaptive_size_x)/2);
        b.end_x = b.start_x + adaptive_size_x;
        b.end_y = floor(max_y + lap);
        b.start_y = b.end_y - adaptive_size_y;
        b.start_z = floor((min_z + max_z - adaptive_size_z)/2);
        b.end_z = b.start_z + adaptive_size_z;
    }else if(direction == 4)
    {
        b.start_x = floor((min_x + max_x - adaptive_size_x)/2);
        b.end_x = b.start_x + adaptive_size_x;
        b.start_y = floor(min_y - lap);
        b.end_y = b.start_y + adaptive_size_y;
        b.start_z = floor((min_z + max_z - adaptive_size_z)/2);
        b.end_z = b.start_z + adaptive_size_z;
    }else if(direction == 5)
    {
        b.start_x = floor((min_x + max_x - adaptive_size_x)/2);
        b.end_x = b.start_x + adaptive_size_x;
        b.start_y = floor((min_y + max_y - adaptive_size_y)/2);
        b.end_y = b.start_y + adaptive_size_y;
        b.end_z = floor(max_z + lap);
        b.start_z = b.end_z - adaptive_size_z;
    }else if(direction == 6)
    {
        b.start_x = floor((min_x + max_x - adaptive_size_x)/2);
        b.end_x = b.start_x + adaptive_size_x;
        b.start_y = floor((min_y + max_y - adaptive_size_y)/2);
        b.end_y = b.start_y + adaptive_size_y;
        b.start_z = floor(min_z - lap);
        b.end_z = b.start_z + adaptive_size_z;
    }

    qDebug()<<"before block: "<<b.start_x<<" "<<b.end_x<<" "<<b.start_y<<" "<<b.end_y<<" "<<b.start_z<<" "<<b.end_z;

    if(b.start_x<box.x0) b.start_x = box.x0;
    if(b.end_x>box.x1)   b.end_x = box.x1;
    if(b.start_y<box.y0) b.start_y = box.y0;
    if(b.end_y>box.y1)   b.end_y = box.y1;
    if(b.start_z<box.z0) b.start_z = box.z0;
    if(b.end_z>box.z1)   b.end_z = box.z1;

    qDebug()<<"block: "<<b.start_x<<" "<<b.end_x<<" "<<b.start_y<<" "<<b.end_y<<" "<<b.start_z<<" "<<b.end_z;
    if(b.end_x - b.start_x > 20 && b.end_y - b.start_y > 20 && b.end_z - b.start_z > 20){
        blockList.push_back(b);
    }

}

vector<NeuronTree> imageBlock::getNeuronTrees(QString brainPath, V3DPluginCallback2& callback){

    vector<NeuronTree> app2NeuronTrees = vector<NeuronTree>();

    V3DLONG* in_sz = new V3DLONG[4];
    in_sz[0] = end_x - start_x;
    in_sz[1] = end_y - start_y;
    in_sz[2] = end_z - start_z;
    in_sz[3] = 1;
    qDebug()<<"start_x: "<<start_x<<" end_x: "<<end_x
           <<"start_y: "<<start_y<<" end_y: "<<end_y
          <<"start_z: "<<start_z<<" end_z: "<<end_z;
    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString().c_str(),start_x,end_x,start_y,end_y,start_z,end_z);
    normalImage(pdata,in_sz);
    Image4DSimple* app2Image = new Image4DSimple();
    app2Image->setData(pdata,in_sz[0],in_sz[1],in_sz[2],in_sz[3],V3D_UINT8);

    int k = 3;
//    int* th = getThresholdByKmeans(pdata,in_sz,k);
//    int app2Th = th[k-2];
    int app2Th = -1;

    paraApp2 p2 = paraApp2();
    p2.p4dImage = app2Image;
    p2.bkg_thresh = app2Th;
    p2.xc0 = 0;
    p2.yc0 = 0;
    p2.zc0 = 0;
    p2.xc1 = in_sz[0] - 1;
    p2.yc1 = in_sz[1] - 1;
    p2.zc1 = in_sz[2] - 1;

    bool isBinaryProcess = false;

    for(int i=0; i<startMarkers.size(); i++){

        qDebug()<<i<<" : proc_app2---------------";

        BoundingBox box = BoundingBox(start_x,start_y,start_z,end_x,end_y,end_z);
        if(!box.isInner(XYZ(startMarkers[i].x,startMarkers[i].y,startMarkers[i].z))){
            continue;
        }

        LocationSimple m = LocationSimple(startMarkers[i].x + 1 - start_x, startMarkers[i].y + 1 - start_y, startMarkers[i].z + 1 - start_z);
        p2.landmarks.push_back(m);
        proc_app2(p2);
        p2.landmarks.clear();
        NeuronTree app2NeuronTree = NeuronTree();
//        sortSWC(p2.result);
        app2NeuronTree.deepCopy(p2.result);

        BranchTree tb = BranchTree();
        tb.initialize(app2NeuronTree);

        if(direction == 0){
            if(tb.branchs.size()>100){
                isBinaryProcess = true;
                break;
            }
            app2NeuronTrees.push_back(app2NeuronTree);
            continue;
        }

        qDebug()<<"---------get level0Index------";
        vector<int> level0Index = vector<int>();
        tb.get_level_index(level0Index,0);

        if(level0Index.size() > 3 || tb.branchs.size()>100){
            isBinaryProcess = true;
            break;
        }else {
            XYZ p;
            if(direction == 1){
                p = XYZ(-1,0,0);
            }else if (direction == 2) {
                p = XYZ(1,0,0);
            }else if (direction == 3) {
                p = XYZ(0,-1,0);
            }else if (direction == 4) {
                p = XYZ(0,1,0);
            }else if (direction == 5) {
                p = XYZ(0,0,-1);
            }else if (direction == 6) {
                p = XYZ(0,0,1);
            }

            vector<bool> branchDFlag = vector<bool>(tb.branchs.size(),false);
            qDebug()<<"level 0 size: "<<level0Index.size();

            for(int j=0; j<level0Index.size(); j++){

                qDebug()<<"j: "<<j<<" index: "<<level0Index[j];

                vector<V3DLONG> pointsIndex = vector<V3DLONG>();
                qDebug()<<"---------------aaaaa----------------";
                tb.branchs[level0Index[j]].get_pointsIndex_of_branch(pointsIndex,app2NeuronTree);

                qDebug()<<"pointsIndex size: "<<pointsIndex.size();


                vector<NeuronSWC> points;
                for(int k=0; k<pointsIndex.size(); k++){
                    points.push_back(app2NeuronTree.listNeuron[pointsIndex[k]]);
                }
                XYZ p1 = getLineDirection(points);
                double dire;
                if(p1 == XYZ(-1,-1,-1)){
                    dire = -1;
                }else {
                    dire = dot(p,p1)/(norm(p)*norm(p1));
                }
                qDebug()<<" dire: "<<dire;

                if(dire>0){
                    branchDFlag[level0Index[j]] = true;
                }

            }

            qDebug()<<"---------------start to flag dbranch---------";

            int maxLevel = tb.get_max_level();
            qDebug()<<"maxLevel: "<<maxLevel;

            map<Branch*,int> branchMap = map<Branch*,int>();
            for(int j=0; j<tb.branchs.size(); j++){
                Branch* b = &(tb.branchs[j]);
                branchMap[b] = j;
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
            vector<bool> pointDIndex = vector<bool>(app2NeuronTree.listNeuron.size(),false);
            for(int j=0; j<branchDFlag.size(); j++){
                if(branchDFlag[j]){
                    vector<V3DLONG> pointIndex = vector<V3DLONG>();
                    pointIndex.clear();

                    tb.branchs[j].get_pointsIndex_of_branch(pointIndex,app2NeuronTree);
                    qDebug()<<"point size: "<<pointIndex.size();
                    for(int k=1; k<pointIndex.size(); k++){
                        pointDIndex[pointIndex[k]] = true;
                        pointDNum++;
                    }
                }
            }

            qDebug()<<"delete point number: "<<pointDNum;

            qDebug()<<"---------start to copy------------------";

            QList<NeuronSWC> listNeuron = QList<NeuronSWC>();
            listNeuron.clear();
            for(int j=0; j<pointDIndex.size(); j++){
                if(!pointDIndex[j]){
                    NeuronSWC p = app2NeuronTree.listNeuron[j];
                    listNeuron.append(p);
                }
            }

            app2NeuronTree.listNeuron.clear();
            app2NeuronTree.listNeuron = listNeuron;

            app2NeuronTrees.push_back(app2NeuronTree);
        }

    }

    if(isBinaryProcess){

        qDebug()<<"------------BinaryProcess image-----------------------";

        int tolSZ = in_sz[0]*in_sz[1]*in_sz[2];
        unsigned char* bdata = new unsigned char[tolSZ];
        BinaryProcess(pdata,bdata,in_sz[0],in_sz[1],in_sz[2],3,5);
        for(int i=0; i<tolSZ; i++){
            pdata[i] = bdata[i];
        }
        if(bdata){
            delete[] bdata;
            bdata = 0;
        }
        app2NeuronTrees.clear();
        p2.bkg_thresh = -1;

        for(int i=0; i<startMarkers.size(); i++){

            qDebug()<<i<<" : proc_app2---------------";

            BoundingBox box = BoundingBox(start_x,start_y,start_z,end_x,end_y,end_z);
            if(!box.isInner(XYZ(startMarkers[i].x,startMarkers[i].y,startMarkers[i].z))){
                continue;
            }

            LocationSimple m = LocationSimple(startMarkers[i].x + 1 - start_x, startMarkers[i].y + 1 - start_y, startMarkers[i].z + 1 - start_z);
            p2.landmarks.push_back(m);
            proc_app2(p2);
            p2.landmarks.clear();
            NeuronTree app2NeuronTree = NeuronTree();
//            sortSWC(p2.result);
            app2NeuronTree.deepCopy(p2.result);

            if(direction == 0){
                app2NeuronTrees.push_back(app2NeuronTree);
                continue;
            }

            BranchTree tb = BranchTree();
            tb.initialize(app2NeuronTree);
            qDebug()<<"---------get level0Index------";
            vector<int> level0Index = vector<int>();
            tb.get_level_index(level0Index,0);

            XYZ p;
            if(direction == 1){
                p = XYZ(1,0,0);
            }else if (direction == 2) {
                p = XYZ(-1,0,0);
            }else if (direction == 3) {
                p = XYZ(0,1,0);
            }else if (direction == 4) {
                p = XYZ(0,-1,0);
            }else if (direction == 5) {
                p = XYZ(0,0,1);
            }else if (direction == 6) {
                p = XYZ(0,0,-1);
            }

            vector<bool> branchDFlag = vector<bool>(tb.branchs.size(),false);
            qDebug()<<"level 0 size: "<<level0Index.size();

            for(int j=0; j<level0Index.size(); j++){

                qDebug()<<"j: "<<j<<" index: "<<level0Index[j];

                vector<V3DLONG> pointsIndex = vector<V3DLONG>();
                qDebug()<<"---------------aaaaa----------------";
                tb.branchs[level0Index[j]].get_pointsIndex_of_branch(pointsIndex,app2NeuronTree);

                qDebug()<<"pointsIndex size: "<<pointsIndex.size();


                vector<NeuronSWC> points;
                for(int k=0; k<pointsIndex.size(); k++){
                    points.push_back(app2NeuronTree.listNeuron[pointsIndex[k]]);
                }
                XYZ p1 = getLineDirection(points);
                double dire;
                if(p1 == XYZ(-1,-1,-1)){
                    dire = -1;
                }else {
                    dire = dot(p,p1)/(norm(p)*norm(p1));
                }
                qDebug()<<" dire: "<<dire;

                if(dire>0){
                    branchDFlag[level0Index[j]] = true;
                }

            }

            qDebug()<<"---------------start to flag dbranch---------";

            int maxLevel = tb.get_max_level();
            qDebug()<<"maxLevel: "<<maxLevel;

            map<Branch*,int> branchMap = map<Branch*,int>();
            for(int j=0; j<tb.branchs.size(); j++){
                Branch* b = &(tb.branchs[j]);
                branchMap[b] = j;
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
            vector<bool> pointDIndex = vector<bool>(app2NeuronTree.listNeuron.size(),false);
            for(int j=0; j<branchDFlag.size(); j++){
                if(branchDFlag[j]){
                    vector<V3DLONG> pointIndex = vector<V3DLONG>();
                    pointIndex.clear();

                    tb.branchs[j].get_pointsIndex_of_branch(pointIndex,app2NeuronTree);
                    qDebug()<<"point size: "<<pointIndex.size();
                    for(int k=1; k<pointIndex.size(); k++){
                        pointDIndex[pointIndex[k]] = true;
                        pointDNum++;
                    }
                }
            }

            qDebug()<<"delete point number: "<<pointDNum;

            qDebug()<<"---------start to copy------------------";

            QList<NeuronSWC> listNeuron = QList<NeuronSWC>();
            listNeuron.clear();
            for(int j=0; j<pointDIndex.size(); j++){
                if(!pointDIndex[j]){
                    NeuronSWC p = app2NeuronTree.listNeuron[j];
                    listNeuron.append(p);
                }
            }

            app2NeuronTree.listNeuron.clear();
            app2NeuronTree.listNeuron = listNeuron;

            app2NeuronTrees.push_back(app2NeuronTree);

        }

    }

    if(in_sz){
        delete[] in_sz;
        in_sz = 0;
    }
    if(app2Image)
        delete app2Image;
    return app2NeuronTrees;
}

void imageBlock::getGlobelNeuronTree(NeuronTree &nt){
    for(int j=0; j<nt.listNeuron.size(); j++){
        nt.listNeuron[j].x += start_x;
        nt.listNeuron[j].y += start_y;
        nt.listNeuron[j].z += start_z;
    }
}

void imageBlock::getLocalNeuronTree(NeuronTree &nt){
    for(int j=0; j<nt.listNeuron.size(); j++){
        nt.listNeuron[j].x -= start_x;
        nt.listNeuron[j].y -= start_y;
        nt.listNeuron[j].z -= start_z;
    }
}

void imageBlock::getGlobelNeuronTrees(vector<NeuronTree> &trees){
    for(int i=0; i<trees.size(); i++){
        getGlobelNeuronTree(trees[i]);
    }
}

NeuronTree imageBlock::cutBlockSWC(const NeuronTree &nt){
    NeuronTree cut = NeuronTree();

    BoundingBox block = BoundingBox(start_x,start_y,start_z,end_x,end_y,end_z);

    for(int i=0; i<nt.listNeuron.size(); i++){
        XYZ v = XYZ(nt.listNeuron[i].x,nt.listNeuron[i].y,nt.listNeuron[i].z);
        if(block.isInner(v)){
            cut.listNeuron.push_back(nt.listNeuron[i]);
        }
    }
    return cut;
}

void imageBlock::getTipBlocks(vector<NeuronTree> &trees, BoundingBox box, vector<imageBlock> &blockList){

    qDebug()<<"--------------in getTipBlocks----------------";

    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    LandmarkList tip_out;
    LandmarkList tip_in;

    for(int i=0; i<trees.size(); i++){
        NeuronTree& nt = trees[i];
        nt.hashNeuron.clear();
        int pointSize = nt.listNeuron.size();
        qDebug()<<"pointSize: "<<pointSize;
        for(int j=0; j<pointSize; j++){
            nt.hashNeuron.insert(nt.listNeuron[j].n,j);
        }
        vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
        for(V3DLONG j=0; j<pointSize; j++){
            V3DLONG prt = nt.listNeuron[j].parent;
            if(prt != -1){
                V3DLONG prtIndex = nt.hashNeuron.value(prt);
                children[prtIndex].push_back(j);
            }
        }
        qDebug()<<"--------children end---------";
        vector<bool> isOverlapPoint = vector<bool>(pointSize,false);
        for(int j=0; j<pointSize; j++){
            if(children[j].size() == 0){
                qDebug()<<j<<" point------------";
                LocationSimple newTip;
                NeuronSWC curr = nt.listNeuron.at(j);
                if(curr.x < start_x + overlap || curr.x > end_x - overlap
                        || curr.y < start_y + overlap || curr.y > end_y - overlap
                        || curr.z < start_z + overlap || curr.z > end_z - overlap){
                    if( curr.x < start_x + overlap)
                    {
                        while (curr.x < start_x + overlap) {
                            isOverlapPoint[nt.hashNeuron.value(curr.n)] = true;
                            int currPrtIndex = nt.hashNeuron.value(curr.parent);
                            curr = nt.listNeuron[currPrtIndex];
                            qDebug()<<"curr: "<<QString::number(curr.x).toStdString().c_str()<<" "
                                   <<QString::number(curr.y).toStdString().c_str()<<" "<<QString::number(curr.z).toStdString().c_str();
                        }
                        newTip.x = curr.x;
                        newTip.y = curr.y;
                        newTip.z = curr.z;
                        newTip.radius = curr.r;
                        tip_left.push_back(newTip);
                    }else if (curr.x > end_x - overlap)
                    {
                        while (curr.x > end_x - overlap) {
                            isOverlapPoint[nt.hashNeuron.value(curr.n)] = true;
                            int currPrtIndex = nt.hashNeuron.value(curr.parent);
                            curr = nt.listNeuron[currPrtIndex];
                            qDebug()<<"curr: "<<QString::number(curr.x).toStdString().c_str()<<" "
                                   <<QString::number(curr.y).toStdString().c_str()<<" "<<QString::number(curr.z).toStdString().c_str();
                        }
                        newTip.x = curr.x;
                        newTip.y = curr.y;
                        newTip.z = curr.z;
                        newTip.radius = curr.r;
                        tip_right.push_back(newTip);
                    }else if (curr.y < start_y + overlap)
                    {
                        while (curr.y < start_y + overlap) {
                            isOverlapPoint[nt.hashNeuron.value(curr.n)] = true;
                            int currPrtIndex = nt.hashNeuron.value(curr.parent);
                            curr = nt.listNeuron[currPrtIndex];
                            qDebug()<<"curr: "<<QString::number(curr.x).toStdString().c_str()<<" "
                                   <<QString::number(curr.y).toStdString().c_str()<<" "<<QString::number(curr.z).toStdString().c_str();
                        }
                        newTip.x = curr.x;
                        newTip.y = curr.y;
                        newTip.z = curr.z;
                        tip_up.push_back(newTip);
                    }else if (curr.y > end_y - overlap)
                    {
                        while (curr.y > end_y - overlap) {
                            isOverlapPoint[nt.hashNeuron.value(curr.n)] = true;
                            int currPrtIndex = nt.hashNeuron.value(curr.parent);
                            curr = nt.listNeuron[currPrtIndex];
                            qDebug()<<"curr: "<<QString::number(curr.x).toStdString().c_str()<<" "
                                   <<QString::number(curr.y).toStdString().c_str()<<" "<<QString::number(curr.z).toStdString().c_str();
                        }
                        newTip.x = curr.x;
                        newTip.y = curr.y;
                        newTip.z = curr.z;
                        tip_down.push_back(newTip);
                    }else if (curr.z < start_z + overlap)
                    {
                        while (curr.z < start_z + overlap) {
                            isOverlapPoint[nt.hashNeuron.value(curr.n)] = true;
                            int currPrtIndex = nt.hashNeuron.value(curr.parent);
                            curr = nt.listNeuron[currPrtIndex];
                            qDebug()<<"curr: "<<QString::number(curr.x).toStdString().c_str()<<" "
                                   <<QString::number(curr.y).toStdString().c_str()<<" "<<QString::number(curr.z).toStdString().c_str();
                        }
                        newTip.x = curr.x;
                        newTip.y = curr.y;
                        newTip.z = curr.z;
                        tip_out.push_back(newTip);
                    }else if (curr.z > end_z - overlap)
                    {
                        while (curr.z > end_z - overlap) {
                            isOverlapPoint[nt.hashNeuron.value(curr.n)] = true;
                            int currPrtIndex = nt.hashNeuron.value(curr.parent);
                            curr = nt.listNeuron[currPrtIndex];
                            qDebug()<<"curr: "<<QString::number(curr.x).toStdString().c_str()<<" "
                                   <<QString::number(curr.y).toStdString().c_str()<<" "<<QString::number(curr.z).toStdString().c_str();
                        }
                        newTip.x = curr.x;
                        newTip.y = curr.y;
                        newTip.z = curr.z;
                        tip_in.push_back(newTip);
                    }
                }
            }
        }
        QList<NeuronSWC> afterListNeuron = QList<NeuronSWC>();
        for(int j=0; j<pointSize; j++){
            if(!isOverlapPoint[j]){
                afterListNeuron.push_back(nt.listNeuron[j]);
            }
        }
        nt.listNeuron.clear();
        nt.listNeuron = afterListNeuron;
        sortSWC(nt);
    }

    if(tip_left.size()>0)
    {
        QList<LandmarkList> group_tips_left = group_tips(tip_left,256,1);
        for(int i = 0; i < group_tips_left.size();i++)
            getImageBlockByTips(group_tips_left[i],blockList,256,1,box);
    }
    if(tip_right.size()>0)
    {
        QList<LandmarkList> group_tips_right = group_tips(tip_right,256,2);
        for(int i = 0; i < group_tips_right.size();i++)
            getImageBlockByTips(group_tips_right[i],blockList,256,2,box);
    }
    if(tip_up.size()>0)
    {
        QList<LandmarkList> group_tips_up = group_tips(tip_up,256,3);
        for(int i = 0; i < group_tips_up.size();i++)
            getImageBlockByTips(group_tips_up[i],blockList,256,3,box);
    }
    if(tip_down.size()>0)
    {
        QList<LandmarkList> group_tips_down = group_tips(tip_down,256,4);
        for(int i = 0; i < group_tips_down.size();i++)
            getImageBlockByTips(group_tips_down[i],blockList,256,4,box);
    }

    if(tip_out.size()>0)
    {
        QList<LandmarkList> group_tips_out = group_tips(tip_out,256,5);
        for(int i = 0; i < group_tips_out.size();i++)
            getImageBlockByTips(group_tips_out[i],blockList,256,5,box);
    }

    if(tip_in.size()>0)
    {
        QList<LandmarkList> group_tips_in = group_tips(tip_in,256,6);
        for(int i = 0; i < group_tips_in.size();i++)
            getImageBlockByTips(group_tips_in[i],blockList,256,6,box);
    }

}

void ultratracerTerafly(int type, V3DPluginCallback2 &callback, QWidget *parent){

    BoundingBox box = getGlobalBoundingBox(callback);
    imageBlock block = getFirstBlock(callback,box);

    qDebug()<<"---------get block end------------";

    QString brainPath = callback.getPathTeraFly();

    NeuronTree ori = callback.getSWCTeraFly();
    sortSWC(ori);
    mergeFinalResult(ori);
    NeuronTree resultTree = ultratracerInBox2(brainPath,block,ori,box,callback);

//    vector<NeuronTree> trees = vector<NeuronTree>();
//    trees.push_back(ori);
//    trees.push_back(resultTree);
//    NeuronTree merge =  mergeNeuronTrees(trees);

    callback.setSWCTeraFly(resultTree);
}

NeuronTree ultratracerInBox(QString brainPath, imageBlock block, NeuronTree ori, BoundingBox box, V3DPluginCallback2 &callback){
    vector<imageBlock> blockList = vector<imageBlock>();

    vector<NeuronTree> resultTrees = vector<NeuronTree>();

    vector<NeuronTree> trees = block.getNeuronTrees(brainPath, callback);
    block.getGlobelNeuronTrees(trees);

    if(block.direction == 0 && !ori.listNeuron.isEmpty()){
        NeuronTree cutOri = block.cutBlockSWC(ori);
        deleteSameBranch(trees[0],cutOri);
    }

    block.getTipBlocks(trees,box,blockList);
    resultTrees.push_back(trees[0]);

    while (!blockList.empty()) {
        imageBlock ib = blockList.front();
        blockList.erase(blockList.begin());
        vector<NeuronTree> trees = ib.getNeuronTrees(brainPath,callback);
        ib.getGlobelNeuronTrees(trees);
        ib.getTipBlocks(trees,box,blockList);
        resultTrees.insert(resultTrees.end(),trees.begin(),trees.end());
        qDebug()<<"block size: "<<blockList.size();
    }

    NeuronTree merge = mergeNeuronTrees(resultTrees);

    return merge;

}

BoundingBox getGlobalBoundingBox(V3DPluginCallback2& callback){
    BoundingBox box = BoundingBox();

    const Image4DSimple* image = callback.getImageTeraFly();
    qDebug()<<"x: "<<image->getXDim()<<" y: "<<image->getYDim()<<" z: "<<image->getZDim();

    QString fileName = image->getFileName();
    qDebug()<<"file name: "<<fileName;
    QStringList ls = fileName.split(',');
    qDebug()<<ls[2].split('[')[1]<<" "<<ls[3].split(']')[0]<<" "<<ls[4].split('[')[1]
            <<" "<<ls[5].split(']')[0]<<" "<<ls[6].split('[')[1]<<" "<<ls[7].split(']')[0];

    int dimx = ls[1].split('x')[0].split('(')[1].toInt();
    int dimy = ls[1].split('x')[1].toInt();
    int dimz = ls[1].split('x')[2].split(')')[0].toInt();
    qDebug()<<"dimx: "<<dimx<<" dimy: "<<dimy<<" dimz: "<<dimz;

    int x0 = ls[2].split('[')[1].toInt();
    int x1 = ls[3].split(']')[0].toInt();
    int y0 = ls[4].split('[')[1].toInt();
    int y1 = ls[5].split(']')[0].toInt();
    int z0 = ls[6].split('[')[1].toInt();
    int z1 = ls[7].split(']')[0].toInt();

    QString path =callback.getPathTeraFly();
    qDebug()<<"path: "<<path.toStdString().c_str();
    V3DLONG* sz = 0;
    callback.getDimTeraFly(path.toStdString(),sz);
    for(int i=0; i<3; i++){
        qDebug()<<i<<": "<<sz[i];
    }

    double timesX = sz[0]/(double) dimx;
    double timesY = sz[1]/(double) dimy;
    double timesZ = sz[2]/(double) dimz;

    v3dhandle curwin = callback.currentImageWindow();
    View3DControl * view3d = callback.getView3DControl(curwin);
    V3DLONG in_sz0[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};

    if(!view3d){
        box = BoundingBox(x0,y0,z0,x1,y1,z1);
    }else {
        V3DLONG view3d_datasz0 = view3d->dataDim1();
        V3DLONG view3d_datasz1 = view3d->dataDim2();
        V3DLONG view3d_datasz2 = view3d->dataDim3();
        int xc0 = int(double(view3d->xCut0()) * in_sz0[0] / view3d_datasz0 + 0.5);
        int xc1 = int(double(view3d->xCut1()) * in_sz0[0] / view3d_datasz0 + 0.5);
        int yc0 = int(double(view3d->yCut0()) * in_sz0[1] / view3d_datasz1 + 0.5);
        int yc1 = int(double(view3d->yCut1()) * in_sz0[1] / view3d_datasz1 + 0.5);
        int zc0 = int(double(view3d->zCut0()) * in_sz0[2] / view3d_datasz2 + 0.5);
        int zc1 = int(double(view3d->zCut1()) * in_sz0[2] / view3d_datasz2 + 0.5);

        box = BoundingBox(x0 + xc0, y0 + yc0, z0 + zc0, x0 + xc1, y0 + yc1, z0 + zc1);
    }

    qDebug()<<"before box block: "<<box.x0<<" "<<box.x1<<" "<<box.y0<<" "<<box.y1<<" "<<box.z0<<" "<<box.z1;

    box.x0 *= timesX, box.x1 *= timesX;
    box.y0 *= timesY, box.y1 *= timesY;
    box.z0 *= timesZ, box.z1 *= timesZ;

    qDebug()<<"box block: "<<box.x0<<" "<<box.x1<<" "<<box.y0<<" "<<box.y1<<" "<<box.z0<<" "<<box.z1;

    return box;

}

imageBlock getFirstBlock(V3DPluginCallback2 &callback, BoundingBox box){
//    LandmarkList app2Markers;
    LandmarkList markers = callback.getLandmarkTeraFly();

    NeuronTree ori = callback.getSWCTeraFly();
    sortSWC(ori);

//    imageBlock block = imageBlock();

//    BoundingBox box = getGlobalBoundingBox(callback);
    for(int i=0; i<markers.size(); i++){
        qDebug()<<"i "<<i<<" x: "<<markers[i].x<<" y: "<<markers[i].y<<" z: "<<markers[i].z;

        if(markers[i].color.r == 255 && markers[i].color.g == 255
                && markers[i].color.b == 255 && box.isInner(XYZ(markers[i].x,markers[i].y,markers[i].z))){
            if(ori.listNeuron.isEmpty()){
                LocationSimple m = LocationSimple(markers[i].x-1,markers[i].y-1,markers[i].z-1);
                imageBlock block = imageBlock(m.x-256,m.x+256,
                                              m.y-256,m.y+256,
                                              m.z-128,m.z+128);
                block.startMarkers.push_back(m);
                block.direction = 0;
                return block;
            }else {

                double minD = INT_MAX;
                int minIndex = -1;
                for(int j=0; j<ori.listNeuron.size(); j++){
                    XYZ tmp = XYZ(ori.listNeuron[j].x, ori.listNeuron[j].y, ori.listNeuron[j].z);
                    double d = dist_L2(XYZ(markers[i].x-1,markers[i].y-1,markers[i].z-1),tmp);
                    if(d<minD){
                        minD = d;
                        minIndex = j;
                    }
                }

                if(minD<30 && minIndex != -1){
                    NeuronTree nt1 = NeuronTree();
                    LocationSimple m = LocationSimple(ori.listNeuron[minIndex].x,
                                                      ori.listNeuron[minIndex].y,
                                                      ori.listNeuron[minIndex].z);
                    NeuronSWC tmp = ori.listNeuron[minIndex];
                    int count = 0;
                    while(tmp.parent != -1){
                        nt1.listNeuron.append(tmp);
                        int prtIndex = ori.hashNeuron.value(tmp.parent);
                        tmp = ori.listNeuron[prtIndex];
                        count++;
                        if(count>10)
                            break;
                    }
                    vector<NeuronSWC> points = vector<NeuronSWC>();
                    for(int j=0; j<nt1.listNeuron.size(); j++){
                        points.push_back(nt1.listNeuron[j]);
                    }
                    int direction = 0;
                    XYZ dire = getLineDirection(points);
                    direction = getDirection(dire);
                    vector<imageBlock> blockList = vector<imageBlock>();
                    getImageBlockByTip(m,blockList,256,direction,box);
                    if(blockList.size() == 1){
                        return blockList[0];
                    }else {
                        return imageBlock();
                    }
                }else {
                    LocationSimple m = LocationSimple(markers[i].x-1,markers[i].y-1,markers[i].z-1);
                    imageBlock block = imageBlock(m.x-256,m.x+256,
                                                  m.y-256,m.y+256,
                                                  m.z-128,m.z+128);
                    block.startMarkers.push_back(m);
                    block.direction = 0;
                    return block;
                }

            }
        }else if (markers[i].color.r == 255 && markers[i].color.g == 0
                  && markers[i].color.b == 0 && box.isInner(XYZ(markers[i].x,markers[i].y,markers[i].z))) {
            if(ori.listNeuron.isEmpty()){
                LocationSimple m = LocationSimple(markers[i].x-1,markers[i].y-1,markers[i].z-1);
                imageBlock block = imageBlock(m.x-256,m.x+256,
                                              m.y-256,m.y+256,
                                              m.z-128,m.z+128);
                block.startMarkers.push_back(m);
                block.direction = 0;
                return block;
            }else {

                double minD = INT_MAX;
                int minIndex = -1;
                for(int j=0; j<ori.listNeuron.size(); j++){
                    XYZ tmp = XYZ(ori.listNeuron[j].x, ori.listNeuron[j].y, ori.listNeuron[j].z);
                    double d = dist_L2(XYZ(markers[i].x-1,markers[i].y-1,markers[i].z-1),tmp);
                    if(d<minD){
                        minD = d;
                        minIndex = j;
                    }
                }

                if(minD<30 && minIndex != -1){
                    LocationSimple m = LocationSimple(ori.listNeuron[minIndex].x,
                                                      ori.listNeuron[minIndex].y,
                                                      ori.listNeuron[minIndex].z);
                    imageBlock block = imageBlock(m.x-256,m.x+256,
                                                  m.y-256,m.y+256,
                                                  m.z-128,m.z+128);
                    block.startMarkers.push_back(m);
                    block.direction = 0;
                    return block;

                }else {
                    LocationSimple m = LocationSimple(markers[i].x-1,markers[i].y-1,markers[i].z-1);
                    imageBlock block = imageBlock(m.x-256,m.x+256,
                                                  m.y-256,m.y+256,
                                                  m.z-128,m.z+128);
                    block.startMarkers.push_back(m);
                    block.direction = 0;
                    return block;
                }
            }
        }
    }

    return imageBlock();
}

NeuronTree imageBlock::getNeuronTree(QString brainPath, V3DPluginCallback2 &callback){

//    bool isTip = true;
//    MyMarker o = MyMarker(startMarkers[0].x,startMarkers[0].y,startMarkers[0].z);
//    for(int i=0; i<finalResult.listNeuron.size(); i++){
//        MyMarker s = MyMarker(finalResult.listNeuron[i].x,finalResult.listNeuron[i].y,finalResult.listNeuron[i].z);
//        if(dist(s,o)<10){
//            isTip = false;
//            break;
//        }
//    }
//    if(!isTip){
//        return NeuronTree();
//    }

    V3DLONG* in_sz = new V3DLONG[4];
    in_sz[0] = end_x - start_x;
    in_sz[1] = end_y - start_y;
    in_sz[2] = end_z - start_z;
    in_sz[3] = 1;
    int tolSZ = in_sz[0]*in_sz[1]*in_sz[2];
    qDebug()<<"start_x: "<<start_x<<" end_x: "<<end_x
           <<"start_y: "<<start_y<<" end_y: "<<end_y
          <<"start_z: "<<start_z<<" end_z: "<<end_z;
    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString().c_str(),start_x,end_x,start_y,end_y,start_z,end_z);

    QString saveImagePath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
            QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + ".v3draw";
    simple_saveimage_wrapper(callback,saveImagePath.toStdString().c_str(),pdata,in_sz,1);



    bool isNormal = false;
    double imgAve, imgStd;
    mean_and_std(pdata,tolSZ,imgAve,imgStd);

//    if(imgAve<30){
//        normalImage(pdata,in_sz);
//        isNormal = true;
//    }

    Image4DSimple* app2Image = new Image4DSimple();
    app2Image->setData(pdata,in_sz[0],in_sz[1],in_sz[2],in_sz[3],V3D_UINT8);

//    double imgAve, imgStd;
    mean_and_std(pdata,tolSZ,imgAve,imgStd);
    double td= (imgStd<10)? 10: imgStd;
//    int app2Th = imgAve + 0.7*td;
    double ratio = 0.01;
//    int app2Th = MAX(getPercentTh(pdata,in_sz,ratio),20);
    int app2Th = -1;



    paraApp2 p2 = paraApp2();
    p2.p4dImage = app2Image;
    p2.bkg_thresh = app2Th;
    p2.b_256cube = 0;
//    p2.f_length = 20;
    p2.xc0 = 0;
    p2.yc0 = 0;
    p2.zc0 = 0;
    p2.xc1 = in_sz[0] - 1;
    p2.yc1 = in_sz[1] - 1;
    p2.zc1 = in_sz[2] - 1;

//    p2.bkg_thresh = -1;
    bool kmeansTh = false;

    bool isBinaryProcess = false;

    LocationSimple m = LocationSimple(startMarkers[0].x + 1 - start_x, startMarkers[0].y + 1 - start_y, startMarkers[0].z + 1 - start_z);
    QList<ImageMarker> ms;
    ms.push_back(ImageMarker(m.x,m.y,m.z));
    QString saveMarkerPath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
            QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + ".marker";
    writeMarker_file(saveMarkerPath,ms);

    p2.landmarks.push_back(m);

    if(direction == 0){
        proc_app2(p2);

        NeuronTree app2NeuronTree = NeuronTree();
        app2NeuronTree.deepCopy(p2.result);
//        NeuronTree app2NeuronTree = consensus(app2Image,m,kmeansTh,callback);

        QString app2NeuronTreeOPath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
                QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + "_o.swc";
        writeSWC_file(app2NeuronTreeOPath,app2NeuronTree);

//        NeuronTree pruneTree = pruneNeuronTree(app2Image,app2NeuronTree);
//        sortSWC(pruneTree);
//        QString pruneTreeOPath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
//                QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + "_prune.swc";
//        writeSWC_file(pruneTreeOPath,pruneTree);

        if(app2Image){
            delete app2Image;
            app2Image = 0;
        }
        if(in_sz){
            delete[] in_sz;
            in_sz = 0;
        }

        return app2NeuronTree;
//        return pruneTree;

//        BranchTree tb = BranchTree();
//        tb.initialize(app2NeuronTree);
//        if(tb.branchs.size()>100){
//            isBinaryProcess = true;
//        }else {
//            return app2NeuronTree;
//        }
    }else {
        qDebug()<<"------------BinaryProcess image-----------------------";
        int btimes = 0;
        while (btimes<4) {
            proc_app2_getLine(p2);
            p2.result.hashNeuron.clear();
            int pointSize = p2.result.listNeuron.size();
            qDebug()<<"pointSize: "<<pointSize;
            for(int j=0; j<pointSize; j++){
                p2.result.hashNeuron.insert(p2.result.listNeuron[j].n,j);
            }
            vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
            for(V3DLONG j=0; j<pointSize; j++){
                V3DLONG prt = p2.result.listNeuron[j].parent;
                if(prt != -1){
                    V3DLONG prtIndex = p2.result.hashNeuron.value(prt);
                    children[prtIndex].push_back(j);
                }
            }
            int tipNum = 0;
            for(V3DLONG j=0; j<pointSize; j++){
                if(children[j].size() == 0){
                    tipNum++;
                }
            }

            if(tipNum>10){
                p2.bkg_thresh += 2;
//                BinaryProcess(pdata,in_sz);
//                p2.bkg_thresh = -1;
            }else {
                break;
            }
            btimes++;
        }

        if(p2.result.listNeuron.isEmpty() && !isNormal){
            normalImage(pdata,in_sz);
            proc_app2_getLine(p2);
        }

        NeuronTree app2NeuronTree = NeuronTree();
        app2NeuronTree.deepCopy(p2.result);

        QString app2NeuronTreeOPath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
                QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + "_o.swc";
        writeSWC_file(app2NeuronTreeOPath,app2NeuronTree);

//        NeuronTree pruneTree = pruneNeuronTree(app2Image,app2NeuronTree);
//        sortSWC(pruneTree);
//        QString pruneTreeOPath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
//                QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + "_prune.swc";
//        writeSWC_file(pruneTreeOPath,pruneTree);

        deleteBranchByDirection(app2NeuronTree,direction);

        QString app2NeuronTreePath = "D:\\reTraceTest\\" + QString::number(start_x) + " " + QString::number(end_x) + " " +
                QString::number(start_y) + " " + QString::number(end_y) + " " + QString::number(start_z) + " " + QString::number(end_z) + ".swc";
        writeSWC_file(app2NeuronTreePath,app2NeuronTree);

        if(app2Image){
            delete app2Image;
            app2Image = 0;
        }
        if(in_sz){
            delete[] in_sz;
            in_sz = 0;
        }

        return app2NeuronTree;

//        return pruneTree;
    }

    if(isBinaryProcess && direction == 0){
        int tolSZ = in_sz[0]*in_sz[1]*in_sz[2];
        unsigned char* bdata = new unsigned char[tolSZ];
        BinaryProcess(pdata,bdata,in_sz[0],in_sz[1],in_sz[2],3,5);
        for(int i=0; i<tolSZ; i++){
            pdata[i] = bdata[i];
        }
        if(bdata){
            delete[] bdata;
            bdata = 0;
        }

//        vector<NeuronTree> app2NeuronTrees = getApp2NeuronTrees(app2Th,app2Image,m);
//        NeuronTree app2NeuronTree = consensus(app2NeuronTrees,app2Image,m,callback);
        proc_app2(p2);
//        p2.landmarks.clear();
        NeuronTree app2NeuronTree = NeuronTree();
        app2NeuronTree.deepCopy(p2.result);

        if(app2Image){
            delete app2Image;
            app2Image = 0;
        }
        if(in_sz){
            delete[] in_sz;
            in_sz = 0;
        }

        return app2NeuronTree;
    }
}

void getMainTree(NeuronTree& nt){
    nt.hashNeuron.clear();
    int pointSize = nt.listNeuron.size();
    qDebug()<<"pointSize: "<<pointSize;
    for(int j=0; j<pointSize; j++){
        nt.hashNeuron.insert(nt.listNeuron[j].n,j);
    }
    vector<V3DLONG> isDeleteIndex = vector<V3DLONG>(pointSize,false);
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    for(V3DLONG j=0; j<pointSize; j++){
        V3DLONG prt = nt.listNeuron[j].parent;
        if(prt != -1 && nt.hashNeuron.contains(prt)){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children[prtIndex].push_back(j);
        }
        if(prt != -1 && !nt.hashNeuron.contains(prt)){
            isDeleteIndex[j] = true;
        }
    }

    for(V3DLONG j=0; j<pointSize; j++){
        if(isDeleteIndex[j]){
            vector<V3DLONG> queue = vector<V3DLONG>();
            for(int k=0; k<children[j].size(); k++){
                queue.push_back(children[j][k]);
            }
            while (!queue.empty()) {
                int tmp = queue.front();
                queue.erase(queue.begin());
                isDeleteIndex[tmp] = true;
                for(int k=0; k<children[tmp].size(); k++){
                    queue.push_back(children[tmp][k]);
                }
            }
        }
    }

    QList<NeuronSWC> listNeuron = QList<NeuronSWC>();
    for(V3DLONG j=0; j<pointSize; j++){
        if(!isDeleteIndex[j]){
            listNeuron.push_back(nt.listNeuron[j]);
        }
    }
    nt.listNeuron.clear();
    nt.listNeuron = listNeuron;
}

void imageBlock::getTipBlocks(NeuronTree &tree, BoundingBox box, vector<imageBlock> &blockList){

    imageBlock block = imageBlock(start_x+overlap,end_x-overlap,
                                  start_y+overlap,end_y-overlap,
                                  start_z+overlap,end_z-overlap);
    if(direction == 1){
        block.end_x = end_x;
    }else if (direction == 2) {
        block.start_x = start_x;
    }else if (direction == 3) {
        block.end_y = end_y;
    }else if (direction == 4) {
        block.start_y = start_y;
    }else if (direction == 5) {
        block.end_z = end_z;
    }else if (direction == 6) {
        block.start_z = start_z;
    }
    NeuronTree nt = block.cutBlockSWC(tree);
    getMainTree(nt);


    nt.hashNeuron.clear();
    int pointSize = nt.listNeuron.size();
    qDebug()<<"pointSize: "<<pointSize;
    for(int j=0; j<pointSize; j++){
        nt.hashNeuron.insert(nt.listNeuron[j].n,j);
    }
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    for(V3DLONG j=0; j<pointSize; j++){
        V3DLONG prt = nt.listNeuron[j].parent;
        if(prt != -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children[prtIndex].push_back(j);
        }
    }

    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC curr = nt.listNeuron[i];
        if(children[i].size() == 0){
            if((curr.x < start_x + overlap*2) || (curr.x > end_x - overlap*2)
                    || (curr.y < start_y + overlap*2) || (curr.y > end_y - overlap*2)
                    || (curr.z < start_z + overlap*2) || (curr.z > end_z - overlap*2)){
                LocationSimple newTip;
                newTip.x = curr.x;
                newTip.y = curr.y;
                newTip.z = curr.z;
                newTip.radius = curr.r;

                vector<NeuronSWC> points = vector<NeuronSWC>();

                NeuronSWC tmp = curr;
                int count = 0;
                while(tmp.parent != -1){
                    points.push_back(tmp);
                    int prtIndex = nt.hashNeuron.value(tmp.parent);
                    tmp = nt.listNeuron[prtIndex];
                    count++;
                    if(count>10)
                        break;
                }
                XYZ p = getLineDirection(points);
                if(p == XYZ(-1,-1,-1)){
                    return;
                }
                int dire = getDirection(p);
                getImageBlockByTip(newTip,blockList,512,dire,box);
            }
        }
    }

    tree.listNeuron.clear();
    tree.listNeuron = nt.listNeuron;
    for(int i=0; i<tree.listNeuron.size(); i++){
        if(tree.listNeuron[i].parent == -1){
            tree.listNeuron[i].x = startMarkers[0].x;
            tree.listNeuron[i].y = startMarkers[0].y;
            tree.listNeuron[i].z = startMarkers[0].z;
        }
    }

}

void mergeFinalResult(NeuronTree nt){
    vector<NeuronTree> trees = vector<NeuronTree>();
    trees.push_back(finalResult);
    trees.push_back(nt);
    NeuronTree merge = mergeNeuronTrees(trees);
    finalResult.listNeuron.clear();
    finalResult.hashNeuron.clear();
    finalResult.listNeuron = merge.listNeuron;
    for(int i=0; i<finalResult.listNeuron.size(); i++){
        finalResult.hashNeuron.insert(finalResult.listNeuron[i].n,i);
    }
}

NeuronTree ultratracerInBox2(QString brainPath, imageBlock block, NeuronTree ori, BoundingBox box, V3DPluginCallback2& callback){
    vector<imageBlock> blockList = vector<imageBlock>();

//    vector<NeuronTree> resultTrees = vector<NeuronTree>();

//    sortSWC(ori);
//    mergeFinalResult(ori);

    NeuronTree tree = block.getNeuronTree(brainPath, callback);
    block.getGlobelNeuronTree(tree);

    if(block.direction == 0 && !ori.listNeuron.isEmpty()){
        NeuronTree cutOri = block.cutBlockSWC(finalResult);
        deleteSameBranch(tree,cutOri);
    }

    block.getTipBlocks(tree,box,blockList);
    mergeFinalResult(tree);
//    callback.setSWCTeraFly(finalResult);

    while (!blockList.empty()) {
        imageBlock ib = blockList.front();
        blockList.erase(blockList.begin());
        NeuronTree tree = ib.getNeuronTree(brainPath,callback);
        ib.getGlobelNeuronTree(tree);
        ib.getTipBlocks(tree,box,blockList);
        mergeFinalResult(tree);

        sort(blockList.begin(),blockList.end());
//        callback.setSWCTeraFly(finalResult);
        qDebug()<<"block size: "<<blockList.size();
    }

    return finalResult;
}

bool writeBlock(V3DPluginCallback2 &callback, QWidget *parent){
    BoundingBox box = getGlobalBoundingBox(callback);
    imageBlock block = getFirstBlock(callback,box);

    NeuronTree tree = callback.getSWCTeraFly();
    qDebug()<<"swc path: "<<tree.file.toStdString().c_str();

    if(block.start_x == 0 && block.end_x == 0 &&
            block.start_y == 0 && block.end_y == 0 &&
            block.start_z == 0 && block.end_z == 0){
        QMessageBox::information(parent,"info",QString("Please add marker in box!"));
        return false;
    }

    QString brainPath = callback.getPathTeraFly();

    QString blockDirQS = "D:\\imageBlock";
    LandmarkList markers = callback.getLandmarkTeraFly();
    LandmarkList somaMarker;
    for(int i=0; i<markers.size(); i++){
        if(markers[i].color.r == 255 && markers[i].color.g == 0
                && markers[i].color.b == 0){
            somaMarker.append(markers[i]);
        }
    }
    if(somaMarker.size() != 1){
        QMessageBox::information(parent,"info",QString("Please add one soma marker(red) only!"));
        return false;
    }

    QString blockPath = blockDirQS + "\\" + QString::number((int)somaMarker[0].x) + "_" +
            QString::number((int)somaMarker[0].y) + "_" + QString::number((int)somaMarker[0].z)
            + ".smarker";

    qDebug()<<"path : "<<blockPath.toStdString().c_str();

    QFileInfo blockFileInfo = QFileInfo(blockPath);
    if(blockFileInfo.exists()){
        qDebug()<<"----------is file---------";
        FILE* fp = fopen(blockPath.toLatin1(),"a");
        qDebug()<<"fp after";
        if(!fp){
            QMessageBox::information(parent,"info",QString("Could not open the file to save the imageBlock!"));
            return false;
        }
        fprintf(fp,"%.3f %.3f %.3f %ld %ld %ld %ld %ld %ld %d %.3f %.3f %.3f %.3f %.3f %.3f %s\n",
                block.startMarkers[0].x,block.startMarkers[0].y,block.startMarkers[0].z,
                block.start_x,block.end_x,block.start_y,block.end_y,block.start_z,block.end_z,block.direction,
                box.x0,box.y0,box.z0,box.x1,box.y1,box.z1,brainPath.toStdString().c_str());
        fclose(fp);
    }else {
        qDebug()<<"----------make file---------";
        QDir blockDir = QDir(blockDirQS);
        if(!blockDir.exists()){
            blockDir.mkdir(blockDirQS);
        }
        FILE* fp = fopen(blockPath.toLatin1(),"a");
        qDebug()<<"fp after";
        if(!fp){
            QMessageBox::information(parent,"info",QString("Could not open the file to save the imageBlock!"));
            return false;
        }
        fprintf(fp,"##x,y,z,start_x,end_x,start_y,end_y,start_z,end_z,direction,BondingBox_x0,BondingBox_y0,BondingBox_z0,BondingBox_x1,BondingBox_y1,BondingBox_z1,brainPath\n");
        fprintf(fp,"%.3f %.3f %.3f %ld %ld %ld %ld %ld %ld %d %.3f %.3f %.3f %.3f %.3f %.3f %s\n",
                block.startMarkers[0].x,block.startMarkers[0].y,block.startMarkers[0].z,
                block.start_x,block.end_x,block.start_y,block.end_y,block.start_z,block.end_z,block.direction,
                box.x0,box.y0,box.z0,box.x1,box.y1,box.z1,brainPath.toStdString().c_str());
        fclose(fp);
    }
    QMessageBox::information(parent,"info",QString("save file to ")+blockPath);
    return true;
}

bool readBlocks(const QString &filename, vector<imageBlock>& blocks, vector<BoundingBox>& boxs, QString& brainPath){
    vector<std::string> arrayList = vector<std::string>();
    qDebug()<<"-------------in read Blocks------------";
    std::ifstream infile;
    infile.open(filename.toStdString().c_str());

    if(infile){
        std::string str;
        while (std::getline(infile,str)) {
            arrayList.push_back(str);
        }
    }else {
        qDebug()<<"read imageBlock failed!";
        return false;
    }
    infile.close();

    for(int i=0; i<arrayList.size(); i++){

        qDebug()<<"i: "<<i;

        QString current = QString::fromStdString(arrayList[i]);
//        current.fromStdString(arrayList.at(i));
        QStringList s = current.split(QString(" "));
//        qDebug()<<"s[0].left(0) : "<<s[0].left(0);
        if(s[0].left(1) == "#")
            continue;
        imageBlock block = imageBlock();
        BoundingBox box = BoundingBox();
        LocationSimple m;
        qDebug()<<"s0: "<<s[0].toStdString().c_str()<<" s1: "<<s[1].toStdString().c_str()<<" s2: "<<s[2].toStdString().c_str();
        m.x = s[0].toFloat();
        m.y = s[1].toFloat();
        m.z = s[2].toFloat();
        qDebug()<<"mx: "<<m.x<<" my: "<<m.y<<" mz: "<<m.z;
        block.start_x = s[3].toLongLong();
        block.end_x = s[4].toLongLong();
        block.start_y = s[5].toLongLong();
        block.end_y = s[6].toLongLong();
        block.start_z = s[7].toLongLong();
        block.end_z = s[8].toLongLong();
        block.direction = s[9].toInt();
        box.x0 = s[10].toFloat();
        box.y0 = s[11].toFloat();
        box.z0 = s[12].toFloat();
        box.x1 = s[13].toFloat();
        box.y1 = s[14].toFloat();
        box.z1 = s[15].toFloat();
        if(i==1){
            brainPath = s[16];
        }
        block.startMarkers.push_back(m);
        blocks.push_back(block);
        boxs.push_back(box);
    }

    return true;
}

bool tracingPipeline(QString imageBlockPath, QString swcFile, V3DPluginCallback2& callback){
    vector<imageBlock> blocks = vector<imageBlock>();
    vector<BoundingBox> boxs = vector<BoundingBox>();
    QString brainPath;
    if(!readBlocks(imageBlockPath,blocks,boxs,brainPath)){
        return false;
    }
    if(blocks.size() != boxs.size()){
        qDebug()<<"block size is not equal to box siz!";
        return false;
    }
    NeuronTree ori = readSWC_file(swcFile);

    sortSWC(ori);
    mergeFinalResult(ori);
    for(int i=0; i<blocks.size(); i++){

        int minIndex = -1;
        double minD = INT_MAX;
        for(int j=0; j<ori.listNeuron.size(); j++){
            XYZ tmp = XYZ(ori.listNeuron[j].x, ori.listNeuron[j].y, ori.listNeuron[j].z);
            double d = dist_L2(XYZ(blocks[i].startMarkers[0].x,blocks[i].startMarkers[0].y, blocks[i].startMarkers[0].z),tmp);
            if(d<minD){
                minD = d;
                minIndex = j;
            }
        }
        if(minD<5 && minIndex != -1){
            blocks[i].startMarkers[0].x = ori.listNeuron[minIndex].x;
            blocks[i].startMarkers[0].y = ori.listNeuron[minIndex].y;
            blocks[i].startMarkers[0].z = ori.listNeuron[minIndex].z;
        }

        qDebug()<<blocks[i].start_x<<" "<<blocks[i].end_x<<" "<<blocks[i].start_y<<" "<<
                  blocks[i].end_y<<" "<<blocks[i].start_z<<" "<<blocks[i].end_z<<" "<<
                  blocks[i].direction<<" "<<blocks[i].startMarkers[0].x<<" "<<
                  blocks[i].startMarkers[0].y<<blocks[i].startMarkers[0].z<<
                  boxs[i].x0<<" "<<boxs[i].x1<<" "<<boxs[i].y0<<" "<<boxs[i].y1<<" "<<
                  boxs[i].z0<<" "<<boxs[i].z1<<" "<<brainPath.toStdString().c_str();

        ultratracerInBox2(brainPath,blocks[i],ori,boxs[i],callback);

    }
    writeESWC_file(swcFile,finalResult);
    return true;
}


void mirrorImage(unsigned char* pdata, V3DLONG* inSZ, int mirrorDirection){
    if(mirrorDirection == 0){
        for(int z=0; z<inSZ[2]; z++){
            for(int y=0; y<inSZ[1]; y++){
                for(int x=0; x<inSZ[0]/2; x++){
                    int index = z*inSZ[0]*inSZ[1] + y*inSZ[0] + x;
                    unsigned char tmp = pdata[index];
                    int mirrorIndex = z*inSZ[0]*inSZ[1] + y*inSZ[0] + (inSZ[0] - x - 1);
                    pdata[index] = pdata[mirrorIndex];
                    pdata[mirrorIndex] = tmp;
                }
            }
        }
    }else if (mirrorDirection == 1) {
        for(int z=0; z<inSZ[2]; z++){
            for(int x=0; x<inSZ[0]; x++){
                for(int y=0; y<inSZ[1]/2; y++){
                    int index = z*inSZ[0]*inSZ[1] + y*inSZ[0] + x;
                    unsigned char tmp = pdata[index];
                    int mirrorIndex = z*inSZ[0]*inSZ[1] + (inSZ[1] - y - 1)*inSZ[0] + x;
                    pdata[index] = pdata[mirrorIndex];
                    pdata[mirrorIndex] = tmp;
                }
            }
        }
    }else if (mirrorDirection == 2) {
        for(int y=0; y<inSZ[1]; y++){
            for(int x=0; x<inSZ[0]; x++){
                for(int z=0; z<inSZ[2]/2; z++){
                    int index = z*inSZ[0]*inSZ[1] + y*inSZ[0] + x;
                    unsigned char tmp = pdata[index];
                    int mirrorIndex = (inSZ[2] - z - 1)*inSZ[0]*inSZ[1] + y*inSZ[0] + x;
                    pdata[index] = pdata[mirrorIndex];
                    pdata[mirrorIndex] = tmp;
                }
            }
        }
    }
}

NeuronTree getApp2WithParameter(Image4DSimple* image, LocationSimple m, int downSampleTimes, int mirrorDirection, int th, int type){
    paraApp2 p = paraApp2();
    p.p4dImage = image;
    V3DLONG inSZ[4] = {p.p4dImage->getXDim(),p.p4dImage->getYDim(),p.p4dImage->getZDim(),p.p4dImage->getCDim()};
    p.b_256cube = false;
    p.bkg_thresh = th;
    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = inSZ[0] - 1;
    p.yc1 = inSZ[1] - 1;
    p.zc1 = inSZ[2] - 1;
    p.landmarks.push_back(m);

    unsigned char* pdata = p.p4dImage->getRawData();
    int totalSZ = p.p4dImage->getTotalUnitNumber();

    unsigned char* pdataCopy = new unsigned char[totalSZ];
    for(int i=0; i<totalSZ; i++){
        pdataCopy[i] = pdata[i];
    }
    Image4DSimple* copyImage = new Image4DSimple();
    copyImage->setData(pdataCopy,p.p4dImage);

    Image4DSimple* downSampleImage = new Image4DSimple();

    unsigned char* downSampleData1d = 0;
    V3DLONG downSampleSZ[4] = {0,0,0,0};
    if(downSampleTimes == 1){
        mirrorImage(pdataCopy,inSZ,mirrorDirection);
        p.p4dImage = copyImage;
        for(int i=0; i<p.landmarks.size(); i++){
            if(mirrorDirection == 0){
                p.landmarks[i].x = inSZ[0] - 1 - p.landmarks[i].x;
            }else if (mirrorDirection == 1) {
                p.landmarks[i].y = inSZ[1] - 1 - p.landmarks[i].y;
            }else if (mirrorDirection == 2) {
                p.landmarks[i].z = inSZ[2] - 1 - p.landmarks[i].z;
            }
        }

    }else{
        downsampling_img_xyz(pdata,inSZ,downSampleTimes,downSampleTimes,downSampleData1d,downSampleSZ);
        mirrorImage(downSampleData1d,downSampleSZ,mirrorDirection);
        downSampleImage->setData(downSampleData1d,downSampleSZ[0],downSampleSZ[1],downSampleSZ[2],downSampleSZ[3],V3D_UINT8);
        p.p4dImage = downSampleImage;
        p.xc0 = (int) (p.xc0/downSampleTimes + 0.5); if(p.xc0<0) p.xc0 = 0;
        p.xc1 = (int) (p.xc1/downSampleTimes + 0.5); if(p.xc1>downSampleSZ[0]-1) p.xc1 = downSampleSZ[0]-1;
        p.yc0 = (int) (p.yc0/downSampleTimes + 0.5); if(p.yc0<0) p.yc0 = 0;
        p.yc1 = (int) (p.yc1/downSampleTimes + 0.5); if(p.yc1>downSampleSZ[1]-1) p.yc1 = downSampleSZ[1]-1;
        p.zc0 = (int) (p.zc0/downSampleTimes + 0.5); if(p.zc0<0) p.zc0 = 0;
        p.zc1 = (int) (p.zc1/downSampleTimes + 0.5); if(p.zc1>downSampleSZ[2]-1) p.zc1 = downSampleSZ[2]-1;
        p.bkg_thresh = -1;
        for(int i=0; i<p.landmarks.size(); i++){
            p.landmarks[i].x = p.landmarks[i].x/downSampleTimes;
            p.landmarks[i].y = p.landmarks[i].y/downSampleTimes;
            p.landmarks[i].z = p.landmarks[i].z/downSampleTimes;

            if(mirrorDirection == 0){
                p.landmarks[i].x = downSampleSZ[0] - 1 - p.landmarks[i].x;
            }else if (mirrorDirection == 1) {
                p.landmarks[i].y = downSampleSZ[1] - 1 - p.landmarks[i].y;
            }else if (mirrorDirection == 2) {
                p.landmarks[i].z = downSampleSZ[2] - 1 - p.landmarks[i].z;
            }
        }
    }



    proc_app2(p);
    sortSWC(p.result);
    NeuronTree app2NeuronTree = NeuronTree();
    app2NeuronTree.deepCopy(p.result);

    for(int i=0; i<app2NeuronTree.listNeuron.size(); i++){

        if(downSampleTimes == 1){
            if(mirrorDirection == 0){
                app2NeuronTree.listNeuron[i].x = inSZ[0] - 1 - app2NeuronTree.listNeuron[i].x;
            }else if (mirrorDirection == 1) {
                app2NeuronTree.listNeuron[i].y = inSZ[1] - 1 - app2NeuronTree.listNeuron[i].y;
            }else if (mirrorDirection == 2) {
                app2NeuronTree.listNeuron[i].z = inSZ[2] - 1 - app2NeuronTree.listNeuron[i].z;
            }
        }else {
            if(mirrorDirection == 0){
                app2NeuronTree.listNeuron[i].x = downSampleSZ[0] - 1 - app2NeuronTree.listNeuron[i].x;
            }else if (mirrorDirection == 1) {
                app2NeuronTree.listNeuron[i].y = downSampleSZ[1] - 1 - app2NeuronTree.listNeuron[i].y;
            }else if (mirrorDirection == 2) {
                app2NeuronTree.listNeuron[i].z = downSampleSZ[2] - 1 - app2NeuronTree.listNeuron[i].z;
            }

            app2NeuronTree.listNeuron[i].x *= downSampleTimes;
            app2NeuronTree.listNeuron[i].y *= downSampleTimes;
            app2NeuronTree.listNeuron[i].z *= downSampleTimes;
        }

    }

    for(int i=0; i<app2NeuronTree.listNeuron.size(); i++){
        app2NeuronTree.listNeuron[i].type = type;
    }

//    QString file = imgFileName + "_app2_downsample" + QString::number(downSampleTimes)
//            + "_mirror_" + QString::number(mirrorDirection) + "_th_" +QString::number(th) + ".swc";
//    writeSWC_file(file,app2NeuronTree);
    if(downSampleImage)
        delete downSampleImage;
    return app2NeuronTree;

}

void rotateImage(unsigned char* inData, unsigned char* &outData, V3DLONG* inSZ, V3DLONG* outSZ, int rotateAxis, int angle, int& minX, int& minY, int& minZ){
    V3DLONG totalSZ = inSZ[0]*inSZ[1]*inSZ[2];
    double centerX = inSZ[0]/2.0;
    double centerY = inSZ[1]/2.0;
    double centerZ = inSZ[2]/2.0;
    vector<vector<int>> rotateCoordinate = vector<vector<int>>(totalSZ,vector<int>(3,0));
    for(int i=0; i<totalSZ; i++){
        int x = i%inSZ[0];
        int y = (i/inSZ[0])%inSZ[1];
        int z = i/(inSZ[0]*inSZ[1]);
        int rx, ry, rz;
        if(rotateAxis == 0){
            rx = (x - centerX);
            ry = (int)(cos(angle)*(y - centerY) - sin(angle)*(z - centerZ) + 0.5);
            rz = (int)(sin(angle)*(y - centerY) + cos(angle)*(z - centerZ) + 0.5);
        }else if (rotateAxis == 1) {
            rx = (int)(cos(angle)*(x - centerX) + sin(angle)*(z - centerZ) + 0.5);
            ry = (y - centerY);
            rz = (int)(-sin(angle)*(x - centerX) + cos(angle)*(z - centerZ) + 0.5);
        }else {
            rx = (int)(cos(angle)*(x - centerX) - sin(angle)*(y - centerY) + 0.5);
            ry = (int)(sin(angle)*(x - centerX) + cos(angle)*(y - centerY) + 0.5);
            rz = (z-centerZ);
        }
        rotateCoordinate[i][0] = rx;
        rotateCoordinate[i][1] = ry;
        rotateCoordinate[i][2] = rz;
    }
    minX = INT_MAX,  minY = INT_MAX,  minZ = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN, maxZ = INT_MIN;
    for(int i=0; i<totalSZ; i++){
        if(rotateCoordinate[i][0]<minX) minX = rotateCoordinate[i][0];
        if(rotateCoordinate[i][0]>maxX) maxX = rotateCoordinate[i][0];
        if(rotateCoordinate[i][1]<minY) minY = rotateCoordinate[i][1];
        if(rotateCoordinate[i][1]>maxY) maxY = rotateCoordinate[i][1];
        if(rotateCoordinate[i][2]<minZ) minZ = rotateCoordinate[i][2];
        if(rotateCoordinate[i][2]>maxZ) maxZ = rotateCoordinate[i][2];
    }
    outSZ[0] = maxX - minX + 1;
    outSZ[1] = maxY - minY + 1;
    outSZ[2] = maxZ - minZ + 1;
    outSZ[3] = 1;

    V3DLONG totalRSZ = outSZ[0]*outSZ[1]*outSZ[2];
    if(outData){
        delete[] outData;
        outData = 0;
    }
    outData = new unsigned char[totalRSZ];
    if(outData){
        qDebug()<<"outData is exsit";
    }else {
        qDebug()<<"outData is not  exsit";
    }
    memset(outData,0,totalRSZ*sizeof(unsigned char));
    for(int i=0; i<totalSZ; i++){
        int rx = rotateCoordinate[i][0] - minX;
        int ry = rotateCoordinate[i][1] - minY;
        int rz = rotateCoordinate[i][2] - minZ;
        int rIndex = rz*outSZ[0]*outSZ[1] + ry*outSZ[0] + rx;
        outData[rIndex] = inData[i];
    }
}

NeuronTree getApp2RotateImage(Image4DSimple* image, LocationSimple m, int rotateAxis, int angle, int th){
    paraApp2 p = paraApp2();
    p.p4dImage = image;
//    QString imgFileName = QString(p.p4dImage->getFileName());
    V3DLONG inSZ[4] = {p.p4dImage->getXDim(),p.p4dImage->getYDim(),p.p4dImage->getZDim(),p.p4dImage->getCDim()};
    p.b_256cube = false;
    p.bkg_thresh = th;
    unsigned char* pdata = p.p4dImage->getRawData();
    int totalSZ = p.p4dImage->getTotalUnitNumber();
    p.landmarks.push_back(m);

    unsigned char* rdata = 0;
    V3DLONG rSZ[4] = {0,0,0,0};
    int minX, minY, minZ;
    rotateImage(pdata,rdata,inSZ,rSZ,rotateAxis,angle,minX,minY,minZ);

    qDebug()<<"rSZ: "<<rSZ[0]<<" "<<rSZ[1]<<" "<<rSZ[2];
    if(rdata){
        qDebug()<<"rdata exsit";
    }else {
        qDebug()<<"rdata is not exsit";
    }

//    simple_saveimage_wrapper(callback,(imgFileName+"_ratateImage" + QString::number(rotateAxis)+ ".v3draw").toStdString().c_str(),rdata,rSZ,1);

    Image4DSimple* rImage = new Image4DSimple();
    rImage->setData(rdata,rSZ[0],rSZ[1],rSZ[2],rSZ[3],V3D_UINT8);
    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = rSZ[0] - 1;
    p.yc1 = rSZ[1] - 1;
    p.zc1 = rSZ[2] - 1;

    p.p4dImage = rImage;

    double centerX = inSZ[0]/2.0, centerY = inSZ[1]/2.0, centerZ = inSZ[2]/2.0;

    for(int i=0; i<p.landmarks.size(); i++){
        float x = p.landmarks[i].x - centerX - 1;
        float y = p.landmarks[i].y - centerY - 1;
        float z = p.landmarks[i].z - centerZ - 1;
        if(rotateAxis == 0){
            p.landmarks[i].x = x - minX + 1;
            p.landmarks[i].y = cos(angle)*y - sin(angle)*z - minY + 1;
            p.landmarks[i].z = sin(angle)*y + cos(angle)*z - minZ + 1;
        }else if (rotateAxis == 1) {
            p.landmarks[i].x = cos(angle)*x + sin(angle)*z - minX + 1;
            p.landmarks[i].y = y - minY + 1;
            p.landmarks[i].z = -sin(angle)*x + cos(angle)*z - minZ + 1;
        }else {
            p.landmarks[i].x = cos(angle)*x - sin(angle)*y - minX + 1;
            p.landmarks[i].y = sin(angle)*x + cos(angle)*y - minY + 1;
            p.landmarks[i].z = z - minZ + 1;
        }

    }

    proc_app2(p);

    NeuronTree app2NeuronTree = NeuronTree();
    app2NeuronTree.deepCopy(p.result);

    for(int i=0; i<app2NeuronTree.listNeuron.size(); i++){
        float x = app2NeuronTree.listNeuron[i].x + minX;
        float y = app2NeuronTree.listNeuron[i].y + minY;
        float z = app2NeuronTree.listNeuron[i].z + minZ;
        if(rotateAxis == 0){
            app2NeuronTree.listNeuron[i].x = x + centerX;
            app2NeuronTree.listNeuron[i].y = (cos(-angle)*y - sin(-angle)*z) + centerY;
            app2NeuronTree.listNeuron[i].z = (sin(-angle)*y + cos(-angle)*z) + centerZ;
        }else if (rotateAxis == 1) {
            app2NeuronTree.listNeuron[i].x = (cos(-angle)*x + sin(-angle)*z) + centerX;
            app2NeuronTree.listNeuron[i].y = y + centerY;
            app2NeuronTree.listNeuron[i].z = (-sin(-angle)*x + cos(-angle)*z) + centerZ;
        }else {
            app2NeuronTree.listNeuron[i].x = (cos(-angle)*x - sin(-angle)*y) + centerX;
            app2NeuronTree.listNeuron[i].y = (sin(-angle)*x + cos(-angle)*y) + centerY;
            app2NeuronTree.listNeuron[i].z = z + centerZ;
        }
    }

//    writeSWC_file(imgFileName+"_ratateImage_" + QString::number(rotateAxis) + ".swc",app2NeuronTree);

    if(rImage){
        delete rImage;
        rImage = 0;
    }

    return app2NeuronTree;

}

vector<NeuronTree> getApp2NeuronTrees(int app2Th, Image4DSimple* image, LocationSimple m){

    vector<NeuronTree> app2NeuronTrees = vector<NeuronTree>();
    int mirror[3] = {0,1,2};
    int downSampleTimes[1] = {1};
    int type = 2;
    for(int i=0; i<2; i++){
        for(int j=0; j<3; j++){
            NeuronTree app2NeuronTree = getApp2WithParameter(image,m,downSampleTimes[i],mirror[j],app2Th,type);
            app2NeuronTrees.push_back(app2NeuronTree);
            type++;
        }
    }

    return app2NeuronTrees;

}

NeuronTree consensus(vector<NeuronTree> trees, Image4DSimple* inImg, LocationSimple m, V3DPluginCallback2& callback){
    V3DLONG sz[4] = {inImg->getXDim(),inImg->getYDim(),inImg->getZDim(),inImg->getCDim()};
    V3DLONG totalSZ = sz[0]*sz[1]*sz[2];
    vector<unsigned char*> masks = vector<unsigned char*>();
    for(int i=0; i<trees.size(); i++){
        unsigned char* mask = 0;
        for(int j=0; j<trees[i].listNeuron.size(); j++){
            trees[i].listNeuron[j].r = 2;
        }
        vector<MyMarker*> markers = swc_convert(trees[i]);
        swc2mask(mask,markers,sz[0],sz[1],sz[2]);
        masks.push_back(mask);
    }

    unsigned char* consensusImgData1d = new unsigned char[totalSZ];
    unsigned char* pdata = inImg->getRawData();

    for(int i=0; i<totalSZ; i++){
        bool flag = true;
        for(int j=0; j<masks.size(); j++){
            if(masks[j][i] != 255)
                flag = false;
        }
        if(flag){
            consensusImgData1d[i] = pdata[i];
        }else {
            consensusImgData1d[i] = 0;
        }
    }

//    QString imgFileName = QString(inImg->getFileName()) + "_consensus.v3draw";
//    simple_saveimage_wrapper(callback,imgFileName.toStdString().c_str(),consensusImgData1d,sz,1);

    for(int j=0; j<masks.size(); j++){
        if(masks[j])
            delete[] masks[j];
    }

    paraApp2 p = paraApp2();
    p.bkg_thresh = 1;
    p.b_256cube = false;
    p.landmarks.push_back(m);

    Image4DSimple* consensusImg = new Image4DSimple();
    consensusImg->setData(consensusImgData1d,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    p.p4dImage = consensusImg;
    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = p.p4dImage->getXDim()-1;
    p.yc1 = p.p4dImage->getYDim()-1;
    p.zc1 = p.p4dImage->getZDim()-1;

    proc_app2(p);

    NeuronTree result = NeuronTree();
    result.deepCopy(p.result);

    if(consensusImg)
        delete consensusImg;

    return result;

}

NeuronTree consensus(Image4DSimple* image, LocationSimple m, bool kmeansTh, V3DPluginCallback2& callback){
    unsigned char* pdata = image->getRawData();
    V3DLONG sz[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};

    int app2Th;

    if(kmeansTh){
        int k = 3;
        int* ths = getThresholdByKmeans(image,k);
        app2Th = ths[k-2];
        if(ths){
            delete[] ths;
            ths = 0;
        }
    }else {
        double imgAve, imgStd;
        V3DLONG totalSZ = sz[0]*sz[1]*sz[2];
        mean_and_std(pdata,totalSZ,imgAve,imgStd);
        double td= (imgStd<10)? 10: imgStd;
        app2Th = imgAve + 0.7*td;
    }

    vector<NeuronTree> trees  = vector<NeuronTree>();
    paraApp2 p = paraApp2();
    //origin image, don't rotate
    p.p4dImage = image;
    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = sz[0] - 1;
    p.yc1 = sz[1] - 1;
    p.zc1 = sz[2] - 1;
    p.b_256cube = false;
    p.bkg_thresh = app2Th;
    p.landmarks.push_back(m);

    qDebug()<<"xc1: "<<p.xc1<<" yc1: "<<p.yc1<<" zc1: "<<p.zc1<<" m: "<<p.landmarks[0].x<<" "<<p.landmarks[0].y<<" "<<p.landmarks[0].z;
    proc_app2(p);
    NeuronTree app2NeuronTree1 = NeuronTree();
    app2NeuronTree1.deepCopy(p.result);
    for(int i=0 ;i<app2NeuronTree1.listNeuron.size(); i++){
        app2NeuronTree1.listNeuron[i].type = 2;
    }

    //origin image, rotate 30 degrees around X axis
    int angle = 30;
    NeuronTree app2NeuronTree2 = getApp2RotateImage(image,m,0,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree2.listNeuron.size(); i++){
        app2NeuronTree2.listNeuron[i].type = 3;
    }

    //origin image, rotate 30 degrees around Y axis
    NeuronTree app2NeuronTree3 = getApp2RotateImage(image,m,1,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree3.listNeuron.size(); i++){
        app2NeuronTree3.listNeuron[i].type = 4;
    }

    //origin image, rotate 30 degrees around Z axis
    NeuronTree app2NeuronTree4 = getApp2RotateImage(image,m,2,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree4.listNeuron.size(); i++){
        app2NeuronTree4.listNeuron[i].type = 5;
    }

    //downSample image(2 times), don't rotate
    unsigned char* downSampleData1d = 0;
    V3DLONG downSampleSZ[4] = {0,0,0,0};
    downsampling_img_xyz(pdata,sz,2,2,downSampleData1d,downSampleSZ);
    Image4DSimple* downSampleImage = new Image4DSimple();
    downSampleImage->setData(downSampleData1d,downSampleSZ[0],downSampleSZ[1],downSampleSZ[2],downSampleSZ[3],V3D_UINT8);

    p.p4dImage = downSampleImage;
    p.xc1 = downSampleSZ[0] - 1;
    p.yc1 = downSampleSZ[1] - 1;
    p.zc1 = downSampleSZ[2] - 1;
    p.landmarks.clear();
    LocationSimple mm;
    mm.x = (m.x - 1)/2 +1;
    mm.y = (m.y - 1)/2 +1;
    mm.z = (m.z - 1)/2 +1;
    p.landmarks.push_back(mm);

    proc_app2(p);
    NeuronTree app2NeuronTree5 = NeuronTree();
    app2NeuronTree5.deepCopy(p.result);
    for(int i=0 ;i<app2NeuronTree5.listNeuron.size(); i++){
        app2NeuronTree5.listNeuron[i].x *= 2;
        app2NeuronTree5.listNeuron[i].y *= 2;
        app2NeuronTree5.listNeuron[i].z *= 2;
        app2NeuronTree5.listNeuron[i].type = 6;
    }

    //downSample image(2 times), rotate 30 degrees around X axis
    NeuronTree app2NeuronTree6 = getApp2RotateImage(downSampleImage,mm,0,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree6.listNeuron.size(); i++){
        app2NeuronTree6.listNeuron[i].x *= 2;
        app2NeuronTree6.listNeuron[i].y *= 2;
        app2NeuronTree6.listNeuron[i].z *= 2;
        app2NeuronTree6.listNeuron[i].type = 7;
    }

    //downSample image(2 times), rotate 30 degrees around Y axis
    NeuronTree app2NeuronTree7 = getApp2RotateImage(downSampleImage,mm,1,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree7.listNeuron.size(); i++){
        app2NeuronTree7.listNeuron[i].x *= 2;
        app2NeuronTree7.listNeuron[i].y *= 2;
        app2NeuronTree7.listNeuron[i].z *= 2;
        app2NeuronTree7.listNeuron[i].type = 8;
    }

    //downSample image(2 times), rotate 30 degrees around Z axis
    NeuronTree app2NeuronTree8 = getApp2RotateImage(downSampleImage,mm,2,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree8.listNeuron.size(); i++){
        app2NeuronTree8.listNeuron[i].x *= 2;
        app2NeuronTree8.listNeuron[i].y *= 2;
        app2NeuronTree8.listNeuron[i].z *= 2;
        app2NeuronTree8.listNeuron[i].type = 9;
    }

    //consensus
    trees.push_back(app2NeuronTree1);
    trees.push_back(app2NeuronTree2);
    trees.push_back(app2NeuronTree3);
    trees.push_back(app2NeuronTree4);
    trees.push_back(app2NeuronTree5);
    trees.push_back(app2NeuronTree6);
    trees.push_back(app2NeuronTree7);
    trees.push_back(app2NeuronTree8);

    NeuronTree consensusTree = consensus(trees,image,m,callback);

    if(image){
        delete image;
        image = 0;
    }

    if(downSampleImage){
        delete downSampleImage;
        downSampleImage = 0;
    }

    return consensusTree;
}

void deleteBranchByDirection(NeuronTree& target, int direction){
    BranchTree tb = BranchTree();
    tb.initialize(target);
    qDebug()<<"---------get level0Index------";
    vector<int> level0Index = vector<int>();
    tb.get_level_index(level0Index,0);

    XYZ p;
    if(direction == 1){
        p = XYZ(1,0,0);
    }else if (direction == 2) {
        p = XYZ(-1,0,0);
    }else if (direction == 3) {
        p = XYZ(0,1,0);
    }else if (direction == 4) {
        p = XYZ(0,-1,0);
    }else if (direction == 5) {
        p = XYZ(0,0,1);
    }else if (direction == 6) {
        p = XYZ(0,0,-1);
    }

    vector<bool> branchDFlag = vector<bool>(tb.branchs.size(),false);
    qDebug()<<"level 0 size: "<<level0Index.size();

    for(int j=0; j<level0Index.size(); j++){

        qDebug()<<"j: "<<j<<" index: "<<level0Index[j];

        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        qDebug()<<"---------------aaaaa----------------";
        tb.branchs[level0Index[j]].get_pointsIndex_of_branch(pointsIndex,target);

        qDebug()<<"pointsIndex size: "<<pointsIndex.size();


        vector<NeuronSWC> points;
        for(int k=0; k<pointsIndex.size(); k++){
            points.push_back(target.listNeuron[pointsIndex[k]]);
        }
        XYZ p1 = getLineDirection(points);
        double dire;
        if(p1 == XYZ(-1,-1,-1)){
            dire = -1;
        }else {
            dire = dot(p,p1)/(norm(p)*norm(p1));
        }
        qDebug()<<" dire: "<<dire;

        if(dire>0){
            branchDFlag[level0Index[j]] = true;
        }

    }

    qDebug()<<"---------------start to flag dbranch---------";

    int maxLevel = tb.get_max_level();
    qDebug()<<"maxLevel: "<<maxLevel;

    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int j=0; j<tb.branchs.size(); j++){
        Branch* b = &(tb.branchs[j]);
        branchMap[b] = j;
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
    for(int j=0; j<branchDFlag.size(); j++){
        if(branchDFlag[j]){
            vector<V3DLONG> pointIndex = vector<V3DLONG>();
            pointIndex.clear();

            tb.branchs[j].get_pointsIndex_of_branch(pointIndex,target);
            qDebug()<<"point size: "<<pointIndex.size();
            for(int k=1; k<pointIndex.size(); k++){
                pointDIndex[pointIndex[k]] = true;
                pointDNum++;
            }
        }
    }

    qDebug()<<"delete point number: "<<pointDNum;

    qDebug()<<"---------start to copy------------------";

    QList<NeuronSWC> listNeuron = QList<NeuronSWC>();
    listNeuron.clear();
    for(int j=0; j<pointDIndex.size(); j++){
        if(!pointDIndex[j]){
            NeuronSWC p = target.listNeuron[j];
            listNeuron.append(p);
        }
    }

    target.listNeuron.clear();
    target.listNeuron = listNeuron;
}











