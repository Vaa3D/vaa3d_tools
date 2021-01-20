#include "somefunction.h"

#include "app2.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
#include "marker_radius.h"
#include "swc_convert.h"

#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "memory.h"

#include <fstream>
#include <sstream>

#include "../../../vaa3d_tools/released_plugins/v3d_plugins/resample_swc/resampling.h"


bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result){
    return SortSWC(neurons,result,VOID,0);
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

double getSwcLength(NeuronTree& nt){
    double d = 0;
    nt.hashNeuron.clear();
    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }
    for(int i=0; i<nt.listNeuron.size(); i++){
        int par = nt.listNeuron[i].parent;
        if (par != -1){
            int prtIndex = nt.hashNeuron.value(par);
            XYZ p1 = XYZ(nt.listNeuron[i].x,nt.listNeuron[i].y,nt.listNeuron[i].z);
            XYZ p2 = XYZ(nt.listNeuron[prtIndex].x,nt.listNeuron[prtIndex].y,nt.listNeuron[prtIndex].z);
            d += dist_L2(p1,p2);
        }
    }
    return d;
}

void getSWCMeanStd(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std){

    mean = std = 0;

    qDebug()<<"listNeuron Size: "<<nt.listNeuron.size();

    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].r = 1;
    }
    unsigned char* mask = 0;

    vector<MyMarker*> markers = swc_convert(nt);
    swc2mask(mask,markers,sz[0],sz[1],sz[2]);
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    int count = 0;

    for(int i=0; i<tolSZ; i++){
        if(mask[i] == 255){
            mean += pdata[i];
            count++;
        }
    }

    qDebug()<<"count: "<<count;

    if(count>0){
        mean /= (double) count;
    }

    for(int i=0; i<tolSZ; i++){
        if(mask[i] == 255){
            std += (pdata[i]-mean)*(pdata[i]-mean);
        }
    }

    if(count>0){
        std = sqrt(std/(double)count);
    }

    if(mask){
        delete[] mask;
        mask = 0;
    }

}

void getSWCMeanStd2(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std){
    mean = std = 0;
    V3DLONG x,y,z;
    V3DLONG index;
    double rr = 10;
    vector<double> intensities;
    intensities.clear();
    for(int i=0; i<nt.listNeuron.size(); i++){
        x = nt.listNeuron[i].x + 0.5;
        y = nt.listNeuron[i].y + 0.5;
        z = nt.listNeuron[i].z + 0.5;
        rr = nt.listNeuron[i].r;

        if(x<0 || x>=sz[0] || y<0 || y>=sz[1] || z<0 || z>=sz[2]){
            continue;
        }
        int xs=qBound(0, int(x-rr), int(sz[0]-1)),
            xe=qBound(0, int(x+rr), int(sz[0]-1)),
            ys=qBound(0, int(y-rr), int(sz[1]-1)),
            ye=qBound(0, int(y+rr), int(sz[1]-1)),
            zs=qBound(0, int(z-rr), int(sz[2]-1)),
            ze=qBound(0, int(z+rr), int(sz[2]-1));
        double d=rr*rr, tt;
        double v = 0;
        V3DLONG n = 0;
        for (int kt=zs;kt<=ze;kt++)
        {
            tt = double(z)-kt;
            double d1 = tt*tt;
            for (int jt=ys;jt<=ye;jt++)
            {
                tt = double(y)-jt;
                double d2 = d1 + tt*tt;
                if (d2>rr)
                    continue;
                for (int it=xs;it<=xe;it++)
                {
                    tt = double(x)-it;
                    double d3 = d2+tt*tt;
                    if (d3>rr)
                        continue;
                    index = z*sz[0]*sz[1] + y*sz[0] + x;
                    v += pdata[index];
                    n++;
                }
            }
        }
        if(n<=0){
            intensities.push_back(-1);
        }else {
            intensities.push_back(v/n);
        }



    }
    int size = intensities.size();
    for(int i=0; i<size; i++){
        mean += intensities[i];
    }
    if(size>0)
        mean /= (double) size;
    for(int i=0; i<size; i++){
        std += (intensities[i]-mean)*(intensities[i]-mean);
    }
    if(size>0)
        std = sqrt(std/(double)size);

}



bool app2WithPreinfo(QString dir, QString brainPath, QString outDir, double ratio, int th, int resolutionTimes,
                     int imageFlag, double lower, double upper, int isMulti, double app2Length,
                     double contrastTh, double contrastRatio,
                     ofstream& csvFile, V3DPluginCallback2& callback){
#ifdef Q_OS_LINUX
    QString pathSeparator = "/";
#else
    QString pathSeparator = "\\";
#endif

    QFileInfoList files = QDir(dir).entryInfoList(QDir::Files);
    QString swcPath,apoPath;
    for(int i=0; i<files.size(); i++){
        qDebug()<<"suffix: "<<files[i].suffix();
        if(files[i].suffix() == "eswc" || files[i].suffix() == "swc"){
            swcPath = files[i].absoluteFilePath();
        }
        if(files[i].suffix() == "apo"){
            apoPath = files[i].absoluteFilePath();
        }
    }

    qDebug()<<swcPath;
    qDebug()<<apoPath;

    NeuronTree nt = readSWC_file(swcPath);
    QList<CellAPO> markers = readAPO_file(apoPath);
    if(markers.size()<1){
        qDebug()<<"marker size is smaller than 1";
        return false;
    }
    XYZ somaXYZ = XYZ(markers[0].x-1,markers[0].y-1,markers[0].z-1);

    V3DLONG x0,x1,y0,y1,z0,z1;
    if(markers.size() >= 1){
        x0 = (int)(somaXYZ.x/resolutionTimes+0.5) - 512/resolutionTimes;
        x1 = (int)(somaXYZ.x/resolutionTimes+0.5) + 512/resolutionTimes;
        y0 = (int)(somaXYZ.y/resolutionTimes+0.5) - 512/resolutionTimes;
        y1 = (int)(somaXYZ.y/resolutionTimes+0.5) + 512/resolutionTimes;
        z0 = (int)(somaXYZ.z/resolutionTimes+0.5) - 256/resolutionTimes;
        z1 = (int)(somaXYZ.z/resolutionTimes+0.5) + 256/resolutionTimes;
    }
//    else{
//        double dxy = 0, dz = 0;
//        for(int i=1; i<markers.size(); i++){
//            double tmpx = abs(markers[i].x - 1 - somaXYZ.x);
//            double tmpy = abs(markers[i].y - 1 - somaXYZ.y);
//            double tmpz = abs(markers[i].z - 1 - somaXYZ.z);

//            if(tmpx>dxy){
//                dxy = tmpx;
//            }
//            if(tmpy>dxy){
//                dxy = tmpy;
//            }
//            if(tmpz>dz){
//                dz = tmpz;
//            }
//        }
//        dxy += 20; dz += 20;

//        x0 = (int)((somaXYZ.x-dxy)/resolutionTimes+0.5);
//        x1 = (int)((somaXYZ.x+dxy)/resolutionTimes+0.5);
//        y0 = (int)((somaXYZ.y-dxy)/resolutionTimes+0.5);
//        y1 = (int)((somaXYZ.y+dxy)/resolutionTimes+0.5);
//        z0 = (int)((somaXYZ.z-dz)/resolutionTimes+0.5);
//        z1 = (int)((somaXYZ.z+dz)/resolutionTimes+0.5);
//    }

    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";

    if(QFile(imagePath).exists()){
        qDebug()<<imagePath<<" is exist!";
        return true;
    }

    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double imageMean,imageStd;
    if(imageFlag == 1){
        convertDataTo0_255(pdata,sz);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_0_255.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);

        mean_and_std(pdata,tolSZ,imageMean,imageStd);
        if(imageMean<contrastTh){
            changeContrast(pdata,sz,contrastRatio);
            QString contrastImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                    + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_ratio_" + QString::number(contrastRatio) + "_contrast.v3draw";
            simple_saveimage_wrapper(callback,contrastImagePath.toStdString().c_str(),pdata,sz,1);
        }
    }

    mean_and_std(pdata,tolSZ,imageMean,imageStd);



    NeuronTree backSWC,foreSWC,otherSWC;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x = s.x/resolutionTimes - x0;
        s.y = s.y/resolutionTimes - y0;
        s.z = s.z/resolutionTimes - z0;
        qDebug()<<"type: "<<s.type<<" xyz: "<<s.x<<" "<<s.y<<" "<<s.z;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
        }else if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
        }else{
            otherSWC.listNeuron.push_back(s);
        }
    }
    sortSWC(otherSWC);
    qDebug()<<"fore swc size:"<<foreSWC.listNeuron.size();
    qDebug()<<"back swc size:"<<backSWC.listNeuron.size();

//    NeuronTree backSwcR = resample(backSWC,0.5);
//    qDebug()<<"back swc size:"<<backSwcR.listNeuron.size();

    double bmean,bstd,fmean,fstd;
    getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);

    if(imageFlag == 2){
        convertDataPiecewise(pdata,sz,bmean,fmean,lower,upper,0);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_imageFlag2.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);
        getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
        getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    }else if (imageFlag == 3) {
        convertDataPiecewise(pdata,sz,bmean,fmean,lower,upper,1);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_imageFlag3.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);
        getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
        getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    }

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;

    if(ratio == 0){
        ratio = bstd/(bstd + fstd);
    }
    if(ratio == -4){
        ratio = bmean/(bmean+fmean);
    }

    int app2Th = fmean*ratio + bmean*(1-ratio);
    if(ratio == -1){
        double td = (imageStd>10)?imageStd:10;
        app2Th = imageMean + td*0.5;
    }
    if(ratio == -2){
        app2Th = th;
    }

    if(ratio == -3){
        app2Th = bmean + th;
    }


    qDebug()<<"app2Th: "<<app2Th;

    csvFile<<dir.split(pathSeparator).back().toStdString().c_str()<<','<<fmean<<','<<bmean<<','<<ratio<<','<<th<<','<<app2Th
             <<','<<fstd<<','<<bstd<<endl;

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    ImageMarker m = ImageMarker(markers[0].x/resolutionTimes-x0,markers[0].y/resolutionTimes-y0,markers[0].z/resolutionTimes-z0);
    m.color = XYZW(255,0,0,0);
    QList<ImageMarker> ms = QList<ImageMarker>();
    ms.push_back(m);

    mean_shift_fun fun_obj;
    fun_obj.pushNewData<unsigned char>((unsigned char*)pdata, sz);
    V3DLONG possPoint = xyz2pos((V3DLONG)(m.x-0.5),(V3DLONG)(m.y-0.5),(V3DLONG)(m.z-0.5),sz[0],sz[0]*sz[1]);
    vector<float> massCenter = fun_obj.mean_shift_center_mass(possPoint,5);

    ImageMarker shift_m = ImageMarker(massCenter[0]+1,massCenter[1]+1,massCenter[2]+1);
    shift_m.color = XYZW(0,255,0,0);
    ms.push_back(shift_m);

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;
    p2.bkg_thresh = app2Th;

    p2.length_thresh = app2Length;
//    p2.b_256cube = false;
    p2.landmarks.push_back(LocationSimple(shift_m.x,shift_m.y,shift_m.z));

    if(markers.size()>1){
        for(int i=1; i<markers.size(); i++){
            ImageMarker tmpm = ImageMarker(markers[i].x/resolutionTimes-x0,markers[i].y/resolutionTimes-y0,markers[i].z/resolutionTimes-z0);
            ms.push_back(tmpm);
            if(tmpm.x<1 || tmpm.x>sz[0] || tmpm.y<1 || tmpm.y>sz[1] || tmpm.z<1 || tmpm.z>sz[2])
                continue;
            p2.landmarks.push_back(LocationSimple(tmpm.x,tmpm.y,tmpm.z));
        }
    }
    QString localMarkerPath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".marker";

    writeMarker_file(localMarkerPath,ms);

    if(isMulti){
        proc_multiApp2(p2);
    }else {
        proc_app2(p2);
    }
    sortSWC(p2.result);

    QString localSwcPath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".swc";

    writeSWC_file(localSwcPath,p2.result);

    vector<NeuronTree> trees = vector<NeuronTree>();
    for(int i=0; i<p2.result.listNeuron.size(); i++){
        p2.result.listNeuron[i].type = 3;
    }
    trees.push_back(p2.result);
    trees.push_back(otherSWC);
    NeuronTree globalTree = mergeNeuronTrees(trees);

    for(int i=0; i<globalTree.listNeuron.size(); i++){
        globalTree.listNeuron[i].x = (globalTree.listNeuron[i].x+x0)*resolutionTimes;
        globalTree.listNeuron[i].y = (globalTree.listNeuron[i].y+y0)*resolutionTimes;
        globalTree.listNeuron[i].z = (globalTree.listNeuron[i].z+z0)*resolutionTimes;
//        globalTree.listNeuron[i].type = 3;
    }

    QString outApoPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + ".apo";
    QString outSwcPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + ".eswc";
    QString outAnoPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + ".ano";

    ofstream ano;
    ano.open(outAnoPath.toStdString().c_str(),ios::out);
    ano<<"APOFILE="<<(outDir.split(pathSeparator).back() + ".apo").toStdString().c_str()<<endl;
    ano<<"SWCFILE="<<(outDir.split(pathSeparator).back() + ".eswc").toStdString().c_str()<<endl;
    ano.close();
    writeAPO_file(outApoPath,markers);
    writeESWC_file(outSwcPath,globalTree);

    if(image){
        delete image;
        image = 0;
    }
    return true;
}

bool app2WithPreinfo2(QString dir, QString brainPath, QString outDir, ofstream& csvFile, int maxTh, float length, int resolutionTimes,
                      int imageFlag, double lower, double upper, int isMulti, double app2Length,
                      double contrastTh, double contrastRatio, V3DPluginCallback2& callback){
#ifdef Q_OS_LINUX
    QString pathSeparator = "/";
#else
    QString pathSeparator = "\\";
#endif

    QFileInfoList files = QDir(dir).entryInfoList(QDir::Files);
    QString swcPath = "",apoPath = "";
    for(int i=0; i<files.size(); i++){
        qDebug()<<"suffix: "<<files[i].suffix();
        if(files[i].suffix() == "eswc" || files[i].suffix() == "swc"){
            swcPath = files[i].absoluteFilePath();
        }
        if(files[i].suffix() == "apo"){
            apoPath = files[i].absoluteFilePath();
        }
    }

    if(swcPath == "" || apoPath == ""){
        return false;
    }

    qDebug()<<swcPath;
    qDebug()<<apoPath;

    NeuronTree nt = readSWC_file(swcPath);
    QList<CellAPO> markers = readAPO_file(apoPath);
    if(markers.size()<1){
        qDebug()<<"marker size is smaller than 1";
        return false;
    }
    XYZ somaXYZ = XYZ(markers[0].x-1,markers[0].y-1,markers[0].z-1);

    V3DLONG x0,x1,y0,y1,z0,z1;
    if(markers.size() >= 1){
        x0 = (int)(somaXYZ.x/resolutionTimes+0.5) - 512/resolutionTimes;
        x1 = (int)(somaXYZ.x/resolutionTimes+0.5) + 512/resolutionTimes;
        y0 = (int)(somaXYZ.y/resolutionTimes+0.5) - 512/resolutionTimes;
        y1 = (int)(somaXYZ.y/resolutionTimes+0.5) + 512/resolutionTimes;
        z0 = (int)(somaXYZ.z/resolutionTimes+0.5) - 256/resolutionTimes;
        z1 = (int)(somaXYZ.z/resolutionTimes+0.5) + 256/resolutionTimes;
    }
//    else{
//        double dxy = 0, dz = 0;
//        for(int i=1; i<markers.size(); i++){
//            double tmpx = abs(markers[i].x - 1 - somaXYZ.x);
//            double tmpy = abs(markers[i].y - 1 - somaXYZ.y);
//            double tmpz = abs(markers[i].z - 1 - somaXYZ.z);

//            if(tmpx>dxy){
//                dxy = tmpx;
//            }
//            if(tmpy>dxy){
//                dxy = tmpy;
//            }
//            if(tmpz>dz){
//                dz = tmpz;
//            }
//        }
//        dxy += 20; dz += 20;

//        x0 = (int)((somaXYZ.x-dxy)/resolutionTimes+0.5);
//        x1 = (int)((somaXYZ.x+dxy)/resolutionTimes+0.5);
//        y0 = (int)((somaXYZ.y-dxy)/resolutionTimes+0.5);
//        y1 = (int)((somaXYZ.y+dxy)/resolutionTimes+0.5);
//        z0 = (int)((somaXYZ.z-dz)/resolutionTimes+0.5);
//        z1 = (int)((somaXYZ.z+dz)/resolutionTimes+0.5);
//    }


    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";

    if(QFile(imagePath).exists()){
        qDebug()<<imagePath<<" is exist!";
        return true;
    }

    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double imageMean,imageStd;
    if(imageFlag == 1){
        convertDataTo0_255(pdata,sz);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_0_255.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);

        mean_and_std(pdata,tolSZ,imageMean,imageStd);
        if(imageMean<contrastTh){
            changeContrast(pdata,sz,contrastRatio);
            QString contrastImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                    + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_ratio_" + QString::number(contrastRatio) + "_contrast.v3draw";
            simple_saveimage_wrapper(callback,contrastImagePath.toStdString().c_str(),pdata,sz,1);
        }
    }

    int bCount = 0;
    int fCount = 0;
    int aCount = 0;
    NeuronTree backSWC,foreSWC,otherSWC;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x = s.x/resolutionTimes - x0;
        s.y = s.y/resolutionTimes - y0;
        s.z = s.z/resolutionTimes - z0;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
            bCount++;
        }else if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
            fCount++;
        }else{
            otherSWC.listNeuron.push_back(s);
        }
        aCount++;
    }
    sortSWC(otherSWC);

    qDebug()<<"bCount: "<<bCount<<" fCount: "<<fCount<<" aCount: "<<aCount;
    double bmean,bstd,fmean,fstd;
    getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    double ratio = -3;

    if(imageFlag == 2){
        convertDataPiecewise(pdata,sz,bmean,fmean,lower,upper,0);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_imageFlag2.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);
        getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
        getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    }else if (imageFlag == 3) {
        convertDataPiecewise(pdata,sz,bmean,fmean,lower,upper,1);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_imageFlag3.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);
        getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
        getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    }

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    ImageMarker m = ImageMarker(markers[0].x/resolutionTimes-x0,markers[0].y/resolutionTimes-y0,markers[0].z/resolutionTimes-z0);
    m.color = XYZW(255,0,0,0);
    QList<ImageMarker> ms = QList<ImageMarker>();
    ms.push_back(m);

    mean_shift_fun fun_obj;
    fun_obj.pushNewData<unsigned char>((unsigned char*)pdata, sz);
    V3DLONG possPoint = xyz2pos((V3DLONG)(m.x-0.5),(V3DLONG)(m.y-0.5),(V3DLONG)(m.z-0.5),sz[0],sz[0]*sz[1]);
    vector<float> massCenter = fun_obj.mean_shift_center_mass(possPoint,5);

    ImageMarker shift_m = ImageMarker(massCenter[0]+1,massCenter[1]+1,massCenter[2]+1);
    shift_m.color = XYZW(0,255,0,0);
    ms.push_back(shift_m);

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;

    p2.length_thresh = app2Length;
//    p2.b_256cube = false;
    p2.landmarks.push_back(LocationSimple(shift_m.x,shift_m.y,shift_m.z));

    if(markers.size()>1){
        for(int i=1; i<markers.size(); i++){
            ImageMarker tmpm = ImageMarker(markers[i].x/resolutionTimes-x0,markers[i].y/resolutionTimes-y0,markers[i].z/resolutionTimes-z0);
            ms.push_back(tmpm);
            if(tmpm.x<1 || tmpm.x>sz[0] || tmpm.y<1 || tmpm.y>sz[1] || tmpm.z<1 || tmpm.z>sz[2])
                continue;
            p2.landmarks.push_back(LocationSimple(tmpm.x,tmpm.y,tmpm.z));
        }
    }
    QString localMarkerPath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".marker";

    writeMarker_file(localMarkerPath,ms);

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;
    int app2Th;
    for(int th=maxTh; ; th--){
        app2Th = bmean + th;
        qDebug()<<"app2Th: "<<app2Th;

        if(app2Th<0){
            break;
        }


        p2.bkg_thresh = app2Th;
        if(isMulti){
            proc_multiApp2(p2);
        }else {
            proc_app2(p2);
        }
        sortSWC(p2.result);




        if(p2.result.listNeuron.isEmpty()){
            continue;
        }

        QString localSwcPath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_" + QString::number(th) +".swc";

        writeSWC_file(localSwcPath,p2.result);



        double swcLength = getSwcLength(p2.result);
        if(swcLength>length){

            csvFile<<dir.split(pathSeparator).back().toStdString().c_str()<<','<<fmean<<','<<bmean<<','<<ratio<<','<<th<<','<<app2Th
                     <<','<<fstd<<','<<bstd<<endl;

            vector<NeuronTree> trees = vector<NeuronTree>();
            trees.clear();
            for(int i=0; i<p2.result.listNeuron.size(); i++){
                p2.result.listNeuron[i].type = 3;
            }
            trees.push_back(p2.result);
            trees.push_back(otherSWC);
            NeuronTree globalTree = mergeNeuronTrees(trees);

            for(int i=0; i<globalTree.listNeuron.size(); i++){
                globalTree.listNeuron[i].x = (globalTree.listNeuron[i].x+x0)*resolutionTimes;
                globalTree.listNeuron[i].y = (globalTree.listNeuron[i].y+y0)*resolutionTimes;
                globalTree.listNeuron[i].z = (globalTree.listNeuron[i].z+z0)*resolutionTimes;
            }

            QString outApoPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".apo";
            QString outSwcPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".eswc";
            QString outAnoPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".ano";

            ofstream ano;
            ano.open(outAnoPath.toStdString().c_str(),ios::out);
            ano<<"APOFILE="<<(outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".apo").toStdString().c_str()<<endl;
            ano<<"SWCFILE="<<(outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".eswc").toStdString().c_str()<<endl;
            ano.close();
            writeAPO_file(outApoPath,markers);
            writeESWC_file(outSwcPath,globalTree);

            qDebug()<<"the out size is too big";
            break;
        }
    }

    if(image){
        delete image;
        image = 0;
    }
    return true;
}

bool app2WithPreinfo3(QString dir, QString brainPath, QString outDir, ofstream &csvFile, int maxTh, int minTh,
                      int resolutionTimes, int imageFlag, double lower, double upper,
                      int isMulti, double app2Length,
                      double contrastTh, double contrastRatio, V3DPluginCallback2 &callback){
#ifdef Q_OS_LINUX
    QString pathSeparator = "/";
#else
    QString pathSeparator = "\\";
#endif

    QFileInfoList files = QDir(dir).entryInfoList(QDir::Files);
    QString swcPath = "",apoPath = "";
    for(int i=0; i<files.size(); i++){
        qDebug()<<"suffix: "<<files[i].suffix();
        if(files[i].suffix() == "eswc" || files[i].suffix() == "swc"){
            swcPath = files[i].absoluteFilePath();
        }
        if(files[i].suffix() == "apo"){
            apoPath = files[i].absoluteFilePath();
        }
    }

    if(swcPath == "" || apoPath == ""){
        return false;
    }

    qDebug()<<swcPath;
    qDebug()<<apoPath;

    NeuronTree nt = readSWC_file(swcPath);
    QList<CellAPO> markers = readAPO_file(apoPath);
    if(markers.size()<1){
        qDebug()<<"marker size is smaller than 1";
        return false;
    }
    XYZ somaXYZ = XYZ(markers[0].x-1,markers[0].y-1,markers[0].z-1);
    V3DLONG x0,x1,y0,y1,z0,z1;
    if(markers.size() >= 1){
        x0 = (int)(somaXYZ.x/resolutionTimes+0.5) - 512/resolutionTimes;
        x1 = (int)(somaXYZ.x/resolutionTimes+0.5) + 512/resolutionTimes;
        y0 = (int)(somaXYZ.y/resolutionTimes+0.5) - 512/resolutionTimes;
        y1 = (int)(somaXYZ.y/resolutionTimes+0.5) + 512/resolutionTimes;
        z0 = (int)(somaXYZ.z/resolutionTimes+0.5) - 256/resolutionTimes;
        z1 = (int)(somaXYZ.z/resolutionTimes+0.5) + 256/resolutionTimes;
    }
//    else{
//        double dxy = 0, dz = 0;
//        for(int i=1; i<markers.size(); i++){
//            double tmpx = abs(markers[i].x - 1 - somaXYZ.x);
//            double tmpy = abs(markers[i].y - 1 - somaXYZ.y);
//            double tmpz = abs(markers[i].z - 1 - somaXYZ.z);

//            if(tmpx>dxy){
//                dxy = tmpx;
//            }
//            if(tmpy>dxy){
//                dxy = tmpy;
//            }
//            if(tmpz>dz){
//                dz = tmpz;
//            }
//        }
//        dxy += 20; dz += 20;

//        x0 = (int)((somaXYZ.x-dxy)/resolutionTimes+0.5);
//        x1 = (int)((somaXYZ.x+dxy)/resolutionTimes+0.5);
//        y0 = (int)((somaXYZ.y-dxy)/resolutionTimes+0.5);
//        y1 = (int)((somaXYZ.y+dxy)/resolutionTimes+0.5);
//        z0 = (int)((somaXYZ.z-dz)/resolutionTimes+0.5);
//        z1 = (int)((somaXYZ.z+dz)/resolutionTimes+0.5);
//    }

    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";

    if(QFile(imagePath).exists()){
        qDebug()<<imagePath<<" is exist!";
        return true;
    }

    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double imageMean,imageStd;

    if(imageFlag == 1){
        convertDataTo0_255(pdata,sz);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_0_255.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);

        mean_and_std(pdata,tolSZ,imageMean,imageStd);
        if(imageMean<contrastTh){
            changeContrast(pdata,sz,contrastRatio);
            QString contrastImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                    + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_ratio_" + QString::number(contrastRatio) + "_contrast.v3draw";
            simple_saveimage_wrapper(callback,contrastImagePath.toStdString().c_str(),pdata,sz,1);
        }
    }

    NeuronTree backSWC,foreSWC,otherSWC;
    int bCount = 0;
    int fCount = 0;
    int aCount = 0;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x = s.x/resolutionTimes - x0;
        s.y = s.y/resolutionTimes - y0;
        s.z = s.z/resolutionTimes - z0;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
            bCount++;
        }else if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
            fCount++;
        }else{
            otherSWC.listNeuron.push_back(s);
        }
        aCount++;
    }
    sortSWC(otherSWC);
    qDebug()<<"bCount: "<<bCount<<" fCount: "<<fCount<<" aCount: "<<aCount;

    double bmean,bstd,fmean,fstd;
    getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    double ratio = -3;

    if(imageFlag == 2){
        convertDataPiecewise(pdata,sz,bmean,fmean,lower,upper,0);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_imageFlag2.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);
        getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
        getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    }else if (imageFlag == 3) {
        convertDataPiecewise(pdata,sz,bmean,fmean,lower,upper,1);
        QString convertImagePath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_imageFlag3.v3draw";
        simple_saveimage_wrapper(callback,convertImagePath.toStdString().c_str(),pdata,sz,1);
        getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
        getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    }

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    ImageMarker m = ImageMarker(markers[0].x/resolutionTimes-x0,markers[0].y/resolutionTimes-y0,markers[0].z/resolutionTimes-z0);
    m.color = XYZW(255,0,0,0);
    QList<ImageMarker> ms = QList<ImageMarker>();
    ms.push_back(m);

    mean_shift_fun fun_obj;
    fun_obj.pushNewData<unsigned char>((unsigned char*)pdata, sz);
    V3DLONG possPoint = xyz2pos((V3DLONG)(m.x-0.5),(V3DLONG)(m.y-0.5),(V3DLONG)(m.z-0.5),sz[0],sz[0]*sz[1]);
    vector<float> massCenter = fun_obj.mean_shift_center_mass(possPoint,5);

    ImageMarker shift_m = ImageMarker(massCenter[0]+1,massCenter[1]+1,massCenter[2]+1);
    shift_m.color = XYZW(0,255,0,0);
    ms.push_back(shift_m);    

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;

    p2.length_thresh = app2Length;
//    p2.b_256cube = false;
    p2.landmarks.push_back(LocationSimple(shift_m.x,shift_m.y,shift_m.z));
    QString localMarkerPath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".marker";

    writeMarker_file(localMarkerPath,ms);

    if(markers.size()>1){
        for(int i=1; i<markers.size(); i++){
            ImageMarker tmpm = ImageMarker(markers[i].x/resolutionTimes-x0,markers[i].y/resolutionTimes-y0,markers[i].z/resolutionTimes-z0);
            ms.push_back(tmpm);
            if(tmpm.x<1 || tmpm.x>sz[0] || tmpm.y<1 || tmpm.y>sz[1] || tmpm.z<1 || tmpm.z>sz[2])
                continue;
            p2.landmarks.push_back(LocationSimple(tmpm.x,tmpm.y,tmpm.z));
        }
    }

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;
    int app2Th;
    for(int th=maxTh; th>=minTh; th--){
        app2Th = bmean + th;
        qDebug()<<"app2Th: "<<app2Th;

        if(app2Th<0){
            break;
        }


        p2.bkg_thresh = app2Th;
        if(isMulti){
            proc_multiApp2(p2);
        }else {
            proc_app2(p2);
        }
        sortSWC(p2.result);

        csvFile<<dir.split(pathSeparator).back().toStdString().c_str()<<','<<fmean<<','<<bmean<<','<<ratio<<','<<th<<','<<app2Th
              <<','<<fstd<<','<<bstd<<endl;

        if(p2.result.listNeuron.isEmpty()){
            continue;
        }

        QString localSwcPath = outDir + pathSeparator + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_" + QString::number(th) +".swc";

        writeSWC_file(localSwcPath,p2.result);

        vector<NeuronTree> trees = vector<NeuronTree>();
        trees.clear();
        for(int i=0; i<p2.result.listNeuron.size(); i++){
            p2.result.listNeuron[i].type = 3;
        }
        trees.push_back(p2.result);
        trees.push_back(otherSWC);
        NeuronTree globalTree = mergeNeuronTrees(trees);

        for(int i=0; i<globalTree.listNeuron.size(); i++){
            globalTree.listNeuron[i].x = (globalTree.listNeuron[i].x+x0)*resolutionTimes;
            globalTree.listNeuron[i].y = (globalTree.listNeuron[i].y+y0)*resolutionTimes;
            globalTree.listNeuron[i].z = (globalTree.listNeuron[i].z+z0)*resolutionTimes;
    //        globalTree.listNeuron[i].type = 3;
        }

        QString outApoPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".apo";
        QString outSwcPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".eswc";
        QString outAnoPath = outDir + pathSeparator + outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".ano";

        ofstream ano;
        ano.open(outAnoPath.toStdString().c_str(),ios::out);
        ano<<"APOFILE="<<(outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".apo").toStdString().c_str()<<endl;
        ano<<"SWCFILE="<<(outDir.split(pathSeparator).back() + "_" + QString::number(th) + ".eswc").toStdString().c_str()<<endl;
        ano.close();
        writeAPO_file(outApoPath,markers);
        writeESWC_file(outSwcPath,globalTree);
    }

    if(image){
        delete image;
        image = 0;
    }
    return true;
}

bool app2WithPreinfoForBatch(QString dir, QString brainPath, double ratio, int th, int resolutionTimes,
                             int imageFlag, double lower, double upper, int isMulti, double app2Length,
                             double contrastTh, double contrastRatio,
                             ofstream &csvFile, V3DPluginCallback2& callback){
#ifdef Q_OS_LINUX
    QString pathSeparator = "/";
#else
    QString pathSeparator = "\\";
#endif

    QFileInfoList dirs = QDir(dir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList out = dir.split(pathSeparator);
    QString dirBaseName = out.back();
    out.pop_back();
    QString outDir = out.join(pathSeparator) + pathSeparator + dirBaseName + "_app2_batch1_"
            + QString::number(ratio) + "_" + QString::number(th)+ "_"
            + QString::number(resolutionTimes) + "_" + QString::number(imageFlag)+ "_"
            + QString::number(lower) + "_" + QString::number(upper) + "_"
            + QString::number(isMulti) + "_" + QString::number(app2Length)+ "_"
            + QString::number(contrastTh) + "_" + QString::number(contrastRatio);
    if(!QDir().exists(outDir)){
        QDir().mkdir(outDir);
    }
    qDebug()<<outDir;
    for(int i=0; i<dirs.size(); i++){
        QString dirPath = dirs[i].absoluteFilePath();
        qDebug()<<i<<":"<<dirPath;
        QString id = dirPath.split("/").back();
        QString outDirPath = outDir + pathSeparator + id;
        qDebug()<<"id: "<<id;
        qDebug()<<"outdirpath: "<<outDirPath;
        if(!QDir().exists(outDirPath)){
            QDir().mkdir(outDirPath);
        }
        app2WithPreinfo(dirPath,brainPath,outDirPath,ratio,th,resolutionTimes,imageFlag,lower,upper,isMulti,app2Length,contrastTh,contrastRatio,csvFile,callback);

    }
    return true;
}

bool app2WithPreinfoForBatch2(QString dir, QString brainPath, ofstream &csvFile, int maxTh, float length,
                              int resolutionTimes, int imageFlag, double lower, double upper,
                              int isMulti, double app2Length,
                              double contrastTh, double contrastRatio, V3DPluginCallback2& callback){
#ifdef Q_OS_LINUX
    QString pathSeparator = "/";
#else
    QString pathSeparator = "\\";
#endif

    QFileInfoList dirs = QDir(dir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList out = dir.split(pathSeparator);
    QString dirBaseName = out.back();
    out.pop_back();
    QString outDir = out.join(pathSeparator) + pathSeparator + dirBaseName + "_app2_batch2_"
            + QString::number(maxTh) + "_" + QString::number(length)+ "_"
            + QString::number(resolutionTimes) + "_" + QString::number(imageFlag)+ "_"
            + QString::number(lower) + "_" + QString::number(upper) + "_"
            + QString::number(isMulti) + "_" + QString::number(app2Length)+ "_"
            + QString::number(contrastTh) + "_" + QString::number(contrastRatio);
    if(!QDir().exists(outDir)){
        QDir().mkdir(outDir);
    }
    qDebug()<<outDir;
    for(int i=0; i<dirs.size(); i++){
        QString dirPath = dirs[i].absoluteFilePath();
        qDebug()<<i<<":"<<dirPath;
        QString id = dirPath.split("/").back();
        QString outDirPath = outDir + pathSeparator + id;
        qDebug()<<"id: "<<id;
        qDebug()<<"outdirpath: "<<outDirPath;
        if(!QDir().exists(outDirPath)){
            QDir().mkdir(outDirPath);
        }
        app2WithPreinfo2(dirPath,brainPath,outDirPath,csvFile,maxTh,length,resolutionTimes,imageFlag,lower,upper,isMulti,app2Length,contrastTh,contrastRatio,callback);

    }
    return true;
}

bool app2WithPreinfoForBatch3(QString dir, QString brainPath, ofstream &csvFile, int maxTh, int minTh, int resolutionTimes,
                              int imageFlag, double lower, double upper, int isMulti, double app2Length,
                              double contrastTh, double contrastRatio, V3DPluginCallback2 &callback){
#ifdef Q_OS_LINUX
    QString pathSeparator = "/";
#else
    QString pathSeparator = "\\";
#endif

    QFileInfoList dirs = QDir(dir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList out = dir.split(pathSeparator);
    QString dirBaseName = out.back();
    out.pop_back();
    QString outDir = out.join(pathSeparator) + pathSeparator + dirBaseName + "_app2_batch3_"
            + QString::number(maxTh) + "_" + QString::number(minTh)+ "_"
            + QString::number(resolutionTimes) + "_" + QString::number(imageFlag)+ "_"
            + QString::number(lower) + "_" + QString::number(upper) + "_"
            + QString::number(isMulti) + "_" + QString::number(app2Length)+ "_"
            + QString::number(contrastTh) + "_" + QString::number(contrastRatio);
    if(!QDir().exists(outDir)){
        QDir().mkdir(outDir);
    }
    qDebug()<<outDir;
    for(int i=0; i<dirs.size(); i++){
        QString dirPath = dirs[i].absoluteFilePath();
        qDebug()<<i<<":"<<dirPath;
        QString id = dirPath.split("/").back();
        QString outDirPath = outDir + pathSeparator + id;
        qDebug()<<"id: "<<id;
        qDebug()<<"outdirpath: "<<outDirPath;
        if(!QDir().exists(outDirPath)){
            QDir().mkdir(outDirPath);
        }
        app2WithPreinfo3(dirPath,brainPath,outDirPath,csvFile,maxTh,minTh,resolutionTimes,imageFlag,lower,upper,isMulti,app2Length,contrastTh,contrastRatio,callback);

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

void convertDataTo0_255(unsigned char *data1d, long long *sz){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double iMin = INT_MAX;
    double iMax = 0;
    for(V3DLONG i=0; i<tolSZ; i++){
        if(data1d[i]>iMax){
            iMax = data1d[i];
        }
        if(data1d[i]<iMin){
            iMin = data1d[i];
        }
    }

    for(V3DLONG i =0; i<tolSZ; i++){
        double tmp = ((data1d[i]-iMin)/(iMax-iMin))*255;
        if(tmp>255) tmp = 255;
        data1d[i] = (unsigned char) tmp;
    }
}

void changeContrast(unsigned char* data1d, V3DLONG* sz, double contrastRatio){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    for(V3DLONG i=0; i<tolSZ; i++){
        double tmp = data1d[i]*contrastRatio;
        if(tmp>255)
            tmp = 255;
        if(tmp<0)
            tmp = 0;
        data1d[i] = (unsigned char)tmp;
    }
}

void convertDataPiecewise(unsigned char* data1d, V3DLONG* sz, double th1, double th2, double lower, double upper, int mode = 0){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    double iMin = INT_MAX;
    double iMax = 0;
    for(V3DLONG i=0; i<tolSZ; i++){
        if(data1d[i]>iMax){
            iMax = data1d[i];
        }
        if(data1d[i]<iMin){
            iMin = data1d[i];
        }
    }

    double rate0 = lower/(th1-iMin);
    double rate1 = (upper-lower)/(th2-th1);
    double rate2 = (255-upper)/(iMax-th2);

    if(mode == 0){
        for(V3DLONG i =0; i<tolSZ; i++){
            double tmp;
            if(data1d[i]<th1){
                tmp = (data1d[i]-iMin)*rate0;
                if(tmp>lower)
                    tmp = lower;
                data1d[i] = (unsigned char) tmp;
            }else if (data1d[i]<th2) {
                tmp = (data1d[i]-th1)*rate1 + lower;
                if(tmp>upper)
                    tmp = upper;
                data1d[i] = (unsigned char) tmp;
            }else {
                tmp = (data1d[i]-th2)*rate2 + upper;
                if(tmp>255)
                    tmp = 255;
                data1d[i] = (unsigned char) tmp;
            }
        }
    }else if(mode == 1){
        for(V3DLONG i =0; i<tolSZ; i++){
            double tmp;
            if(data1d[i]<th1){
                tmp = (data1d[i]-iMin)*rate0;
                if(tmp>lower)
                    tmp = lower;
                data1d[i] = (unsigned char) tmp;
            }else{
                tmp = (data1d[i]-th1)*rate1 + lower;
                if(tmp>255)
                    tmp = 255;
                data1d[i] = (unsigned char) tmp;
            }
        }
    }

}













