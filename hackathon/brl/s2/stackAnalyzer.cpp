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



void StackAnalyzer::loadScan(QString latestString){


    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!
    //QString latestString =QString("/Volumes/mat/BRL/testData/ZSeries-01142016-0940-048/ZSeries-01142016-0940-048_Cycle00001_Ch2_000001.ome.tif");
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
            qDebug()<<fileList[f];
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


        Image4DSimple total4DImage;
        total4DImage.setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);

        PARA_APP2 p;
        p.is_gsdt = false;
        p.is_coverage_prune = true;
        p.is_break_accept = false;
        p.bkg_thresh = 150;
        p.length_thresh = 5;
        p.cnn_type = 2;
        p.channel = 0;
        p.SR_ratio = 3.0/9.9;
        p.b_256cube = 1;
        p.b_RadiusFrom2D = true;
        p.b_resample = 1;
        p.b_intensity = 0;
        p.b_brightfiled = 0;
        p.outswc_file = "/Users/brianl/dump/_app2.swc";

        p.p4dImage = &total4DImage;
        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;

        QString versionStr = "v2.621";
        proc_app2(*cb, p, versionStr);

        NeuronTree nt;
        nt = readSWC_file(p.outswc_file);
        LandmarkList newTargetList;
        V3DLONG neuronNum = nt.listNeuron.size();
        bool scan_left = false, scan_right = false, scan_up = false, scan_down = false;
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG node_x = nt.listNeuron[i].x;
            V3DLONG node_y = nt.listNeuron[i].y;
            V3DLONG node_z = nt.listNeuron[i].z;

            LocationSimple newTarget;
            if(node_x <= 0.05*p.p4dImage->getXDim() && !scan_left)
            {
                newTarget.x = -p.p4dImage->getXDim();
                newTarget.y = 0;
                newTarget.z = node_z;

                scan_left = true;
                newTargetList.push_back(newTarget);
            }
            if(node_x >= 0.95*p.p4dImage->getXDim() && !scan_right)
            {
                newTarget.x = p.p4dImage->getXDim();
                newTarget.y = 0;
                newTarget.z = node_z;
                scan_right = true;
                newTargetList.push_back(newTarget);
            }
            if(node_y <= 0.05*p.p4dImage->getYDim() && !scan_up)
            {
                newTarget.x = 0;
                newTarget.y = -p.p4dImage->getYDim();
                newTarget.z = node_z;
                scan_up = true;
                newTargetList.push_back(newTarget);
            }
            if(node_y >= 0.95*p.p4dImage->getYDim() && !scan_down)
            {
                newTarget.x = 0;
                newTarget.y = p.p4dImage->getYDim();
                newTarget.z = node_z;
                scan_down = true;
                newTargetList.push_back(newTarget);
            }
        }
emit analysisDone(newTargetList);
        cb->setImage(newwin, &total4DImage);
        cb->open3DWindow(newwin);
        cb->setSWC(newwin,nt);
        cb->setLandmark(newwin,newTargetList);
        cb->pushObjectIn3DWindow(newwin);
        cb->updateImageWindow(newwin);

        // exact format is TBD...  center of face with tips?
        //                         average location of tips on each face?
        //

    }else{
        qDebug()<<"invalid image";
    }
}

void StackAnalyzer::processStack(Image4DSimple InputImage){
    qDebug()<<InputImage.valid();
    // process stack data
    PARA_APP2 p;
    p.is_gsdt = false;
    p.is_coverage_prune = true;
    p.is_break_accept = false;
    p.bkg_thresh = 10;
    p.length_thresh = 5;
    p.cnn_type = 2;
    p.channel = 0;
    p.SR_ratio = 3.0/9.9;
    p.b_256cube = 1;
    p.b_RadiusFrom2D = true;
    p.b_resample = 1;
    p.b_intensity = 0;
    p.b_brightfiled = 0;
    p.outswc_file = QString(InputImage.getFileName()).append( "_app2.swc");

    p.p4dImage = &InputImage;
    p.xc0 = p.yc0 = p.zc0 = 0;
    p.xc1 = p.p4dImage->getXDim()-1;
    p.yc1 = p.p4dImage->getYDim()-1;
    p.zc1 = p.p4dImage->getZDim()-1;

qDebug()<<p.p4dImage->valid();
QFileInfo testFileInfo = QFileInfo(QString(p.outswc_file));
qDebug()<<testFileInfo.isWritable();
    QString versionStr = "v2.621";
    proc_app2(*cb, p, versionStr);
    NeuronTree nt;
    nt = readSWC_file(p.outswc_file);

qDebug()<< "done with readSWC_file";

    // determine all ROI locations and put them in newTargetList, a QList of 3-ints [x,y,z] coordinates
    // for the next ROI.
    LandmarkList newTargetList;
    V3DLONG neuronNum = nt.listNeuron.size();
    bool scan_left = false, scan_right = false, scan_up = false, scan_down = false;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG node_x = nt.listNeuron[i].x;
        V3DLONG node_y = nt.listNeuron[i].y;

        LocationSimple newTarget;
        if(node_x <= 0.05*p.p4dImage->getXDim() && !scan_left)
        {
            newTarget.x = -p.p4dImage->getXDim()*p.p4dImage->getRezX() + p.p4dImage->getOriginX();
            newTarget.y = p.p4dImage->getOriginY();
            scan_left = true;
            newTargetList.push_back(newTarget);
        }
        if(node_x >= 0.95*p.p4dImage->getXDim() && !scan_right)
        {
            newTarget.x = p.p4dImage->getXDim()*p.p4dImage->getRezX() + p.p4dImage->getOriginX();
            newTarget.y = p.p4dImage->getOriginY();
            scan_right = true;
            newTargetList.push_back(newTarget);
        }
        if(node_y <= 0.05*p.p4dImage->getYDim() && !scan_up)
        {
            newTarget.x = p.p4dImage->getOriginX();
            newTarget.y = -p.p4dImage->getYDim()*p.p4dImage->getRezY() + p.p4dImage->getOriginY();
            scan_up = true;
            newTargetList.push_back(newTarget);
        }
        if(node_y >= 0.95*p.p4dImage->getYDim() && !scan_down)
        {
            newTarget.x = p.p4dImage->getOriginX();
            newTarget.y = p.p4dImage->getYDim()*p.p4dImage->getRezY() + p.p4dImage->getOriginY();
            scan_down = true;
            newTargetList.push_back(newTarget);
        }
    }

//    v3dhandle newwin = cb->newImageWindow();
//    cb->setImage(newwin, pInputImage);
//    cb->open3DWindow(newwin);
//    cb->setSWC(newwin,nt);
//    cb->pushObjectIn3DWindow(newwin);
//    cb->updateImageWindow(newwin);


    emit analysisDone(newTargetList);

return;


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
    V3dR_MainWindow * new3DWindow = NULL;
    new3DWindow = cb->createEmpty3DViewer();
    QList<NeuronTree> * new_treeList = cb->getHandleNeuronTrees_Any3DViewer (new3DWindow);
    if (!new_treeList)
    {
        v3d_msg(QString("New 3D viewer has invalid neuron tree list"));
        return;
    }
    NeuronTree resultTree;
    resultTree = readSWC_file(fileSaveName);
    new_treeList->push_back(resultTree);
    cb->setWindowDataTitle(new3DWindow, "Final reconstruction");
    cb->update_NeuronBoundingBox(new3DWindow);

}
