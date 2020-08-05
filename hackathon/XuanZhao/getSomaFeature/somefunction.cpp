#include "somefunction.h"


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
