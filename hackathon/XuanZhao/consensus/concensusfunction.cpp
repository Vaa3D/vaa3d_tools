#include "concensusfunction.h"

#include "app2.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
#include "marker_radius.h"
#include "basic_memory.cpp"
#include "swc_convert.h"

bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result){
    return SortSWC(neurons,result,VOID,1);
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

NeuronTree getApp2WithParameter(V3DPluginCallback2& callback, int downSampleTimes, int mirrorDirection, int th, int type){
    paraApp2 p = paraApp2();
    p.initialize(callback);
    QString imgFileName = QString(p.p4dImage->getFileName());
    V3DLONG inSZ[4] = {p.p4dImage->getXDim(),p.p4dImage->getYDim(),p.p4dImage->getZDim(),p.p4dImage->getCDim()};
    p.b_256cube = false;
    p.bkg_thresh = th;

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

    QString file = imgFileName + "_app2_downsample" + QString::number(downSampleTimes)
            + "_mirror_" + QString::number(mirrorDirection) + "_th_" +QString::number(th) + ".swc";
    writeSWC_file(file,app2NeuronTree);
    if(downSampleImage)
        delete downSampleImage;
    return app2NeuronTree;

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
        float x = p.landmarks[i].x - centerX;
        float y = p.landmarks[i].y - centerY;
        float z = p.landmarks[i].z - centerZ;
        if(rotateAxis == 0){
            p.landmarks[i].x = x - minX;
            p.landmarks[i].y = cos(angle)*y - sin(angle)*z - minY;
            p.landmarks[i].z = sin(angle)*y + cos(angle)*z - minZ;
        }else if (rotateAxis == 1) {
            p.landmarks[i].x = cos(angle)*x + sin(angle)*z - minX;
            p.landmarks[i].y = y - minY;
            p.landmarks[i].z = -sin(angle)*x + cos(angle)*z - minZ;
        }else {
            p.landmarks[i].x = cos(angle)*x - sin(angle)*y - minX;
            p.landmarks[i].y = sin(angle)*x + cos(angle)*y - minY;
            p.landmarks[i].z = z - minZ;
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

vector<NeuronTree> getApp2NeuronTrees(int app2Th, V3DPluginCallback2& callback, QWidget* parent){

    vector<NeuronTree> app2NeuronTrees = vector<NeuronTree>();
    int mirror[3] = {0,1,2};
    int downSampleTimes[2] = {1,2};
    int type = 2;
    for(int i=0; i<2; i++){
        for(int j=0; j<3; j++){
            NeuronTree app2NeuronTree = getApp2WithParameter(callback,downSampleTimes[i],mirror[j],app2Th,type);
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
        int count = 0;
        for(int j=0; j<masks.size(); j++){
            if(masks[j][i] == 255)
                count++;
            if(count >= masks.size()*3/4){
                consensusImgData1d[i] = pdata[i];
            }else {
                consensusImgData1d[i] = 0;
            }
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


bool consensus(QString imagePath, LocationSimple m, bool kmeansTh, V3DPluginCallback2 &callback){
    unsigned char* pdata = 0;
    V3DLONG sz[4] = {0,0,0,0};
    int dataType = 1;
    simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);
    normalImage(pdata,sz);

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);


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
    proc_app2(p);
    NeuronTree app2NeuronTree1 = NeuronTree();
    app2NeuronTree1.deepCopy(p.result);
    for(int i=0 ;i<app2NeuronTree1.listNeuron.size(); i++){
        app2NeuronTree1.listNeuron[i].type = 2;
    }
    QString swcPath1 = imagePath + "_originImage.swc";
    writeSWC_file(swcPath1,app2NeuronTree1);

    //origin image, rotate 30 degrees around X axis
    int angle = 30;
    NeuronTree app2NeuronTree2 = getApp2RotateImage(image,m,0,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree2.listNeuron.size(); i++){
        app2NeuronTree2.listNeuron[i].type = 3;
    }
    QString swcPath2 = imagePath + "_originImage_rotateX_" + QString::number(angle) + ".swc";
    writeSWC_file(swcPath2,app2NeuronTree2);

    //origin image, rotate 30 degrees around Y axis
    NeuronTree app2NeuronTree3 = getApp2RotateImage(image,m,1,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree3.listNeuron.size(); i++){
        app2NeuronTree3.listNeuron[i].type = 4;
    }
    QString swcPath3 = imagePath + "_originImage_rotateY_" + QString::number(angle) + ".swc";
    writeSWC_file(swcPath3,app2NeuronTree3);

    //origin image, rotate 30 degrees around Z axis
    NeuronTree app2NeuronTree4 = getApp2RotateImage(image,m,2,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree4.listNeuron.size(); i++){
        app2NeuronTree4.listNeuron[i].type = 5;
    }
    QString swcPath4 = imagePath + "_originImage_rotateZ_" + QString::number(angle) + ".swc";
    writeSWC_file(swcPath4,app2NeuronTree4);

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
    m.x /= 2, m.y /= 2, m.z /= 2;
    p.landmarks.push_back(m);

    proc_app2(p);
    NeuronTree app2NeuronTree5 = NeuronTree();
    app2NeuronTree5.deepCopy(p.result);
    for(int i=0 ;i<app2NeuronTree5.listNeuron.size(); i++){
        app2NeuronTree5.listNeuron[i].x *= 2;
        app2NeuronTree5.listNeuron[i].y *= 2;
        app2NeuronTree5.listNeuron[i].z *= 2;
        app2NeuronTree5.listNeuron[i].type = 6;
    }
    QString swcPath5 = imagePath + "_downSampleImage_2.swc";
    writeSWC_file(swcPath5,app2NeuronTree5);

    //downSample image(2 times), rotate 30 degrees around X axis
    NeuronTree app2NeuronTree6 = getApp2RotateImage(downSampleImage,m,0,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree6.listNeuron.size(); i++){
        app2NeuronTree6.listNeuron[i].x *= 2;
        app2NeuronTree6.listNeuron[i].y *= 2;
        app2NeuronTree6.listNeuron[i].z *= 2;
        app2NeuronTree6.listNeuron[i].type = 7;
    }
    QString swcPath6 = imagePath + "_downSampleImage_2_rotateX_" + QString::number(angle) + ".swc";
    writeSWC_file(swcPath6,app2NeuronTree6);

    //downSample image(2 times), rotate 30 degrees around Y axis
    NeuronTree app2NeuronTree7 = getApp2RotateImage(downSampleImage,m,1,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree7.listNeuron.size(); i++){
        app2NeuronTree7.listNeuron[i].x *= 2;
        app2NeuronTree7.listNeuron[i].y *= 2;
        app2NeuronTree7.listNeuron[i].z *= 2;
        app2NeuronTree7.listNeuron[i].type = 8;
    }
    QString swcPath7 = imagePath + "_downSampleImage_2_rotateY_" + QString::number(angle) + ".swc";
    writeSWC_file(swcPath7,app2NeuronTree7);

    //downSample image(2 times), rotate 30 degrees around Z axis
    NeuronTree app2NeuronTree8 = getApp2RotateImage(downSampleImage,m,2,angle,app2Th);
    for(int i=0 ;i<app2NeuronTree8.listNeuron.size(); i++){
        app2NeuronTree8.listNeuron[i].x *= 2;
        app2NeuronTree8.listNeuron[i].y *= 2;
        app2NeuronTree8.listNeuron[i].z *= 2;
        app2NeuronTree8.listNeuron[i].type = 9;
    }
    QString swcPath8 = imagePath + "_downSampleImage_2_rotateZ_" + QString::number(angle) + ".swc";
    writeSWC_file(swcPath8,app2NeuronTree8);

    //consensus
    trees.push_back(app2NeuronTree1);
    trees.push_back(app2NeuronTree2);
    trees.push_back(app2NeuronTree3);
    trees.push_back(app2NeuronTree4);
    trees.push_back(app2NeuronTree5);
    trees.push_back(app2NeuronTree6);
    trees.push_back(app2NeuronTree7);
    trees.push_back(app2NeuronTree8);

    m.x *= 2, m.y *= 2, m.z *= 2;
    NeuronTree consensusTree = consensus(trees,image,m,callback);

    QString consensusSWCPath = image +"_consensus.swc";
    writeSWC_file(consensusSWCPath,consensusTree);

    if(image){
        delete image;
        image = 0;
    }

    if(downSampleImage){
        delete downSampleImage;
        downSampleImage = 0;
    }

    return true;
}









