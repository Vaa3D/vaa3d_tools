#include "axontrace.h"

#include "vn_app2.h"
#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
#include "swc_convert.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

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

NeuronTree imageBlock::cutBlockSWC(const NeuronTree &nt){
    NeuronTree cut = NeuronTree();

    BoundingBox block = BoundingBox(start_x,start_y,start_z,end_x,end_y,end_z);

    for(int i=0; i<nt.listNeuron.size(); i++){
        XYZ v = XYZ(nt.listNeuron[i].x,nt.listNeuron[i].y,nt.listNeuron[i].z);
        if(block.isInner(v)){
            cut.listNeuron.push_back(nt.listNeuron[i]);
        }
    }
    cut.hashNeuron.clear();
    for(int i=0; i<cut.listNeuron.size(); i++){
        cut.hashNeuron.insert(cut.listNeuron[i].n,i);
    }
    return cut;
}

void imageBlock::getTipBlock(NeuronTree &tree, BoundingBox box, vector<imageBlock> &blockList){

    imageBlock block = imageBlock(start_x+overlap,end_x-overlap,
                                  start_y+overlap,end_y-overlap,
                                  start_z+overlap,end_z-overlap);

//    NeuronTree nt = block.cutBlockSWC(tree);
//    getMainTree(nt);
    NeuronTree nt;
    nt.deepCopy(tree);

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
                getImageBlockByTip(newTip,blockList,512,box);
            }
        }
    }

    tree.listNeuron.clear();
    tree.listNeuron = nt.listNeuron;
    qDebug()<<"tree size:"<<tree.listNeuron.size();
    for(int i=0; i<tree.listNeuron.size(); i++){
        if(tree.listNeuron[i].parent == -1){
            tree.listNeuron[i].x = startMarker.x;
            tree.listNeuron[i].y = startMarker.y;
            tree.listNeuron[i].z = startMarker.z;
        }
    }

}

NeuronTree imageBlock::getMaskTree(){
    NeuronTree nt = cutBlockSWC(finalResult);
    NeuronTree maskTree = NeuronTree();
    int index, pIndex;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        if(s.x == startMarker.x && s.y == startMarker.y && s.z == startMarker.z){
            index = i;
            break;
        }
//        maskTree.listNeuron.push_back(s);
    }
    pIndex = nt.hashNeuron[nt.listNeuron[index].parent];
    qDebug()<<"index, pIndex: "<<index<<" "<<pIndex;

    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        if(i == index || i == pIndex){
            continue;
        }
        maskTree.listNeuron.push_back(s);
    }

    maskTree.hashNeuron.clear();
    for(int i=0; i<maskTree.listNeuron.size(); i++){
        maskTree.hashNeuron.insert(maskTree.listNeuron[i].n,i);
    }
    qDebug()<<"in getMaskTree maskTree size: "<<maskTree.listNeuron.size();
    return maskTree;
}

void imageBlock::getMaskImage(unsigned char *inimg, unsigned char *&outimg, V3DLONG *sz, double r){
    qDebug()<<" in getMaksImage ";
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    NeuronTree maskTree = getMaskTree();
    getLocalNeuronTree(maskTree);
    if(r>0){
        for(int i=0; i<maskTree.listNeuron.size(); i++){
            maskTree.listNeuron[i].r = r;
        }
    }
    vector<MyMarker*> maskMarkers = swc_convert(maskTree);
    qDebug()<<"maskMarkers size: "<<maskMarkers.size();
    unsigned char* maskFlag = 0;
    swc2mask(maskFlag,maskMarkers,sz[0],sz[1],sz[2]);
    outimg = new unsigned char[tolSZ];
    memset(outimg,0,sizeof(unsigned char)*tolSZ);
    for(int i=0; i<tolSZ; i++){
        if(maskFlag[i] == (unsigned char)255){
//            qDebug()<<i<<" : masked";
            outimg[i] = 0;
        }else {
            outimg[i] = inimg[i];
        }
    }
    if(maskFlag){
        delete[] maskFlag;
        maskFlag = 0;
    }
}

XYZ imageBlock::getLastDirection(){
    XYZ lastDirection;

    if(finalResult.listNeuron.size()<5)
        return XYZ(0,0,0);
    NeuronSWC lastPoint;
    for(int i=0; i<finalResult.listNeuron.size(); i++){
        NeuronSWC s = finalResult.listNeuron[i];
        if(s.x == startMarker.x && s.y == startMarker.y && s.z == startMarker.z){
            lastPoint = s;
        }
    }

    NeuronSWC tmp = lastPoint;
    vector<NeuronSWC> ns = vector<NeuronSWC>();
    int count = 0;
    while (tmp.parent != -1) {
        ns.push_back(tmp);
        V3DLONG prtIndex = finalResult.hashNeuron[tmp.parent];
        tmp = finalResult.listNeuron[prtIndex];
        if(count>20)
            break;
        count++;
    }

    XYZ cur = XYZ(0,0,0);
    XYZ cur_front = XYZ(0,0,0);
    int mcount = 0;
    for(int i=0; i<10; i++){
        if(i>=ns.size()){
            break;
        }
        cur = cur + XYZ(ns[i].x,ns[i].y,ns[i].z);
        mcount++;
    }
    cur = cur/XYZ(mcount,mcount,mcount);
    mcount = 0;
    for(int i=ns.size()-1; i>ns.size()-10; i-=2){
        if(i<0){
            break;
        }
        cur_front = cur_front + XYZ(ns[i].x,ns[i].y,ns[i].z);
        mcount++;
    }

    cur_front = cur_front/XYZ(mcount,mcount,mcount);
    lastDirection = cur - cur_front;

    return lastDirection;

}

NeuronTree imageBlock::getNeuronTree(QString brainPath, V3DPluginCallback2 &callback, double maskR){
    V3DLONG* in_sz = new V3DLONG[4];
    in_sz[0] = end_x - start_x;
    in_sz[1] = end_y - start_y;
    in_sz[2] = end_z - start_z;
    in_sz[3] = 1;
    V3DLONG tolSZ = in_sz[0]*in_sz[1]*in_sz[2];
    qDebug()<<"start_x: "<<start_x<<" end_x: "<<end_x
           <<"start_y: "<<start_y<<" end_y: "<<end_y
          <<"start_z: "<<start_z<<" end_z: "<<end_z;
    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString().c_str(),start_x,end_x,start_y,end_y,start_z,end_z);
    if(pdata == 0){
        return NeuronTree();
    }

    QDir saveImageDir = QDir("D:\\reTraceTest");
    if(!saveImageDir.exists()){
        saveImageDir.mkdir("D:\\reTraceTest");
    }

    QString saveImagePath = "D:\\reTraceTest\\" + QString::number(start_x) + "_" + QString::number(end_x) + "_" +
            QString::number(start_y) + "_" + QString::number(end_y) + "_" + QString::number(start_z) + "_" +
            QString::number(end_z) + ".v3draw";
    simple_saveimage_wrapper(callback,saveImagePath.toStdString().c_str(),pdata,in_sz,1);

    unsigned char* maskImage = pdata;
//    getMaskImage(pdata,maskImage,in_sz,maskR);
//    QString maskImagePath = "D:\\reTraceTest\\" + QString::number(start_x) + "_" + QString::number(end_x) + "_" +
//            QString::number(start_y) + "_" + QString::number(end_y) + "_" + QString::number(start_z) + "_" +
//            QString::number(end_z) + "_mask.v3draw";
//    simple_saveimage_wrapper(callback,maskImagePath.toStdString().c_str(),maskImage,in_sz,1);

    double imageMean, imageStd;
    mean_and_std(maskImage,tolSZ,imageMean,imageStd);
    qDebug()<<"maskImage meanStd: "<<imageMean<<" "<<imageStd;
    BinaryProcess(maskImage,in_sz);

    QString maskThresImagePath = "D:\\reTraceTest\\" + QString::number(start_x) + "_" + QString::number(end_x) + "_" +
            QString::number(start_y) + "_" + QString::number(end_y) + "_" + QString::number(start_z) + "_" +
            QString::number(end_z) + "_maskThres.v3draw";
    simple_saveimage_wrapper(callback,maskThresImagePath.toStdString().c_str(),maskImage,in_sz,1);

    Image4DSimple* app2Image = new Image4DSimple();
    app2Image->setData(maskImage,in_sz[0],in_sz[1],in_sz[2],in_sz[3],V3D_UINT8);
    app2Image->setFileName(maskThresImagePath.toStdString().c_str());
    PARA_APP2 p2 = PARA_APP2();
    p2.p4dImage = app2Image;
    p2.bkg_thresh = -1;
    p2.b_256cube = 0;
    p2.f_length = 60;
    p2.xc0 = 0;
    p2.yc0 = 0;
    p2.zc0 = 0;
    p2.xc1 = in_sz[0] - 1;
    p2.yc1 = in_sz[1] - 1;
    p2.zc1 = in_sz[2] - 1;
    p2.lastDirection = this->getLastDirection();

    LocationSimple m = LocationSimple(startMarker.x + 1 - start_x, startMarker.y + 1 - start_y, startMarker.z + 1 - start_z);
    QList<ImageMarker> ms;
    ms.push_back(ImageMarker(m.x,m.y,m.z));
    QString saveMarkerPath = "D:\\reTraceTest\\" + QString::number(start_x) + "_" + QString::number(end_x) + "_" +
            QString::number(start_y) + "_" + QString::number(end_y) + "_" + QString::number(start_z) + "_" + QString::number(end_z) + ".marker";
    writeMarker_file(saveMarkerPath,ms);

    NeuronTree foreTree = cutBlockSWC(finalResult);
    qDebug()<<"foreTree size"<<foreTree.listNeuron.size();
    getLocalNeuronTree(foreTree);
    vector<MyMarker*> maskMarkers = swc_convert(foreTree);
    qDebug()<<"marker size:"<<maskMarkers.size();
    MyMarker* p2Root = 0;
    for(int i=0; i<maskMarkers.size(); i++){
        MyMarker* s = maskMarkers[i];
        if(s->x == startMarker.x-start_x && s->y == startMarker.y-start_y && s->z == startMarker.z-start_z){
            p2Root = s;
            break;
        }
    }
    if(p2Root){
        qDebug()<<"p2Root xyz:"<<p2Root->x<<" "<<p2Root->y<<" "<<p2Root->z;
    }
    qDebug()<<"markers end";
    MyMarker* p2RootFore = 0;
    p2RootFore = p2Root;
    int c = 0;
    while (p2RootFore != 0) {
        p2RootFore = p2RootFore->parent;
        c++;
        if(c>30){
            break;
        }
    }
    qDebug()<<"p2RootFore end";
    p2.root = p2Root;
    p2.rootFore = p2RootFore;


    p2.landmarks.push_back(m);
    QString versionStr = "v0.001";
//    proc_app2_dynamic(callback,p2,versionStr);
    proc_app2_line(callback,p2,versionStr);

    NeuronTree app2NeuronTree = NeuronTree();
    app2NeuronTree.deepCopy(p2.result);

    for(int i=0; i<maskMarkers.size(); i++){
        if(maskMarkers[i]){
            delete maskMarkers[i];
            maskMarkers[i] = 0;
        }
    }

    if(app2Image){
        delete app2Image;
        app2Image = 0;
    }
    if(in_sz){
        delete[] in_sz;
        in_sz = 0;
    }
//    if(pdata){
//        delete[] pdata;
//        pdata = 0;
//    }

    return app2NeuronTree;
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

void getImageBlockByTip(LocationSimple tip, vector<imageBlock> &blockList, int block_size, BoundingBox box){
    imageBlock b = imageBlock();
    b.startMarker = tip;

    b.start_x = tip.x - block_size/2;
    b.end_x = b.start_x + block_size;
    b.start_y = tip.y - block_size/2;
    b.end_y = b.start_y + block_size;
    b.start_z = tip.z - block_size/4;
    b.end_z = b.start_z + block_size/2;

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

BoundingBox getGlobalBoundingBox(QString brainPath){
    qDebug()<<" in getGlobalBoundingBox ";
    BoundingBox box = BoundingBox();
    QString brain =  QFileInfo(brainPath).baseName();
    qDebug()<<"brain: "<<brain;
    QStringList ls = brain.split('x');
    int x0 = 0;
    int y0 = 0;
    int z0 = 0;
    int y1 = ls[0].split('(')[1].toInt();
    int x1 = ls[1].toInt();
    int z1 = ls[2].split(')')[0].toInt();

    box = BoundingBox(x0, y0, z0, x1, y1, z1);
    qDebug()<<"box block: "<<box.x0<<" "<<box.x1<<" "<<box.y0<<" "<<box.y1<<" "<<box.z0<<" "<<box.z1;
    return box;
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

NeuronTree ultratracerAxon(QString brainPath, BoundingBox box, NeuronTree ori, V3DPluginCallback2& callback){

    QDir saveImageDir = QDir("D:\\testDynamicTracing");
    if(!saveImageDir.exists()){
        saveImageDir.mkdir("D:\\testDynamicTracing");
    }

    int pointSize = ori.listNeuron.size();
    qDebug()<<"pointSize: "<<pointSize;

    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    for(V3DLONG j=0; j<pointSize; j++){
        V3DLONG prt = ori.listNeuron[j].parent;
        if(prt != -1){
            V3DLONG prtIndex = ori.hashNeuron.value(prt);
            children[prtIndex].push_back(j);
        }
    }
    LandmarkList tips;
    for(int i=0; i<pointSize; i++){
        if(children[i].size() == 0){
            LocationSimple tip = LocationSimple(ori.listNeuron[i].x,ori.listNeuron[i].y,ori.listNeuron[i].z);
            tips.push_back(tip);
        }
    }
    if(tips.size() != 1){
        qDebug()<<"tip size: "<<tips.size();
        return finalResult;
    }
    vector<imageBlock> blockList = vector<imageBlock>();
    getImageBlockByTip(tips[0],blockList,512,box);
    int bn = 1;
    double maskR = 0;

    while (!blockList.empty()) {
        imageBlock ib = blockList.front();
        blockList.erase(blockList.begin());
        if(bn == 1){
            maskR = 2;
        }else {
            maskR = -1;
        }
        NeuronTree l = ib.getNeuronTree(brainPath,callback,maskR);
        qDebug()<<"l size:"<<l.listNeuron.size();
        ib.getGlobelNeuronTree(l);
        ib.getTipBlock(l,box,blockList);
        mergeFinalResult(l);
        qDebug()<<"block size: "<<blockList.size()<<" bn: "<<bn;
        bn++;
    }

    return finalResult;

}

void ultratracerAxonTerafly(V3DPluginCallback2 &callback, QWidget *parent){
    QString brainPath = callback.getPathTeraFly();
    NeuronTree ori = callback.getSWCTeraFly();

    NeuronTree target,other;
    for(int i=0; i<ori.listNeuron.size(); i++){
        NeuronSWC s = ori.listNeuron[i];
        if(s.type == 2){
            target.listNeuron.push_back(s);
        }else{
            other.listNeuron.push_back(s);
        }
    }
    sortSWC(target);
    mergeFinalResult(target);
    BoundingBox box = getGlobalBoundingBox(callback);

    NeuronTree resultTreeTmp = ultratracerAxon(brainPath,box,target,callback);
    vector<NeuronTree> trees;
    trees.push_back(other);
    trees.push_back(resultTreeTmp);
    NeuronTree resultTree = mergeNeuronTrees(trees);
    callback.setSWCTeraFly(resultTree);
}

NeuronTree ultratracerAxonTerafly(QString brainPath, NeuronTree ori, V3DPluginCallback2 &callback){
//    sortSWC(ori);
    mergeFinalResult(ori);
    BoundingBox box = getGlobalBoundingBox(brainPath);
    NeuronTree resultTree = ultratracerAxon(brainPath,box,ori,callback);
    return resultTree;
}

