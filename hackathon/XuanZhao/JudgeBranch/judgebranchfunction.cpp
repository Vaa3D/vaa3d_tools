#include "judgebranchfunction.h"

RandomForest* train(RandomForest* rf, V3DPluginCallback2* callback){
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

    BranchTree bt;
    bt.initialize(nt);

    for(int i=0; i<bt.branchs.size(); i++){
        Branch& b =  bt.branchs.at(i);

        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        b.get_pointsIndex_of_branch(pointsIndex,nt);

        if(pointsIndex.size()<3)
            continue;

        QVector<float> feature = QVector<float>();
        b.caculateFeature(data1d,sz,nt);

        feature.append(b.level);
        feature.append(b.distance);
        feature.append(b.length);
        feature.append(b.intensityMean);
        feature.append(b.intensityStd);
        feature.append(b.intensityRationToLocal);
        feature.append(b.intensityRationToLocal);
        feature.append(b.gradientMean);
        feature.append(b.angleChangeMean);

        if(nt.listNeuron.at(pointsIndex.at(1)).type == 2){
            feature.append(1);
        }else if (nt.listNeuron.at(pointsIndex.at(1)).type == 3) {
            feature.append(2);
        }

        data.append(feature);
    }

    cout<<"-------------start to train randomforest-------"<<endl;

    int numTrees = 100;
    RandomForest* newRF = new RandomForest(numTrees,data);

    newRF->C = 2;
    newRF->M = 9;
    newRF->Ms = round(log(newRF->M)/log(2) + 1);
    newRF->start();

    cout<<"----------------train end------------------"<<endl;

//    QVector<DecisionTree> trees = rf->getTrees();
    if(!rf){
        return newRF;
    }else {
        RandomForest* result = new RandomForest();
        result->mergeRandomForest(rf,newRF,data);
        return result;
    }


}

void judgeBranch(RandomForest* rf, V3DPluginCallback2* callback){
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

//    NeuronTree nt = callback->getSWC(curwin);
    QList<NeuronTree>* nts =  callback->getHandleNeuronTrees_3DGlobalViewer(curwin);

    if(nts->size()<1 || nts->at(0).listNeuron.isEmpty()){
        QMessageBox::information(0, "", "The swc is empty!");
        return;
    }

    NeuronTree& nt = (*nts)[0];

    BranchTree bt;
    bt.initialize(nt);

    for(int i=0; i<bt.branchs.size(); i++){
        cout<<"i: "<<i<<endl;
        Branch* b =  &(bt.branchs[i]);
        splitBranch(b,data1d,sz,nt,rf);
    }

    cout<<"----------------set swc---------------"<<endl;
//    nt.listNeuron.clear();
//    callback->getHandleNeuronTrees_3DGlobalViewer()
//    callback->setSWC(curwin,nt);
//    callback->updateImageWindow(curwin);
    writeSWC_file("C:\\Users\\BrainCenter\\Desktop\\result.swc",nt);


}

void splitBranch(Branch* b, unsigned char* data1d, V3DLONG* sz, NeuronTree& nt, RandomForest* rf){
//    cout<<"iiiiiiiiiiiiiiiiiiiiiii"<<endl;

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    b->get_pointsIndex_of_branch(pointsIndex,nt);

    QVector<float> feature = QVector<float>();
    b->caculateFeature(data1d,sz,nt);



    feature.append(b->level);
    feature.append(b->distance);
    feature.append(b->length);
    feature.append(b->intensityMean);
    feature.append(b->intensityStd);
    feature.append(b->intensityRationToLocal);
    feature.append(b->intensityRationToLocal);
    feature.append(b->gradientMean);
    feature.append(b->angleChangeMean);
//    cout<<"ijijijijij"<<endl;

    int c = rf->evaluate(feature);

    cout<<"c: "<<c<<endl;

//    cout<<"jjjjjjjjjjjjjjjjjjjjjjjjjjj"<<endl;

    if(c == 0){
        for(int j=1; j<pointsIndex.size(); j++){
            nt.listNeuron[pointsIndex[j]].type = 2;
        }
    }else if(c == 1){
        if(b->length<=5 || pointsIndex.size()<=5){
            for(int j=1; j<pointsIndex.size(); j++){
                nt.listNeuron[pointsIndex[j]].type = 3;
            }
        }else {
            Branch* b1 = new Branch();
            Branch* b2 = new Branch();
//            cout<<"split----------"<<endl;
//            cout<<"points size: "<<pointsIndex.size()<<endl;
            b1->headPointIndex = pointsIndex.front();
            b1->endPointIndex = pointsIndex[(int)(pointsIndex.size()/2)];
            b1->level = b->level;
            b2->headPointIndex = pointsIndex[(int)(pointsIndex.size()/2)];
            b2->endPointIndex = pointsIndex.back();
            b2->level = b->level;

//            cout<<"===============----------"<<endl;

            splitBranch(b1,data1d,sz,nt,rf);
//            cout<<"split----1111111------"<<endl;
            splitBranch(b2,data1d,sz,nt,rf);
//            cout<<"split------22222222----"<<endl;
            if(b1)
                delete b1;
            if(b2)
                delete b2;
        }
    }
}
