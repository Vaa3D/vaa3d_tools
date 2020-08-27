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
    vector<unsigned char> intensities;
    for(int i=0; i<nt.listNeuron.size(); i++){
        x = nt.listNeuron[i].x + 0.5;
        y = nt.listNeuron[i].y + 0.5;
        z = nt.listNeuron[i].z + 0.5;
        if(x<0 || x>=sz[0] || y<0 || y>=sz[1] || z<0 || z>=sz[2]){
            continue;
        }
        index = z*sz[0]*sz[1] + y*sz[0] + x;
        intensities.push_back(pdata[index]);
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



bool app2WithPreinfo(QString dir, QString brainPath, QString outDir, double ratio, int th, ofstream& csvFile, V3DPluginCallback2& callback){
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
    size_t x0 = (int)(somaXYZ.x+0.5) - 512;
    size_t x1 = (int)(somaXYZ.x+0.5) + 512;
    size_t y0 = (int)(somaXYZ.y+0.5) - 512;
    size_t y1 = (int)(somaXYZ.y+0.5) + 512;
    size_t z0 = (int)(somaXYZ.z+0.5) - 256;
    size_t z1 = (int)(somaXYZ.z+0.5) + 256;

    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = outDir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";

    if(QFile(imagePath).exists()){
        qDebug()<<imagePath<<" is exist!";
        return true;
    }

    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    NeuronTree backSWC,foreSWC;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x -= x0;
        s.y -= y0;
        s.z -= z0;
        qDebug()<<"type: "<<s.type<<" xyz: "<<s.x<<" "<<s.y<<" "<<s.z;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
        }
        if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
        }
    }
    qDebug()<<"fore swc size:"<<foreSWC.listNeuron.size();
    qDebug()<<"back swc size:"<<backSWC.listNeuron.size();

//    NeuronTree backSwcR = resample(backSWC,0.5);
//    qDebug()<<"back swc size:"<<backSwcR.listNeuron.size();

    double bmean,bstd,fmean,fstd;
    getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;

    if(ratio == 0){
        ratio = bstd/(bstd + fstd);
    }
    if(ratio == -4){
        ratio = bmean/(bmean+fmean);
    }

    int app2Th = fmean*ratio + bmean*(1-ratio);
    if(ratio == -1){
        app2Th = bmean + 3*bstd;
    }
    if(ratio == -2){
        app2Th = fmean - 3*fstd;
    }

    if(ratio == -3){
        app2Th = bmean + th;
    }


    qDebug()<<"app2Th: "<<app2Th;

    csvFile<<dir.split("\\").back().toStdString().c_str()<<','<<fmean<<','<<bmean<<','<<ratio<<','<<th<<','<<app2Th
             <<','<<fstd<<','<<bstd<<endl;

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    ImageMarker m = ImageMarker(512,512,256);
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

    QString localMarkerPath = outDir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".marker";

    writeMarker_file(localMarkerPath,ms);

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;
    p2.bkg_thresh = app2Th;
//    p2.b_256cube = false;
    p2.landmarks.push_back(LocationSimple(shift_m.x,shift_m.y,shift_m.z));

    proc_app2(p2);
    sortSWC(p2.result);

    QString localSwcPath = outDir + "\\" + QString::number(markers[0].x) + "_"
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
    ano<<"APOFILE="<<(outDir.split("\\").back() + ".apo").toStdString().c_str()<<endl;
    ano<<"SWCFILE="<<(outDir.split("\\").back() + ".eswc").toStdString().c_str()<<endl;
    ano.close();
    writeAPO_file(outApoPath,markers);
    writeESWC_file(outSwcPath,p2.result);

    if(image){
        delete image;
        image = 0;
    }
    return true;
}

bool app2WithPreinfo2(QString dir, QString brainPath, QString outDir, ofstream& csvFile, int maxTh, float length, V3DPluginCallback2& callback){
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
    size_t x0 = (int)(somaXYZ.x+0.5) - 512;
    size_t x1 = (int)(somaXYZ.x+0.5) + 512;
    size_t y0 = (int)(somaXYZ.y+0.5) - 512;
    size_t y1 = (int)(somaXYZ.y+0.5) + 512;
    size_t z0 = (int)(somaXYZ.z+0.5) - 256;
    size_t z1 = (int)(somaXYZ.z+0.5) + 256;

    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = outDir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";

    if(QFile(imagePath).exists()){
        qDebug()<<imagePath<<" is exist!";
        return true;
    }

    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    int bCount = 0;
    int fCount = 0;
    int aCount = 0;
    NeuronTree backSWC,foreSWC;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x -= x0;
        s.y -= y0;
        s.z -= z0;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
            bCount++;
        }
        if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
            fCount++;
        }
        aCount++;
    }

    qDebug()<<"bCount: "<<bCount<<" fCount: "<<fCount<<" aCount: "<<aCount;
    double bmean,bstd,fmean,fstd;
    getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    double ratio = -3;

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    ImageMarker m = ImageMarker(512,512,256);
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

    QString localMarkerPath = outDir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".marker";

    writeMarker_file(localMarkerPath,ms);

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;

//    p2.b_256cube = false;
    p2.landmarks.push_back(LocationSimple(shift_m.x,shift_m.y,shift_m.z));

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;
    int app2Th;
    for(int th=maxTh; ; th--){
        app2Th = bmean + th;
        qDebug()<<"app2Th: "<<app2Th;

        if(app2Th<0){
            break;
        }


        p2.bkg_thresh = app2Th;
        proc_app2(p2);
        sortSWC(p2.result);


        csvFile<<dir.split("\\").back().toStdString().c_str()<<','<<fmean<<','<<bmean<<','<<ratio<<','<<th<<','<<app2Th
                 <<','<<fstd<<','<<bstd<<endl;

        if(p2.result.listNeuron.isEmpty()){
            continue;
        }

        QString localSwcPath = outDir + "\\" + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_" + QString::number(th) +".swc";

        writeSWC_file(localSwcPath,p2.result);

        for(int i=0; i<p2.result.listNeuron.size(); i++){
            p2.result.listNeuron[i].x += x0;
            p2.result.listNeuron[i].y += y0;
            p2.result.listNeuron[i].z += z0;
            p2.result.listNeuron[i].type = 3;
        }

        QString outApoPath = outDir + "\\" + outDir.split("\\").back() + "_" + QString::number(th) + ".apo";
        QString outSwcPath = outDir + "\\" + outDir.split("\\").back() + "_" + QString::number(th) + ".eswc";
        QString outAnoPath = outDir + "\\" + outDir.split("\\").back() + "_" + QString::number(th) + ".ano";

        ofstream ano;
        ano.open(outAnoPath.toStdString().c_str(),ios::out);
        ano<<"APOFILE="<<(outDir.split("\\").back() + "_" + QString::number(th) + ".apo").toStdString().c_str()<<endl;
        ano<<"SWCFILE="<<(outDir.split("\\").back() + "_" + QString::number(th) + ".eswc").toStdString().c_str()<<endl;
        ano.close();
        writeAPO_file(outApoPath,markers);
        writeESWC_file(outSwcPath,p2.result);

        double swcLength = getSwcLength(p2.result);
        if(swcLength>length){
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

bool app2WithPreinfo3(QString dir, QString brainPath, QString outDir, ofstream &csvFile, int maxTh, int minTh, V3DPluginCallback2 &callback){
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
    size_t x0 = (int)(somaXYZ.x+0.5) - 512;
    size_t x1 = (int)(somaXYZ.x+0.5) + 512;
    size_t y0 = (int)(somaXYZ.y+0.5) - 512;
    size_t y1 = (int)(somaXYZ.y+0.5) + 512;
    size_t z0 = (int)(somaXYZ.z+0.5) - 256;
    size_t z1 = (int)(somaXYZ.z+0.5) + 256;

    unsigned char* pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {x1-x0,y1-y0,z1-z0,1};

    QString imagePath = outDir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".v3draw";

    if(QFile(imagePath).exists()){
        qDebug()<<imagePath<<" is exist!";
        return true;
    }

    simple_saveimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,1);

    NeuronTree backSWC,foreSWC;
    int bCount = 0;
    int fCount = 0;
    int aCount = 0;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.x -= x0;
        s.y -= y0;
        s.z -= z0;
        if(s.type == 2){
            backSWC.listNeuron.push_back(s);
            bCount++;
        }
        if(s.type == 3){
            foreSWC.listNeuron.push_back(s);
            fCount++;
        }
        aCount++;
    }
    qDebug()<<"bCount: "<<bCount<<" fCount: "<<fCount<<" aCount: "<<aCount;

    double bmean,bstd,fmean,fstd;
    getSWCMeanStd2(pdata,sz,backSWC,bmean,bstd);
    getSWCMeanStd2(pdata,sz,foreSWC,fmean,fstd);
    double ratio = -3;

    Image4DSimple* image = new Image4DSimple();
    image->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);

    ImageMarker m = ImageMarker(512,512,256);
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

    QString localMarkerPath = outDir + "\\" + QString::number(markers[0].x) + "_"
            + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + ".marker";

    writeMarker_file(localMarkerPath,ms);

    paraApp2 p2 = paraApp2();
    p2.p4dImage = image;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = sz[0] - 1;
    p2.yc1 = sz[1] - 1;
    p2.zc1 = sz[2] - 1;

//    p2.b_256cube = false;
    p2.landmarks.push_back(LocationSimple(shift_m.x,shift_m.y,shift_m.z));

    qDebug()<<"bmean: "<<bmean<<" bstd: "<<bstd<<" fmean: "<<fmean<<" fstd: "<<fstd;
    int app2Th;
    for(int th=maxTh; th>=minTh; th--){
        app2Th = bmean + th;
        qDebug()<<"app2Th: "<<app2Th;

        if(app2Th<0){
            break;
        }


        p2.bkg_thresh = app2Th;
        proc_app2(p2);
        sortSWC(p2.result);

        csvFile<<dir.split("\\").back().toStdString().c_str()<<','<<fmean<<','<<bmean<<','<<ratio<<','<<th<<','<<app2Th
              <<','<<fstd<<','<<bstd<<endl;

        if(p2.result.listNeuron.isEmpty()){
            continue;
        }

        QString localSwcPath = outDir + "\\" + QString::number(markers[0].x) + "_"
                + QString::number(markers[0].y) + "_" + QString::number(markers[0].z) + "_" + QString::number(th) +".swc";

        writeSWC_file(localSwcPath,p2.result);

        for(int i=0; i<p2.result.listNeuron.size(); i++){
            p2.result.listNeuron[i].x += x0;
            p2.result.listNeuron[i].y += y0;
            p2.result.listNeuron[i].z += z0;
            p2.result.listNeuron[i].type = 3;
        }

        QString outApoPath = outDir + "\\" + outDir.split("\\").back() + "_" + QString::number(th) + ".apo";
        QString outSwcPath = outDir + "\\" + outDir.split("\\").back() + "_" + QString::number(th) + ".eswc";
        QString outAnoPath = outDir + "\\" + outDir.split("\\").back() + "_" + QString::number(th) + ".ano";

        ofstream ano;
        ano.open(outAnoPath.toStdString().c_str(),ios::out);
        ano<<"APOFILE="<<(outDir.split("\\").back() + "_" + QString::number(th) + ".apo").toStdString().c_str()<<endl;
        ano<<"SWCFILE="<<(outDir.split("\\").back() + "_" + QString::number(th) + ".eswc").toStdString().c_str()<<endl;
        ano.close();
        writeAPO_file(outApoPath,markers);
        writeESWC_file(outSwcPath,p2.result);
    }

    if(image){
        delete image;
        image = 0;
    }
    return true;
}

bool app2WithPreinfoForBatch(QString dir, QString brainPath, double ratio, int th, ofstream &csvFile, V3DPluginCallback2& callback){
    QFileInfoList dirs = QDir(dir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList out = dir.split("\\");
    QString dirBaseName = out.back();
    out.pop_back();
    QString outDir = out.join("\\") + "\\" + dirBaseName + "_app2_" + QString::number(ratio) + "_" + QString::number(th);
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
        app2WithPreinfo(dirPath,brainPath,outDirPath,ratio,th,csvFile,callback);

    }
    return true;
}

bool app2WithPreinfoForBatch2(QString dir, QString brainPath, ofstream &csvFile, int maxTh, float length, V3DPluginCallback2& callback){
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
        app2WithPreinfo2(dirPath,brainPath,outDirPath,csvFile,maxTh,length,callback);

    }
    return true;
}

bool app2WithPreinfoForBatch3(QString dir, QString brainPath, ofstream &csvFile, int maxTh, int minTh, V3DPluginCallback2 &callback){
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
        app2WithPreinfo3(dirPath,brainPath,outDirPath,csvFile,maxTh,minTh,callback);

    }
    return true;
}














