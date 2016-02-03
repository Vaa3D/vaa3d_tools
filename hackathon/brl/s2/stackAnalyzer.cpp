#include "stackAnalyzer.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


StackAnalyzer::StackAnalyzer(V3DPluginCallback2 &callback)
{
    cb = &callback;
}



void StackAnalyzer::loadScan(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation ){
    qDebug()<<"loadScan input: "<<latestString;
    qDebug()<<"overlap input:"<< QString::number(overlap);

    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!

    //latestString =QString("/data/mat/BRL/testData/ZSeries-01142016-0940-048/ZSeries-01142016-0940-048_Cycle00001_Ch2_000001.ome.tif");
    //LandmarkList inputRootList;
    qDebug()<<"loadScan input: "<<latestString;
    //   LocationSimple testroot;
    //   testroot.x = 10;testroot.y = 31;testroot.z = 101;inputRootList.push_back(testroot);
    //   testroot.x = 205;testroot.y = 111;testroot.z = 102;inputRootList.push_back(testroot);


    QFileInfo imageFileInfo = QFileInfo(latestString);
    if (imageFileInfo.isReadable()){
        v3dhandle newwin = cb->newImageWindow();
        Image4DSimple * pNewImage = cb->loadImage(latestString.toLatin1().data());
        QDir imageDir =  imageFileInfo.dir();
        QStringList filterList;
        filterList.append(QString("*Ch2*.tif"));
        imageDir.setNameFilters(filterList);
        QStringList fileList = imageDir.entryList();

        //get the parent dir and the list of ch1....ome.tif files
        //use this to id the number of images in the stack (in one channel?!)
        V3DLONG x = pNewImage->getXDim();
        V3DLONG y = pNewImage->getYDim();
        V3DLONG nFrames = fileList.length();

        V3DLONG tunits = x*y*nFrames;
        unsigned short int * total1dData = new unsigned short int [tunits];
        V3DLONG totalImageIndex = 0;
        for (int f=0; f<nFrames; f++){
            //qDebug()<<fileList[f];
            Image4DSimple * pNewImage = cb->loadImage(imageDir.absoluteFilePath(fileList[f]).toLatin1().data());
            if (pNewImage->valid()){
                unsigned short int * data1d = 0;
                data1d = new unsigned short int [x*y];
                data1d = (unsigned short int*)pNewImage->getRawData();
                for (V3DLONG i = 0; i< (x*y); i++){
                    total1dData[totalImageIndex]= data1d[i];
                    totalImageIndex++;
                }
            }else{
                qDebug()<<imageDir.absoluteFilePath(fileList[f])<<" failed!";
            }
        }

        Image4DSimple* total4DImage = new Image4DSimple;
        total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);

        //new code starts here:


        QDir saveDir = imageFileInfo.absoluteDir();//QDir("/Users/zhiz/Desktop/2016_02_01_Mon_20_28/"); // Zhi pick a directory here.
        saveDir.cdUp();
        int scanNumber = 0;
        QString swcString =   saveDir.absolutePath().append("/").append(QString::number(scanNumber)).append("test.swc");

        QString scanDataFileString = saveDir.absolutePath().append("/").append("scanData.txt");
        qDebug()<<scanDataFileString;
        QFile saveTextFile;
        saveTextFile.setFileName(scanDataFileString);// add currentScanFile
        if (!saveTextFile.isOpen()){
            if (!saveTextFile.open(QIODevice::Text|QIODevice::ReadWrite  )){
                qDebug()<<"unable to save file!";
                return;}     }
        QTextStream outputStream;
        outputStream.setDevice(&saveTextFile);
        total4DImage->setOriginX(tileLocation.x);
        total4DImage->setOriginY(tileLocation.y);
        qDebug()<<total4DImage->getOriginX();

        outputStream<<total4DImage->getOriginX()<<" "<<total4DImage->getOriginY()<<" "<<swcString<<"\n";
        PARA_APP2 p;
        p.is_gsdt = false;
        p.is_coverage_prune = true;
        p.is_break_accept = false;
        p.bkg_thresh = background;
        p.length_thresh = 5;
        p.cnn_type = 2;
        p.channel = 0;
        p.SR_ratio = 3.0/9.9;
        p.b_256cube = 1;
        p.b_RadiusFrom2D = true;
        p.b_resample = 1;
        p.b_intensity = 0;
        p.b_brightfiled = 0;
        p.b_menu = interrupt; //if set to be "true", v3d_msg window will show up.

        p.p4dImage = total4DImage;
        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;
        QString versionStr = "v2.621";

        qDebug()<<"starting app2";
        qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

        if(inputRootList.size() <1)
        {
            p.outswc_file =swcString;
            proc_app2(*cb, p, versionStr);
        }
        else
        {
            vector<MyMarker*> tileswc_file;
            for(int i = 0; i < inputRootList.size(); i++)
            {
                p.outswc_file =swcString + (QString::number(i)) + (".swc");
                LocationSimple RootNewLocation;
                RootNewLocation.x = inputRootList.at(i).x - p.p4dImage->getOriginX();
                RootNewLocation.y = inputRootList.at(i).y - p.p4dImage->getOriginY();
                RootNewLocation.z = inputRootList.at(i).z - p.p4dImage->getOriginZ();

                p.landmarks.push_back(RootNewLocation);
                proc_app2(*cb, p, versionStr);
                p.landmarks.clear();
                vector<MyMarker*> inputswc = readSWC_file(p.outswc_file.toStdString());
                qDebug()<<"ran app2";
                for(V3DLONG d = 0; d < inputswc.size(); d++)
                {
                    tileswc_file.push_back(inputswc[d]);
                }
            }
            saveSWC_file(swcString.toStdString().c_str(), tileswc_file);
        }

        NeuronTree nt;
        nt = readSWC_file(swcString);
        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        LandmarkList newTargetList;
        LandmarkList tip_left;
        LandmarkList tip_right;
        LandmarkList tip_up ;
        LandmarkList tip_down;
        QList<NeuronSWC> list = nt.listNeuron;
        for (V3DLONG i=0;i<list.size();i++)
        {
            if (childs[i].size()==0)
            {
                NeuronSWC curr = list.at(i);
                LocationSimple newTip;
                newTip.x = curr.x + p.p4dImage->getOriginX();
                newTip.y = curr.y + p.p4dImage->getOriginY();
                newTip.z = curr.z + p.p4dImage->getOriginZ();
                if( curr.x < 0.05* p.p4dImage->getXDim())
                {
                    tip_left.push_back(newTip);
                }else if (curr.x > 0.95 * p.p4dImage->getXDim())
                {
                    tip_right.push_back(newTip);
                }else if (curr.y < 0.05 * p.p4dImage->getYDim())
                {
                    tip_up.push_back(newTip);
                }else if (curr.y > 0.95*p.p4dImage->getYDim())
                {
                    tip_down.push_back(newTip);
                }
            }
        }

        QList<LandmarkList> newTipsList;
        LocationSimple newTarget;

        if(tip_left.size()>0)
        {
            newTipsList.push_back(tip_left);
            newTarget.x = -p.p4dImage->getXDim();
            newTarget.y = 0;
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_right.size()>0)
        {
            newTipsList.push_back(tip_right);
            newTarget.x = p.p4dImage->getXDim();
            newTarget.y = 0;
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_up.size()>0)
        {
            newTipsList.push_back(tip_up);
            newTarget.x = 0;
            newTarget.y = -p.p4dImage->getYDim();
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_down.size()>0)
        {
            newTipsList.push_back(tip_down);
            newTarget.x = 0;
            newTarget.y = p.p4dImage->getYDim();
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }

        if (!newTargetList.empty())
        {
            for (int i = 0; i<newTargetList.length(); i++)
            {
                newTargetList[i].x = (1.0-overlap)*newTargetList[i].x+p.p4dImage->getOriginX();
                newTargetList[i].y=(1.0-overlap)*newTargetList[i].y+p.p4dImage->getOriginY();
                newTargetList[i].z =(1.0-overlap)*newTargetList[i].z+p.p4dImage->getOriginZ();
            }
        }
        saveTextFile.close();
        cb->setImage(newwin, total4DImage);
        cb->open3DWindow(newwin);
        cb->setSWC(newwin,nt);
        LandmarkList imageLandmarks = newTargetList;
//        cb->setLandmark(newwin,newTargetList);
        RGBA8 myColor;
        myColor.r = 128;
        myColor.g = 200;
        myColor.b = 128;
        myColor.a = 128;
        for (int i =0; i<imageLandmarks.length(); i++){
            imageLandmarks.value(i).color = myColor;
        }
        for (int i = 0; i<newTipsList.length(); i++){

            imageLandmarks.append(newTipsList.value(i));

        }
        if (!imageLandmarks.isEmpty()){
            cb->setLandmark(newwin,imageLandmarks);
            cb->pushObjectIn3DWindow(newwin);
            qDebug()<<"set landmark group";
        }

        cb->pushObjectIn3DWindow(newwin);
        cb->updateImageWindow(newwin);
        emit analysisDone(newTipsList, newTargetList);

    }else{
        qDebug()<<"invalid image";
    }
}

void StackAnalyzer::processStack(Image4DSimple InputImage){

}

void StackAnalyzer::processSmartScan(QString fileWithData){
    // zhi add code to generate combined reconstruction from .txt file
    // at location filewith data
    qDebug()<<"caught filename "<<fileWithData;
    ifstream ifs(fileWithData.toLatin1());
    string info_swc;
    int offsetX, offsetY;
    string swcfilepath;
    vector<MyMarker*> outswc;
    int node_type = 1;
    while(ifs && getline(ifs, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilepath;

        vector<MyMarker*> inputswc = readSWC_file(swcfilepath);;

        for(V3DLONG d = 0; d < inputswc.size(); d++)
        {
            inputswc[d]->x = inputswc[d]->x + offsetX;
            inputswc[d]->y = inputswc[d]->y + offsetY;
            inputswc[d]->type = node_type;
            outswc.push_back(inputswc[d]);
        }
        node_type++;
    }

    QString fileSaveName = fileWithData + ".swc";
    saveSWC_file(fileSaveName.toStdString().c_str(), outswc);
    emit combinedSWC(fileSaveName);
}
