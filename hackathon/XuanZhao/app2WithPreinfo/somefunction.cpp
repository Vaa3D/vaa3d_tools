#include "somefunction.h"

#include "app2.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
#include "marker_radius.h"
#include "swc_convert.h"

#include <fstream>
#include <sstream>

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

void getSWCMeanStd(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std){

    mean = std = 0;

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

}

bool app2WithPreinfo(QString dir, QString brainPath, QString outDir, double ratio, V3DPluginCallback2& callback){
    QFileInfoList files = QDir(dir).entryInfoList(QDir::Files);
    QString swcPath,apoPath;
    for(int i=0; i<files.size(); i++){
        qDebug()<<"suffix: "<<files[i].suffix();
        if(files[i].suffix() == "eswc"){
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
    size_t x0 = (int)(somaXYZ.x+0.5) - 512;
    size_t x1 = (int)(somaXYZ.x+0.5) + 512;
    size_t y0 = (int)(somaXYZ.y+0.5) - 512;
    size_t y1 = (int)(somaXYZ.y+0.5) + 512;
    size_t z0 = (int)(somaXYZ.z+0.5) - 256;
    size_t z1 = (int)(somaXYZ.z+0.5) + 256;

    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = dir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";
    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    NeuronTree backSWC,foreSWC;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x -= x0;
        s.y -= y0;
        s.z -= z0;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
        }
        if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
        }
    }

    double bmean,bstd,fmean,fstd;
    getSWCMeanStd(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd(pdata,sz,foreSWC,fmean,fstd);

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;

    if(ratio == 0){
        ratio = bstd/(bstd + fstd);
    }


    int app2Th = fmean*ratio + bmean*(1-ratio);
    if(ratio == -1){
        app2Th = bmean + 3*bstd;
    }
    if(ratio == -2){
        app2Th = fmean - 3*fstd;
    }

    qDebug()<<"app2Th: "<<app2Th;

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;
    p2.bkg_thresh = app2Th;
    p2.landmarks.push_back(LocationSimple(512,512,256));

    proc_app2(p2);
    sortSWC(p2.result);

    QString localSwcPath = dir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".swc";

    writeSWC_file(localSwcPath,p2.result);

    for(int i=0; i<p2.result.listNeuron.size(); i++){
        p2.result.listNeuron[i].x += x0;
        p2.result.listNeuron[i].y += y0;
        p2.result.listNeuron[i].z += z0;
        p2.result.listNeuron[i].type = 3;
    }

    QString outApoPath = outDir + "\\" + outDir.split("\\").back() + ".apo";
    QString outSwcPath = outDir + "\\" + outDir.split("\\").back() + ".eswc";
    QString outAnoPath = outDir + "\\" + outDir.split("\\").back() + ".ano";

    ofstream ano;
    ano.open(outAnoPath.toStdString().c_str(),ios::out);
    ano<<"APOFILE="<<outApoPath.toStdString().c_str()<<endl;
    ano<<"SWCFILE="<<outSwcPath.toStdString().c_str()<<endl;
    ano.close();
    writeAPO_file(outApoPath,markers);
    writeESWC_file(outSwcPath,p2.result);

    if(image){
        delete image;
        image = 0;
    }
    return true;
}

bool app2WithPreinfoForBatch(QString dir, QString brainPath, double ratio, V3DPluginCallback2& callback){
    QFileInfoList dirs = QDir(dir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList out = dir.split("\\");
    QString dirBaseName = out.back();
    out.pop_back();
    QString outDir = out.join("\\") + "\\" + dirBaseName + "_app2";
    if(!QDir().exists(outDir)){
        QDir().mkdir(outDir);
    }
    qDebug()<<outDir;
    for(int i=0; i<dirs.size(); i++){
        QString dirPath = dirs[i].absoluteFilePath();
        qDebug()<<i<<":"<<dirPath;
        QString id = dirPath.split("/").back();
        QString outDirPath = outDir + "\\" + id;
        qDebug()<<"id: "<<id;
        qDebug()<<"outdirpath: "<<outDirPath;
        if(!QDir().exists(outDirPath)){
            QDir().mkdir(outDirPath);
        }
        app2WithPreinfo(dirPath,brainPath,outDirPath,ratio,callback);

    }
    return true;
}














