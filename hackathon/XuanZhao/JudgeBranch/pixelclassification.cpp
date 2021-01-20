#include "pixelclassification.h"

#include "swc2mask.h"
#include "swc_convert.h"

//#include "branchtree.h"

QVector<QVector<float> > getPixelFeature(unsigned char *pdata, long long *sz){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    qDebug()<<"get features";
    QVector<QVector<float> > features = QVector<QVector<float> >(tolSZ,QVector<float>());
    for(int i=0; i<4; i++){
        float* gs = gaussianSmooth(pdata,sz,sigmas[i]);
        qDebug()<<i<<": get gaussianSmooth";
        float* ggm = gaussianGradientMagnitude(pdata,sz,sigmas[i]);
        qDebug()<<i<<": get gaussianGradientMagnitude";
        float* lg = laplacianOfGaussian(pdata,sz,sigmas[i]);
        qDebug()<<i<<": get laplacianOfGaussian";
        vector<vector<float> > hg = hessianOfGaussian(pdata,sz,sigmas[i]);
        qDebug()<<i<<": get hessianOfGaussian";
        for(int j=0; j<tolSZ; j++){
            features[j].push_back(gs[j]);
            features[j].push_back(ggm[j]);
            features[j].push_back(lg[j]);
            for(int k=0; k<6; k++){
                features[j].push_back(hg[j][k]);
            }
            features[j].push_back(0);
        }
        qDebug()<<i<<"start to free";
        if(gs){
            delete[] gs;
            gs = 0;
        }
        if(ggm){
            delete[] ggm;
            ggm = 0;
        }
        if(lg){
            delete[] lg;
            lg = 0;
        }
//        if(hg){
//            delete[] hg;
//            hg = 0;
//        }
        qDebug()<<i<<": free memory";
    }
    return features;
}

RandomForest* trainPixelClassification(RandomForest* rf, V3DPluginCallback2* callback){
    QVector<QVector<float> > data = QVector<QVector<float> >();

    v3dhandle curwin = callback->currentImageWindow();

    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return rf;
    }

    Image4DSimple* p4dImage = callback->getImage(curwin);

    if(!p4dImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return rf;
    }

    if(p4dImage->getDatatype() != V3D_UINT8){
        QMessageBox::information(0, "", "The image is not V3D_UINT8");
        return rf;
    }

    V3DLONG sz[4] = {p4dImage->getXDim(),p4dImage->getYDim(),p4dImage->getZDim(),p4dImage->getCDim()};
    if(sz[3] != 1){
        QMessageBox::information(0, "", "The image channel is not 1");
        return rf;
    }

    unsigned char* data1d = p4dImage->getRawData();

    NeuronTree nt = callback->getSWC(curwin);

    if(nt.listNeuron.isEmpty()){
        QMessageBox::information(0, "", "The swc is empty!");
        return rf;
    }

    NeuronTree foreTree, backTree;

    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        s.r = 1;
        if(s.type == 2){
            backTree.listNeuron.push_back(s);
        }else if (s.type == 3) {
            foreTree.listNeuron.push_back(s);
        }
    }


    if(backTree.listNeuron.isEmpty() || foreTree.listNeuron.isEmpty()){
        QMessageBox::information(0, "", "Please add label!");
        return rf;
    }

    qDebug()<<"get tree end";


    vector<MyMarker*> fmarkers = swc_convert(foreTree);
    vector<MyMarker*> bmarkers = swc_convert(backTree);
    unsigned char* maskf = 0;
    unsigned char* maskb = 0;
    swcTomask(maskf,fmarkers,sz[0],sz[1],sz[2]);
    swcTomask(maskb,bmarkers,sz[0],sz[1],sz[2]);

    qDebug()<<"mask end";

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    QVector<QVector<float> > features = getPixelFeature(data1d,sz);

    for(int i=0; i<tolSZ; i++){
        QVector<float>& feature = features[i];
        if(maskf[i] == 255){
            feature[feature.size()-1] = 1;
            data.append(feature);
        }else if (maskb[i] == 255) {
            feature[feature.size()-1] = 2;
            data.append(feature);
        }

    }

    int numTrees = 100;
    RandomForest* newRF = new RandomForest(numTrees,data);

    newRF->C = 2;
    newRF->M = features[0].size() - 1;
    newRF->Ms = round(log((double)newRF->M)/log(2.0) + 1);
    newRF->start();

    cout<<"----------------train end------------------"<<endl;

    if(maskb){
        delete[] maskb;
        maskb = 0;
    }
    if(maskf){
        delete[] maskf;
        maskf = 0;
    }

//    QVector<DecisionTree> trees = rf->getTrees();
    if(!rf){
        return newRF;
    }else {
        RandomForest* result = new RandomForest();
        result->mergeRandomForest(rf,newRF,data);
        return result;
    }
}

void getPixelClassificationResult(RandomForest* rf, V3DPluginCallback2* callback){
    v3dhandle curwin = callback->currentImageWindow();

    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4dImage = callback->getImage(curwin);

    if(!p4dImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    if(p4dImage->getDatatype() != V3D_UINT8){
        QMessageBox::information(0, "", "The image is not V3D_UINT8");
        return;
    }

    V3DLONG sz[4] = {p4dImage->getXDim(),p4dImage->getYDim(),p4dImage->getZDim(),p4dImage->getCDim()};
    if(sz[3] != 1){
        QMessageBox::information(0, "", "The image channel is not 1");
        return;
    }

    unsigned char* data1d = p4dImage->getRawData();

    QVector<QVector<float> > features = getPixelFeature(data1d,sz);
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    unsigned char* outData = new unsigned char[tolSZ];

    for(int i=0; i<tolSZ; i++){
        QVector<float> feature = features[i];
        int c = rf->evaluate(feature);
        if(c == 0){
            outData[i] = 255;
        }else if (c == 1) {
            outData[i] = 0;
        }
    }

    QString outImagePath = "D:\\reTraceTest\\result.v3draw";

    simple_saveimage_wrapper(*callback,outImagePath.toStdString().c_str(),outData,sz,1);
    if(outData){
        delete[] outData;
        outData = 0;
    }

}















