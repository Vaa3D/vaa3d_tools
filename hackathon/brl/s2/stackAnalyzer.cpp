#include "stackAnalyzer.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"

StackAnalyzer::StackAnalyzer(V3DPluginCallback2 &callback, QObject *parent) :
    QObject(parent)
{
    cb = &callback;
}



void StackAnalyzer::loadScan(){


    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!
    QString latestString =QString("/data/mat/BRL/testData/ZSeries-01142016-0940-048/ZSeries-01142016-0940-048_Cycle00001_Ch2_000001.ome.tif");
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
        p.outswc_file = latestString + "_app2.swc";

        p.p4dImage = &total4DImage;
        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;


        QString versionStr = "v2.621";
        proc_app2(*cb, p, versionStr);
        NeuronTree nt;
        nt = readSWC_file(p.outswc_file);
        cb->setImage(newwin, &total4DImage);
        cb->open3DWindow(newwin);
        cb->setSWC(newwin,nt);
        cb->pushObjectIn3DWindow(newwin);
        cb->updateImageWindow(newwin);




    }else{
        qDebug()<<"invalid image";
    }
}

void StackAnalyzer::processStack(Image4DSimple * pInputImage){
    // process stack data


// emit messageSignal(QString("message!")



    // determine all ROI locations and put them in newTargetList, a QList of 3-ints [x,y,z] coordinates
    // for the next ROI.
    QList<QList<int> > newTargetList;
    // exact format is TBD...  center of face with tips?
    //                         average location of tips on each face?
    //


    // emit analysisDone(newTargetList);




}
