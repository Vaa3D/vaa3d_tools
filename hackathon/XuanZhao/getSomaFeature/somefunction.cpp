#include "somefunction.h"


double getZSliceBackMean(unsigned char* pdata, V3DLONG* sz, int z){
    double sliceBackMean = 0;
    int backCount = 0;
    double mean = 0;
    for(int y=0; y<=511; y++){
        for(int x=0; x<=511; x++){
            V3DLONG index = z*sz[0]*sz[1] +y*sz[0] +x;
            mean += pdata[index];
        }
    }
    mean /= (double)(sz[0]*sz[1]);

    for(int y=0; y<=511; y++){
        for(int x=0; x<=511; x++){
            V3DLONG index = z*sz[0]*sz[1] +y*sz[0] +x;
            if(pdata[index]<mean){
                sliceBackMean += pdata[index];
                backCount++;
            }
        }
    }
    sliceBackMean /= (double)backCount;

    return sliceBackMean;
}

vector<somaFeature> getBrainSomasFeature(QString brainPath, V3DPluginCallback2& callback){

    vector<somaFeature> somaFeatures = vector<somaFeature>();

    QFileInfoList brains = QDir(brainPath).entryInfoList(QDir::Files);

    vector<XYZ> xyzs;
    for(int i=0; i<brains.size(); i++){
        QString xyz = brains[i].completeBaseName();
        QString suffix = brains[i].suffix();
        if(suffix == "v3draw"){
            somaFeature f = somaFeature();
            f.name = xyz + ".v3draw";
            qDebug()<<xyz;
            float x = xyz.split("_")[0].toFloat();
            float y = xyz.split("_")[1].toFloat();
            float z = xyz.split("_")[2].toFloat();
//            qDebug()<<"x: "<<x<<" y: "<<y<<" z: "<<z;
            xyzs.push_back(XYZ(x,y,z));
            somaFeatures.push_back(f);
        }
    }

    for(int i=0; i<xyzs.size(); i++){
        somaFeature& f = somaFeatures[i];
        XYZ soma = xyzs[i];
        for(int j=0; j<xyzs.size(); j++){
            XYZ otherSoma = xyzs[j];
            if(i!=j){
                if(abs(soma.x-otherSoma.x)<128 && abs(soma.y-otherSoma.y)<128 && abs(soma.z-otherSoma.z)<64){
                    double d = dist_L2(soma,otherSoma);
                    f.anum++;
                    f.bnum++;
                    f.cnum++;
                    f.admean += d;
                    f.bdmean += d;
                    f.cdmean += d;
                }else if (abs(soma.x-otherSoma.x)<256 && abs(soma.y-otherSoma.y)<256 && abs(soma.z-otherSoma.z)<128) {
                    double d = dist_L2(soma,otherSoma);
                    f.bnum++;
                    f.cnum++;
                    f.bdmean += d;
                    f.cdmean += d;
                }else if (abs(soma.x-otherSoma.x)<512 && abs(soma.y-otherSoma.y)<512 && abs(soma.z-otherSoma.z)<256) {
                    double d = dist_L2(soma,otherSoma);
                    f.cnum++;
                    f.cdmean += d;
                }
            }
        }
        if(f.anum>0)
            f.admean /= f.anum;
        if(f.bnum>0)
            f.bdmean /= f.bnum;
        if(f.cnum>0)
            f.cdmean /= f.cnum;
        qDebug()<<"name: "<<f.name<<"anum: "<<f.anum<<" admean: "<<f.admean<<" bnum: "<<f.bnum<<" bdmean: "<<f.bdmean<<" cnum: "<<f.cnum<<" cdmean: "<<f.cdmean;

        QString imagePath = brainPath + "\\" + f.name;
        qDebug()<<imagePath;
        unsigned char* pdata = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);
        mean_and_std(pdata,sz[0]*sz[1]*sz[2],f.bIntensityMean,f.bIntensityStd);
        double td = (f.bIntensityStd>10) ? f.bIntensityStd : 10;
        f.th = f.bIntensityMean + 0.5*td;
        for(int x=253; x<=257; x++){
            for(int y=253; y<=257; y++){
                int z = 127;
                int index = z*sz[0]*sz[1] +y*sz[0] + x;
                f.somaIntensityMean += pdata[index];
            }
        }
        f.somaIntensityMean /= 25.0;

        qDebug()<<"bIntensityMean: "<<f.bIntensityMean<<" bIntensityStd: "<<f.bIntensityStd<<" th: "<<f.th<<" somaIntensityMean: "<<f.somaIntensityMean;
        if(pdata){
            delete[] pdata;
            pdata = 0;
        }

    }

    return somaFeatures;

}


vector<somaFeature> getBrainSomasFeature2(QString brainPath, V3DPluginCallback2& callback){
    vector<somaFeature> somaFeatures = vector<somaFeature>();

    QFileInfoList brains = QDir(brainPath).entryInfoList(QDir::Files);

    vector<XYZ> xyzs;
    for(int i=0; i<brains.size(); i++){
        QString xyz = brains[i].completeBaseName();
        QString suffix = brains[i].suffix();
        if(suffix == "v3draw"){
            somaFeature f = somaFeature();
            f.name = xyz + ".v3draw";
            qDebug()<<xyz;
            float x = xyz.split("_")[0].toFloat();
            float y = xyz.split("_")[1].toFloat();
            float z = xyz.split("_")[2].toFloat();
//            qDebug()<<"x: "<<x<<" y: "<<y<<" z: "<<z;
            xyzs.push_back(XYZ(x,y,z));
            somaFeatures.push_back(f);
        }
    }

    for(int i=0; i<xyzs.size(); i++){
        somaFeature& f = somaFeatures[i];
        XYZ soma = xyzs[i];
        for(int j=0; j<xyzs.size(); j++){
            XYZ otherSoma = xyzs[j];
            if(i!=j){
                if(abs(soma.x-otherSoma.x)<128 && abs(soma.y-otherSoma.y)<128 && abs(soma.z-otherSoma.z)<64){
                    double d = dist_L2(soma,otherSoma);
                    f.anum++;
                    f.bnum++;
                    f.cnum++;
                    f.admean += d;
                    f.bdmean += d;
                    f.cdmean += d;
                }else if (abs(soma.x-otherSoma.x)<256 && abs(soma.y-otherSoma.y)<256 && abs(soma.z-otherSoma.z)<128) {
                    double d = dist_L2(soma,otherSoma);
                    f.bnum++;
                    f.cnum++;
                    f.bdmean += d;
                    f.cdmean += d;
                }else if (abs(soma.x-otherSoma.x)<512 && abs(soma.y-otherSoma.y)<512 && abs(soma.z-otherSoma.z)<256) {
                    double d = dist_L2(soma,otherSoma);
                    f.cnum++;
                    f.cdmean += d;
                }
            }
        }
        if(f.anum>0)
            f.admean /= f.anum;
        if(f.bnum>0)
            f.bdmean /= f.bnum;
        if(f.cnum>0)
            f.cdmean /= f.cnum;
        qDebug()<<"name: "<<f.name<<"anum: "<<f.anum<<" admean: "<<f.admean<<" bnum: "<<f.bnum<<" bdmean: "<<f.bdmean<<" cnum: "<<f.cnum<<" cdmean: "<<f.cdmean;

        if(f.cnum == 0){
            f.multiType = 1;
        }else if (f.bnum == 0 && f.cnum > 0) {
            f.multiType = 2;
        }else if (f.anum == 0 && f.bnum > 0 && f.bnum <= 21) {
            f.multiType = 3;
        }else if (f.anum > 0 && f.anum <= 5 && f.bnum <= 21) {
            f.multiType = 4;
        }else if (f.anum > 5 || f.bnum > 21) {
            f.multiType = 5;
        }else {
            f.multiType = 6;
        }

        QString imagePath = brainPath + "\\" + f.name;
        qDebug()<<imagePath;
        unsigned char* pdata = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);
        mean_and_std(pdata,sz[0]*sz[1]*sz[2],f.bIntensityMean,f.bIntensityStd);
        double td = (f.bIntensityStd>10) ? f.bIntensityStd : 10;
        f.th = f.bIntensityMean + 0.5*td;
        for(int x=253; x<=257; x++){
            for(int y=253; y<=257; y++){
                int z = 127;
                int index = z*sz[0]*sz[1] +y*sz[0] + x;
                f.somaIntensityMean += pdata[index];
            }
        }
        f.somaIntensityMean /= 25.0;

        V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
        vector<int> intensityNum = vector<int>(256,0);
        for(int i=0; i<tolSZ; i++){
            intensityNum[pdata[i]]++;
        }
        int count = 0;
        bool percent1Flag = false;
        for(int i=255; i>=0; i--){
            count += intensityNum[i];
            if(count/(double)tolSZ > 0.01 && !percent1Flag){
                f.intensityPercent1 = i;
                percent1Flag = true;
            }
            if(count/(double)tolSZ > 0.05){
                f.intensityPercent5 = i;
                break;
            }
        }
        count = 0;
        for(int i=255; i>=f.th; i--){
            count += intensityNum[i];
        }
        f.thPercent = count/(double)tolSZ;

        count = 0;
        for(int i=0 ;i<tolSZ; i++){
            if(pdata[i]>f.bIntensityMean){
                f.bIntensityPartialStd += (pdata[i] - f.bIntensityMean)*(pdata[i] - f.bIntensityMean);
                count++;
            }
        }
        if(count>0){
            f.bIntensityPartialStd = sqrt(f.bIntensityPartialStd/(double)count);
        }

        double zSliceBackMean1 = getZSliceBackMean(pdata,sz,0);
        for(int z=1; z<=255; z++){
            double zSliceBackMean2 = getZSliceBackMean(pdata,sz,z);
            double diff = abs(zSliceBackMean2-zSliceBackMean1);
//            qDebug()<<z<<": "<<diff;
            if(diff>10){
                f.isLayered = 1;
            }
            zSliceBackMean1 = zSliceBackMean2;
        }

        qDebug()<<"bIntensityMean: "<<f.bIntensityMean<<" bIntensityStd: "<<f.bIntensityStd<<" th: "<<f.th<<" somaIntensityMean: "<<f.somaIntensityMean;
        if(pdata){
            delete[] pdata;
            pdata = 0;
        }

    }

    return somaFeatures;
}
