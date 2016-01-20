#include "stackAnalyzer.h"

StackAnalyzer::StackAnalyzer(V3DPluginCallback2 &callback, QObject *parent) :
    QObject(parent)
{
    cb = &callback;
}



void StackAnalyzer::loadScan(){


    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!
    QString latestString =QString("/Volumes/mat/BRL/testData/ZSeries-01142016-0940-048/ZSeries-01142016-0940-048_Cycle00001_Ch2_000001.ome.tif");
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
        long x = pNewImage->getXDim();
        long y = pNewImage->getYDim();
        long nFrames = fileList.length();
        long pBytes = pNewImage->getUnitBytes();



        V3DLONG tunits = x*y*nFrames*pBytes;
        unsigned char * total1dData = new unsigned char [tunits];
        long totalImageIndex = 0;
        for (int f=0; f<nFrames; f++){
            qDebug()<<fileList[f];
            Image4DSimple * pNewImage = cb->loadImage(imageDir.absoluteFilePath(fileList[f]).toLatin1().data());
            if (pNewImage->valid()){
                unsigned char * data1d = 0;
                data1d = new unsigned char [x*y*pBytes];
                pNewImage->setNewRawDataPointer(data1d);
                for (long i = 0; i< (x*y*pBytes); i++){
                    total1dData[totalImageIndex]= data1d[i];
                    totalImageIndex++;
                }
            }else{
                qDebug()<<imageDir.absoluteFilePath(fileList[f])<<" failed!";
            }
        }
        Image4DSimple  total4DImage;
        total4DImage.setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);
        cb->setImage(newwin, &total4DImage);
        cb->setImageName(newwin,QString("test"));
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
