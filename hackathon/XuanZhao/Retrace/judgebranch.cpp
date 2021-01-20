#include "judgebranch.h"

#include "app2.h"
//#include "my_surf_objs.cpp"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
//#include "../../../released_plugins/v3d_plugins/neuron_reliability_score/src/fastmarching_linker.h"
//#include "../../../released_plugins/v3d_plugins/neuron_reliability_score/src/fastmarching_tree.h"
//#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
#include "swc2mask.h"
#include "swc_convert.h"
#include "retracefunction.h"

#include "marker_radius.h"
#include "hierarchy_prune.h"

#include "pca.h"
#include "basic_memory.cpp"

#include <math.h>

#include <qmap.h>

//template<class T> bool swc2maskPackage(T * &outimg1d, vector<MyMarker * > & inswc, long sz0, long sz1, long sz2){
//    return swc2mask(outimg1d,inswc,sz0,sz1,sz2);
//}



bool keyPoint::meanShift(unsigned char *pdata, V3DLONG *sz, int windowradius){
    BoundingBox box(0,0,0,sz[0],sz[1],sz[2]);
    if(!box.isInner(XYZ(x,y,z))){
        return false;
    }
    if(windowradius == 0){
        windowradius = r;
    }

    mean_shift_fun fun_obj;
    fun_obj.pushNewData<unsigned char>((unsigned char*)pdata, sz);
    V3DLONG possPoint = xyz2pos((V3DLONG)(x+0.5),(V3DLONG)(y+0.5),(V3DLONG)(z+0.5),sz[0],sz[0]*sz[1]);
    vector<float> massCenter = fun_obj.mean_shift_center_mass(possPoint,windowradius);

    x = massCenter[0];
    y = massCenter[1];
    z = massCenter[2];

    return true;
}


bool keyTree::initial(NeuronTree nt){
    int pointNum = nt.listNeuron.size();
    nt.hashNeuron.clear();
    for(int i=0; i<pointNum; i++){
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointNum,vector<V3DLONG>());

    for(int i=0; i<pointNum; i++){
        V3DLONG prt = nt.listNeuron[i].parent;
        if(prt != -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children[prtIndex].push_back(i);
        }
    }

    V3DLONG count = 1;
    QHash<int,int> k2Index = QHash<int,int>();
    QHash<int,int> index2K = QHash<int,int>();
    for(int i=0; i<pointNum; i++){
        if(children[i].size() != 1){
            keyPoint p;
            p.x = nt.listNeuron[i].x;
            p.y = nt.listNeuron[i].y;
            p.z = nt.listNeuron[i].z;
            p.r = nt.listNeuron[i].r;
            p.type = nt.listNeuron[i].type;
            if(children[i].size() == 0){
                p.isTip = true;
            }else {
                p.isTip = false;
            }
            p.n = count;
            k2Index.insert(p.n,i);
            index2K.insert(i,p.n);
            count++;
            listKeyPoint.push_back(p);
        }
    }

    for(int i=0; i<listKeyPoint.size(); i++){
        int index = k2Index.value(listKeyPoint[i].n);
        V3DLONG prt = nt.listNeuron[index].parent;
        if(prt != -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            while (children[prtIndex].size() == 1) {
                prt = nt.listNeuron[prtIndex].parent;
                if(prt != -1){
                    prtIndex = nt.hashNeuron.value(prt);
                }else {
//                    listKeyPoint[i].parent = index2K.value(prtIndex);
                    break;
                }
            }
            listKeyPoint[i].parent = index2K.value(prtIndex);
        }else {
            listKeyPoint[i].parent = -1;
        }
    }

    for(int i=0; i<listKeyPoint.size(); i++){
        hashKeyPoint.insert(listKeyPoint[i].n,i);
    }

    for(int i=0; i<listKeyPoint.size(); i++){
        int level = 0;
        int prt = listKeyPoint[i].parent;
        while(prt != -1){
            level++;
            int prtIndex = hashKeyPoint.value(prt);
            prt = listKeyPoint[prtIndex].parent;
        }
        listKeyPoint[i].level = level;
    }

    return true;
}

//void keyTree::getBranchFeature(unsigned char *pdata, long long *sz){

//    double imgAve,imgStd;
//    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
//    mean_and_std(pdata,tolSZ,imgAve,imgStd);
//    double td = imgStd<10 ? 10: imgStd;
//    double thres = imgAve + td;

//    QList<ImageMarker> markers;
//    for(int i=0; i<listKeyPoint.size(); i++){
//       listKeyPoint[i].meanShift(pdata,sz,5);
//        markers.push_back(ImageMarker(listKeyPoint[i].x+1,listKeyPoint[i].y+1,listKeyPoint[i].z+1));
//    }
//    writeMarker_file("D:\\shiftMarker.marker",markers);

//    for(int i=0; i<listKeyPoint.size(); i++){
//        if(listKeyPoint[i].parent == -1){
//            continue;
//        }else {
//            vector<MyMarker*> outSWC = vector<MyMarker*>();
//            MyMarker p1(listKeyPoint[i].x,listKeyPoint[i].y,listKeyPoint[i].z);
//            int prtIndex = hashKeyPoint.value(listKeyPoint[i].parent);
//            MyMarker p2(listKeyPoint[prtIndex].x,listKeyPoint[prtIndex].y,listKeyPoint[prtIndex].z);
//            qDebug()<<"----------------start to fastmarching_linker-------------";
//            bool result = fastmarching_linker(p1,p2,pdata,outSWC,sz[0],sz[1],sz[2]);
//            qDebug()<<"----------------fastmarching_linker end-------------";
//            int markerNum = outSWC.size();
//            for(int j=0; j<markerNum; j++){
//                outSWC[j]->radius = markerRadius(pdata,sz,outSWC[j]->x,outSWC[j]->y,outSWC[j]->z,thres);
//            }
//            vector<float> lengthList = vector<float>(markerNum,0);
//            for(int j=1; j<markerNum; j++){
//                lengthList[j] = lengthList[j-1] + dist(*(outSWC[j-1]),*(outSWC[j]));
//            }
//            vector<bool> isInterMarkers = vector<bool>(markerNum,true);
//            for(int j=0; j<markerNum; j++){
//                if(lengthList[j]<outSWC[0]->radius){
//                    isInterMarkers[j] = false;
//                }else {
//                    break;
//                }
//            }
//            if(!listKeyPoint[i].isTip){
//                for(int j=markerNum-1; j>=0; j--){
//                    if((lengthList[markerNum-1] - lengthList[j])<outSWC[markerNum-1]->radius){
//                        isInterMarkers[j] = false;
//                    }else {
//                        break;
//                    }
//                }
//            }
//            vector<MyMarker*> interMarkers = vector<MyMarker*>();
//            for(int j=0; j<markerNum; j++){
//                if(isInterMarkers[j]){
//                    interMarkers.push_back(outSWC[j]);
//                }
//            }

//            for(int j=0; j<interMarkers.size(); j++){
//                interMarkers[j]->radius = 1;
//            }
//            unsigned char* maskR1 = 0;
//            swcTomask(maskR1,interMarkers,sz[0],sz[1],sz[2]);

//            for(int j=0; j<interMarkers.size(); j++){
//                interMarkers[j]->radius = 2;
//            }
//            unsigned char* maskR2 = 0;
//            swcTomask(maskR2,interMarkers,sz[0],sz[1],sz[2]);

//            for(int j=0; j<interMarkers.size(); j++){
//                interMarkers[j]->radius = 3;
//            }
//            unsigned char* maskR3 = 0;
//            swcTomask(maskR3,interMarkers,sz[0],sz[1],sz[2]);

//            float intensityR1 = 0, intensityR2 = 0, intensityR3 = 0;
//            int countR1 = 0, countR2 = 0, countR3 = 0;

//            for(int j=0; j<tolSZ; j++){
//                if(maskR1[j] > 0){
//                    intensityR1 += pdata[j];
//                    countR1++;
//                }
//                if(maskR2[j] > 0){
//                    intensityR2 += pdata[j];
//                    countR2++;
//                }
//                if(maskR3[j] > 0){
//                    intensityR3 += pdata[j];
//                    countR3++;
//                }
//            }

//            if(maskR1)
//                delete[] maskR1;
//            if(maskR2)
//                delete[] maskR2;
//            if(maskR3)
//                delete[] maskR3;

//            if(countR1>0)
//                intensityR1 /= countR1;
//            if(countR2>0)
//                intensityR2 /= countR2;
//            if(countR3>0)
//                intensityR3 /= countR3;

//            double dis = dist(*(outSWC[0]),*(outSWC[markerNum-1]));

//            qDebug()<<"n: "<<listKeyPoint[i].n<<" parent: "<<listKeyPoint[i].parent<<" level: "<<listKeyPoint[i].level
//                   <<" x: "<<listKeyPoint[i].x<<" y: "<<listKeyPoint[i].y<<" z: "<<listKeyPoint[i].z
//                  <<" r: "<<listKeyPoint[i].r<<" distance: "<<dis<<" length: "<<lengthList[markerNum-1]
//                 <<" intensityR1: "<<intensityR1<<" intensityR2: "<<intensityR2<<" intensityR3: "<<intensityR3
//                <<" R1/R2: "<<intensityR1/intensityR2<<" R1/R3: "<<intensityR1/intensityR3;

//        }
//    }
//}

vector<HierarchySegment*> splitHierarchySegment(HierarchySegment *segment){
    vector<MyMarker*> segMarkers = vector<MyMarker*>();
    segment->get_markers(segMarkers);
    int markerNum = segMarkers.size();
    vector<double> lengthList = vector<double>(markerNum,0);
    vector<double> angleList = vector<double>(markerNum,1);
    vector<int> flagList = vector<int>(markerNum,0);
    for(int i=1; i<markerNum; i++){
        lengthList[i] = lengthList[i-1] + dist(*(segMarkers[i]),*(segMarkers[i-1]));
    }
    int d = 10;
    int splitIndex  = 0;
    for(int i=0; i<markerNum; i++){
        if(lengthList[i] < d || (lengthList[markerNum-1] - lengthList[i]) < d){
            continue;
        }else {
            MyMarker* foreMarker = 0;
            MyMarker* backMarker = 0;
            for(int j=i-1; j>=splitIndex;j--){
                if((lengthList[i] -lengthList[j]) > d){
                    foreMarker = segMarkers[j];
                    break;
                }
            }
            for(int j=i+1; j<markerNum; j++){
                if((lengthList[j] - lengthList[i]) > d){
                    backMarker = segMarkers[j];
                    break;
                }
            }
            if(!foreMarker){
                foreMarker = segMarkers[splitIndex];
            }
            if(!backMarker){
                backMarker = segMarkers[markerNum-1];
            }

            if(foreMarker && backMarker){
                XYZ p = XYZ(segMarkers[i]->x,segMarkers[i]->y,segMarkers[i]->z);
                XYZ p1 = XYZ(foreMarker->x,foreMarker->y,foreMarker->z);
                XYZ p2 = XYZ(backMarker->x,backMarker->y,backMarker->z);
                double c = dot((p-p1),(p2-p))/(norm(p-p1)*norm(p2-p));
                angleList[i] = c;
            }

            if(angleList[i]<0.6){
                double angleMax = angleList[i];
                for(int k=i+1; k<markerNum; k++){
                    foreMarker = 0, backMarker = 0;
                    for(int j=k-1; j>=splitIndex;j--){
                        if((lengthList[k] -lengthList[j]) > d){
                            foreMarker = segMarkers[j];
                            break;
                        }
                    }
                    for(int j=k+1; j<markerNum; j++){
                        if((lengthList[j] - lengthList[k]) > d){
                            backMarker = segMarkers[j];
                            break;
                        }
                    }
                    if(!foreMarker){
                        foreMarker = segMarkers[splitIndex];
                    }
                    if(!backMarker){
                        backMarker = segMarkers[markerNum-1];
                    }

                    if(foreMarker && backMarker){
                        XYZ p = XYZ(segMarkers[i]->x,segMarkers[i]->y,segMarkers[i]->z);
                        XYZ p1 = XYZ(foreMarker->x,foreMarker->y,foreMarker->z);
                        XYZ p2 = XYZ(backMarker->x,backMarker->y,backMarker->z);
                        double c = dot((p-p1),(p2-p))/(norm(p-p1)*norm(p2-p));
                        angleList[k] = c;
                    }
                    if(angleList[k] < angleMax){
                        angleMax = angleList[k];
                    }else {
                        flagList[k] = 1;
                        splitIndex = k+1;
                        i = splitIndex+1;
                        break;
                    }
                }
            }

        }
    }

//    double angleMax = 1;
//    for(int i=0; i<markerNum; i++){
//        qDebug()<<"angle: "<<angleList[i];
//        if(angleList[i]<0.5){
//            angleMax = angleList[i];
//            for(int j=i+1; j++; j<markerNum){
//                if(angleList[i]<angleMax){
//                    angleMax = angleList[i];
//                }else {
//                     flagList[i] = 1;
//                     for(int k=j+1; k++; k<markerNum){
//                         if((lengthList[k] - lengthList[j]) > d){
//                             i = k;
//                             break;
//                         }
//                     }
//                     break;
//                }
//            }
//        }
//    }

    vector<HierarchySegment*> segs = vector<HierarchySegment*>();

    int fIndex = 0;
    for(int i=1; i<markerNum; i++){
        if(flagList[i] == 1){
            MyMarker* foreMarker = segMarkers[fIndex];
            MyMarker* backMarker = segMarkers[i];
            HierarchySegment* seg = new HierarchySegment(foreMarker,backMarker,lengthList[i] - lengthList[fIndex],segment->level);
            segs.push_back(seg);
            fIndex = i+1;
        }
    }
    MyMarker* foreMarker = segMarkers[fIndex];
    MyMarker* backMarker = segMarkers[markerNum-1];
    HierarchySegment* seg = new HierarchySegment(foreMarker,backMarker,lengthList[markerNum-1] - lengthList[fIndex],segment->level);
    segs.push_back(seg);

    return segs;
}

void getHierarchySegmentFeature(HierarchySegment* segment,unsigned char *pdata, long long *sz){
    vector<MyMarker*> segMarkers = vector<MyMarker*>();
    segment->get_markers(segMarkers);
    int markerNum = segMarkers.size();

    double imgAve,imgStd;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    mean_and_std(pdata,tolSZ,imgAve,imgStd);
    double td = imgStd<10 ? 10: imgStd;
    double thres = imgAve + td;

    for(int i=0; i<markerNum; i++){
        segMarkers[i]->radius = markerRadius(pdata,sz,segMarkers[i]->x,segMarkers[i]->y,segMarkers[i]->z,thres);
    }
    vector<double> lengthList = vector<double>(markerNum,0);
    vector<bool> isInterMarkers = vector<bool>(markerNum,true);
    for(int i=1; i<markerNum; i++){
        lengthList[i] = lengthList[i-1] + dist(*(segMarkers[i]),*(segMarkers[i-1]));
    }
    for(int i=0; i<markerNum; i++){
        if(lengthList[i] < segMarkers[0]->radius){
            isInterMarkers[i] = false;
        }else {
            break;
        }
    }
    for(int i=markerNum-1; i>=0; i--){
        if(lengthList[markerNum-1]-lengthList[i] < segMarkers[markerNum-1]->radius){
            isInterMarkers[i] = false;
        }else {
            break;
        }
    }

    vector<MyMarker*> interMarkers = vector<MyMarker*>();
    for(int j=0; j<markerNum; j++){
        if(isInterMarkers[j]){
            interMarkers.push_back(segMarkers[j]);
        }
    }

    for(int j=0; j<interMarkers.size(); j++){
        interMarkers[j]->radius = 1;
    }
    unsigned char* maskR1 = 0;
    swcTomask(maskR1,interMarkers,sz[0],sz[1],sz[2]);

    for(int j=0; j<interMarkers.size(); j++){
        interMarkers[j]->radius = 2;
    }
    unsigned char* maskR2 = 0;
    swcTomask(maskR2,interMarkers,sz[0],sz[1],sz[2]);

    for(int j=0; j<interMarkers.size(); j++){
        interMarkers[j]->radius = 5;
    }
    unsigned char* maskR3 = 0;
    swcTomask(maskR3,interMarkers,sz[0],sz[1],sz[2]);

    float intensityR1 = 0, intensityR2 = 0, intensityR3 = 0;
    int countR1 = 0, countR2 = 0, countR3 = 0;

    for(int j=0; j<tolSZ; j++){
        if(maskR1[j] > 0){
            intensityR1 += pdata[j];
            countR1++;
        }
        if(maskR2[j] > 0){
            intensityR2 += pdata[j];
            countR2++;
        }
        if(maskR3[j] > 0){
            intensityR3 += pdata[j];
            countR3++;
        }
    }

    if(maskR1)
        delete[] maskR1;
    if(maskR2)
        delete[] maskR2;
    if(maskR3)
        delete[] maskR3;

    if(countR1>0)
        intensityR1 /= countR1;
    if(countR2>0)
        intensityR2 /= countR2;
    if(countR3>0)
        intensityR3 /= countR3;

    double dis = dist(*(segMarkers[0]),*(segMarkers[markerNum-1]));

    qDebug()<<"thres: "<<(thres+td);
    qDebug()<<"type: "<<segMarkers[0]->type;
    qDebug()<<" level: "<<segment->level
          <<" r: "<<segMarkers[markerNum-1]->radius<<" distance: "<<dis<<" length: "<<lengthList[markerNum-1]
         <<" intensityR1: "<<intensityR1<<" intensityR2: "<<intensityR2<<" intensityR3: "<<intensityR3
        <<" R1/R2: "<<intensityR1/intensityR2<<" R1/R3: "<<intensityR1/intensityR3;

//    if(intensityR1/intensityR3 < 1.1 && intensityR1 < thres + td){
//        for(int i=0; i<markerNum; i++){
//            segMarkers[i]->type = 3;
//        }
//    }else {
//        for(int i=0; i<markerNum; i++){
//            segMarkers[i]->type = 2;
//        }
//    }

}

void pruneNeuronTree(QString imagePath, V3DPluginCallback2 &callback){
    V3DLONG sz[4] = {0,0,0,0};
    int dataType = 1;
    unsigned char* pdata = 0;
    simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);
//    normalImage(pdata,sz);
    unsigned char *** data3d = 0;
    if(!new3dpointer(data3d,sz[0],sz[1],sz[2],pdata)){
        cout<<"Fail to allocate memory"<<endl;
//        return false;
    }
    paraApp2 p2 = paraApp2();
    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;
    p2.bkg_thresh = -1;
    proc_app2(p2);

    sortSWC(p2.result);

    writeSWC_file((imagePath+".swc"),p2.result);

    vector<MyMarker*> inswc = vector<MyMarker*>();
    inswc = swc_convert(p2.result);
    vector<HierarchySegment*> segments = vector<HierarchySegment*>();
    swc2topo_segs(inswc,segments,0,pdata,sz[0],sz[1],sz[2]);

    int type = 2;
    for(int i=0; i<segments.size(); i++){
        vector<HierarchySegment*> inSegments = splitHierarchySegment(segments[i]);
        qDebug()<<"split size: "<<inSegments.size();
        for(int j=0; j<inSegments.size(); j++){

            vector<MyMarker*> markers = vector<MyMarker*>();
            inSegments[j]->get_markers(markers);
//            for(int k=0; k<markers.size(); k++){
//                markers[k]->type = type%7;
//            }
//            type++;
            qDebug()<<"type: "<<type;
            double d = getHierarchySegmentDirection(data3d,sz,inSegments[j]);
            qDebug()<<"d: "<<d;
            if(d<0.5){
                for(int k=0; k<markers.size(); k++){
                    markers[k]->type = 3;
                }
            }else{
                for(int k=0; k<markers.size(); k++){
                    markers[k]->type = 2;
                }
            }
//            getHierarchySegmentFeature(inSegments[j],pdata,sz);
        }
    }


//    vector<MyMarker*> outswc = vector<MyMarker*>();
//    for(int i=0; i<inswc.size(); i++){
//        inswc[i]->type = 6;
//        qDebug()<<"type: "<<inswc[i]->type;
//                //        outswc.push_back(inswc[i]);
//    }
    NeuronTree nt = swcConvert(inswc);
//    for(int i=0; i<nt.listNeuron.size(); i++){
//        qDebug()<<nt.listNeuron[i].type;
//    }

    writeSWC_file((imagePath + "_prune.swc"),nt);

    delete3dpointer(data3d,sz[0],sz[1],sz[2]);
    if(image){
        delete image;
        image = 0;
    }
}


void pruneNeuronTree(QString imagePath, QString swcPath, V3DPluginCallback2& callback){
    V3DLONG sz[4] = {0,0,0,0};
    int dataType = 1;
    unsigned char* pdata = 0;
    simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);

    unsigned char *** data3d = 0;
    if(!new3dpointer(data3d,sz[0],sz[1],sz[2],pdata)){
        cout<<"Fail to allocate memory"<<endl;
//        return false;
    }

    NeuronTree nt = readSWC_file(swcPath);

//    sortSWC(nt);

    vector<NeuronTree> trees = splitNeuronTree(nt);

    for(int t=0; t<trees.size(); t++){
        vector<MyMarker*> inswc = vector<MyMarker*>();
        inswc = swc_convert(trees[t]);
        vector<HierarchySegment*> segments = vector<HierarchySegment*>();
        swc2topo_segs(inswc,segments,0,pdata,sz[0],sz[1],sz[2]);

        int type = 2;
        for(int i=0; i<segments.size(); i++){
            vector<HierarchySegment*> inSegments = splitHierarchySegment(segments[i]);
            qDebug()<<"split size: "<<inSegments.size();
            for(int j=0; j<inSegments.size(); j++){
//                getHierarchySegmentFeature(inSegments[j],pdata,sz);
                vector<MyMarker*> markers = vector<MyMarker*>();
                inSegments[j]->get_markers(markers);
                for(int k=0; k<markers.size(); k++){
                    markers[k]->type = type%7;
                }
                type++;
                qDebug()<<"type: "<<type;
                getHierarchySegmentDirection(data3d,sz,inSegments[j]);
            }
        }
    }
    delete3dpointer(data3d,sz[0],sz[1],sz[2]);

}

NeuronTree pruneNeuronTree(Image4DSimple* image,NeuronTree &nt){
    unsigned char* pdata = image->getRawData();
    V3DLONG sz[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};

    unsigned char *** data3d = 0;
    if(!new3dpointer(data3d,sz[0],sz[1],sz[2],pdata)){
        cout<<"Fail to allocate memory"<<endl;
//        return false;
    }

    vector<MyMarker*> inswc = vector<MyMarker*>();
    inswc = swc_convert(nt);
    vector<HierarchySegment*> segments = vector<HierarchySegment*>();
    swc2topo_segs(inswc,segments,0,pdata,sz[0],sz[1],sz[2]);

    int type = 2;
    for(int i=0; i<segments.size(); i++){
        vector<HierarchySegment*> inSegments = splitHierarchySegment(segments[i]);
        qDebug()<<"split size: "<<inSegments.size();
        for(int j=0; j<inSegments.size(); j++){
//            getHierarchySegmentFeature(inSegments[j],pdata,sz);
            vector<MyMarker*> markers = vector<MyMarker*>();
            inSegments[j]->get_markers(markers);
//                for(int k=0; k<markers.size(); k++){
//                    markers[k]->type = type%7;
//                }
//                type++;
//                qDebug()<<"type: "<<type;
//            vector<MyMarker*> markers = vector<MyMarker*>();
//            inSegments[j]->get_markers(markers);
//            for(int k=0; k<markers.size(); k++){
//                markers[k]->type = type%7;
//            }
//            type++;
//            qDebug()<<"type: "<<type;
//            double d = getHierarchySegmentDirection(data3d,sz,inSegments[j]);
//            qDebug()<<"d: "<<d;
//            if(d<0.9){
//                for(int k=0; k<markers.size(); k++){
//                    markers[k]->type = 3;
//                }
//            }else{
//                for(int k=0; k<markers.size(); k++){
//                    markers[k]->type = 2;
//                }
//            }
            getHierarchySegmentAllPointDirection(data3d,sz,inSegments[j]);
            int count = 0;
            int startIndex = 0;
            int endIndex = 0;
            for(int k=0; k<markers.size(); k++){
                if(markers[k]->type != 7){
                    count++;
                    endIndex = k;
                }else {
                    if(count<5){
                        startIndex = k+1;
                        count = 0;
                    }else {
                        for(int kk = startIndex; kk<=endIndex; kk++){
                            markers[kk]->type = 4;
                        }
                        count = 0;
                    }
                }
            }
        }
    }

    qDebug()<<"-----end----";

    qDebug()<<"inswc size: "<<inswc.size();

    vector<MyMarker*> outswc = deleteSegmentByType(inswc,4);

    qDebug()<<"outswc size: "<<outswc.size();

    delete3dpointer(data3d,sz[0],sz[1],sz[2]);
    qDebug()<<"-----delete end-------";
    return swcConvert(outswc);

}

void pruneNeuronTree(V3DPluginCallback2& callback){
    v3dhandle cur = callback.currentImageWindow();
    Image4DSimple* image = callback.getImage(cur);
    paraApp2 p2 = paraApp2();
    p2.initialize(callback);
    unsigned char* pdata = p2.p4dImage->getRawData();
    V3DLONG sz[4] = {p2.p4dImage->getXDim(),p2.p4dImage->getYDim(),p2.p4dImage->getZDim(),p2.p4dImage->getCDim()};
//    normalImage(pdata,sz);
//    BinaryProcess(pdata,sz);
    p2.bkg_thresh = -1;
    proc_app2(p2);
    sortSWC(p2.result);
    NeuronTree nt = pruneNeuronTree(image,p2.result);
    nt.color = XYZW(0,0,0,0);
    callback.setSWC(cur,nt);
    callback.open3DWindow(cur);
    callback.getView3DControl(cur)->updateWithTriView();
}

vector<NeuronTree> splitNeuronTree(NeuronTree nt){
    int poinNum = nt.listNeuron.size();
    vector<vector<V3DLONG>> children = vector<vector<V3DLONG>>(poinNum,vector<V3DLONG>());
    vector<V3DLONG> roots = vector<V3DLONG>();
    for(V3DLONG i=0; i<poinNum; i++){
        V3DLONG prt = nt.listNeuron[i].parent;
        if(prt != -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children[prtIndex].push_back(i);
        }else {
            roots.push_back(i);
        }
    }
    vector<NeuronTree> trees = vector<NeuronTree>();
    for(int i=0; i<roots.size(); i++){
        vector<V3DLONG> queue = vector<V3DLONG>();
        queue.push_back(roots[i]);
        NeuronTree tree = NeuronTree();

        while (!queue.empty()) {
            V3DLONG tmp = queue.front();
            queue.erase(queue.begin());
            NeuronSWC s = nt.listNeuron[tmp];
            tree.listNeuron.push_back(s);
            for(int j=0; j<children[tmp].size(); j++){
                queue.push_back(children[tmp][j]);
            }
        }
        sortSWC(tree);
        trees.push_back(tree);
    }
    return trees;
}

double getHierarchySegmentDirection(unsigned char ***data3d, long long *sz, HierarchySegment *segment){
    qDebug()<<"------------in getHierarchySegmentDirection-------------------";
    double length = segment->length;


    qDebug()<<"------------in getlength-------------------";

    vector<MyMarker*> segMarkers = vector<MyMarker*>();
    segment->get_markers(segMarkers);
    qDebug()<<"------------in getMarker-------------------";
    int markerNum = segMarkers.size();

    if(length<=0 || markerNum<3){
        return 0;
    }

    qDebug()<<"length: "<<length;
    vector<double> lengthList = vector<double>(markerNum,0);
    for(int i=1; i<markerNum; i++){
        lengthList[i] = lengthList[i-1] + dist(*(segMarkers[i]),*(segMarkers[i-1]));
    }
    LocationSimple pt;
    int middleIndex = 0;
    for(int i=0; i<markerNum; i++){
        if(lengthList[i] > length/2){
            pt.x = segMarkers[i]->x;
            pt.y = segMarkers[i]->y;
            pt.z = segMarkers[i]->z;
            middleIndex = i;
            break;
        }
    }

    double pcaR = 5.0;
    MyMarker* p1 = 0;
    MyMarker* p2 = 0;
    for(int i=middleIndex; i>=0; i--){
        if((lengthList[middleIndex] - lengthList[i]) > pcaR){
            p1 = segMarkers[i];
            break;
        }
    }
    for(int i=middleIndex; i<markerNum; i++){
        if((lengthList[i] - lengthList[middleIndex]) > pcaR){
            p2 = segMarkers[i];
            break;
        }
    }

    if(!p1){
        p1 = segMarkers[0];
    }
    if(!p2){
        p2 = segMarkers[markerNum-1];
    }

    XYZ segDirection = XYZ(p2->x-p1->x,p2->y-p1->y,p2->z-p1->z);

    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];
    int datatype = 1;
    int channo = 1;

    pt.radius = pcaR;
    double sigma1,sigma2,sigma3;

    compute_rgn_stat_new(pt,data3d,channo,sz[0],sz[1],sz[2],sz[3],datatype,vec1,vec2,vec3,sigma1,sigma2,sigma3);
    QString path = QString("D:\\reTraceTest\\") + QString::number(pt.x) + "_" + QString::number(pt.y) + "_" + QString::number(pt.z) + ".marker";
    qDebug()<<path.toStdString().c_str();
//    showDirection(vec1,vec2,vec3,sigma1,sigma2,sigma3,pt,path);

    XYZ pcaDirection =  XYZ(vec1[0],vec1[1],vec1[2]);

    double dotDirection = dot(segDirection,pcaDirection);
    if(dotDirection<0){
        segDirection = XYZ(0,0,0)-segDirection;
        dotDirection = dot(segDirection,pcaDirection);
    }
    if(vec1){
        delete[] vec1;
        vec1 = 0;
    }
    if(vec2){
        delete[] vec2;
        vec2 = 0;
    }
    if(vec3){
        delete[] vec3;
        vec3 = 0;
    }
    return dotDirection/(norm(segDirection)*norm(pcaDirection));
}

vector<MyMarker*> deleteSegmentByType(vector<MyMarker*>& inswc, int type){

    int markerNum = inswc.size();

    qDebug()<<"marker num: "<<markerNum;

    QMap<MyMarker*,int> markerIndexMap = QMap<MyMarker*,int>();
    for(int i=0; i<markerNum; i++){
        markerIndexMap.insert(inswc[i],i);
    }
    qDebug()<<"map end";
    vector<vector<int>> children = vector<vector<int>>(markerNum,vector<int>());
    for(int i=0; i<markerNum; i++){
        if(inswc[i]->parent){
            int prtIndex = markerIndexMap[inswc[i]->parent];
            children[prtIndex].push_back(i);
        }
    }
    qDebug()<<"children end";
    vector<bool> isRemain = vector<bool>(markerNum,true);
    for(int i=0; i<markerNum; i++){
        if(inswc[i]->type == type){
            int curIndex = markerIndexMap[inswc[i]];
            vector<int> queue = vector<int>();
            queue.push_back(curIndex);
            while (!queue.empty()) {
                int tmp = queue.front();
                queue.erase(queue.begin());
                isRemain[tmp] = false;
                for(int j=0; j<children[tmp].size(); j++){
                    queue.push_back(children[tmp][j]);
                }
            }
        }
    }
    qDebug()<<"flag end";

    vector<MyMarker*> outswc = vector<MyMarker*>();

    for(int i=0; i<markerNum; i++){
        if(isRemain[i]){
            outswc.push_back(inswc[i]);
        }
    }

    return outswc;

}

XYZ getPointDirectionInSegment(vector<MyMarker *> &inswc, int i, int HW){
    XYZ p(0,0,0);
    int N = inswc.size();
    int kk;
    for(int k=1; k<=HW; k++){
        kk = i+k;
        if (kk<0) kk=0;
        if (kk>N-1) kk=N-1;
        p.x += inswc[kk]->x;
        p.y += inswc[kk]->y;
        p.z += inswc[kk]->z;

        kk = i-k;
        if (kk<0) kk=0;
        if (kk>N-1) kk=N-1;
        p.x -= inswc[kk]->x;
        p.y -= inswc[kk]->y;
        p.z -= inswc[kk]->z;
    }
    return p;
}

void getHierarchySegmentAllPointDirection(unsigned char*** data3d, V3DLONG* sz, HierarchySegment* segment){
    vector<MyMarker*> segMarkers = vector<MyMarker*>();
    segment->get_markers(segMarkers);

    double pcaR = 5.0;
    int HW = 5;
    LocationSimple pt;
    pt.radius = pcaR;
    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];
    int datatype = 1;
    int channo = 1;
    double sigma1,sigma2,sigma3;
    for(int i=0; i<segMarkers.size(); i++){
        XYZ pointDirection = getPointDirectionInSegment(segMarkers,i,HW);
        pt.x = segMarkers[i]->x;
        pt.y = segMarkers[i]->y;
        pt.z = segMarkers[i]->z;
        pt.radius = segMarkers[i]->radius + 2;
        compute_rgn_stat_new(pt,data3d,channo,sz[0],sz[1],sz[2],sz[3],datatype,vec1,vec2,vec3,sigma1,sigma2,sigma3);
        XYZ pcaDirection =  XYZ(vec1[0],vec1[1],vec1[2]);
        double dotDirection = dot(pointDirection,pcaDirection);
        if(dotDirection<0){
            pointDirection = XYZ(0,0,0)-pointDirection;
            dotDirection = dot(pointDirection,pcaDirection);
        }
        double d = dotDirection/(norm(pointDirection)*norm(pcaDirection));
        if(d>0.9){
            segMarkers[i]->type = 7;
        }else if (d>0.5) {
            segMarkers[i]->type = 3;
        }else {
            segMarkers[i]->type = 2;
        }
        segMarkers[i]->radius = sigma1/sigma2;    
    }
    if(vec1){
        delete[] vec1;
        vec1 = 0;
    }
    if(vec2){
        delete[] vec2;
        vec2 = 0;
    }
    if(vec3){
        delete[] vec3;
        vec3 = 0;
    }

}
















