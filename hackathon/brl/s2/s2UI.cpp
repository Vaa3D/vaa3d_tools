#include "v3d_message.h"
#include <QWidget>
#include <QDialogButtonBox>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "s2Controller.h"
#include "s2UI.h"


S2UI::S2UI(V3DPluginCallback2 &callback, QWidget *parent):   QDialog(parent)
{
    fileString =QString("");
    lastFile = QString("");
    cb = &callback;
    s2Label = new QLabel(tr("smartScope 2"));
    s2LineEdit = new QLineEdit("01b");
    startS2PushButton = new QPushButton(tr("Start smartScope2"));
    startScanPushButton = new QPushButton(tr("start scan"));
    loadScanPushButton = new QPushButton(tr("load last scan"));
    startPosMonButton = new QPushButton(tr("start monitor"));
    buttonBox1 = new QDialogButtonBox;
    buttonBox1->addButton(startS2PushButton, QDialogButtonBox::ActionRole);
    buttonBox1->addButton(startScanPushButton, QDialogButtonBox::RejectRole);
    buttonBox1->addButton(loadScanPushButton, QDialogButtonBox::RejectRole);

    connect(startS2PushButton, SIGNAL(clicked()), this, SLOT(startS2()));
    connect(startScanPushButton, SIGNAL(clicked()), &myController, SLOT(startScan()));
    connect(loadScanPushButton, SIGNAL(clicked()), this, SLOT(loadScan()));
    connect(startPosMonButton,SIGNAL(clicked()), this, SLOT(posMonButtonClicked()));
    connect(&myController,SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(&myPosMon, SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(&myPosMon, SIGNAL(pmStatus(bool)), this, SLOT(pmStatusHandler(bool)));
    connect(&myPosMon, SIGNAL(newS2Parameter(QMap<int,S2Parameter>)), this, SLOT(updateS2Data(QMap<int,S2Parameter>)));
    connect(this, SIGNAL(startPM()), &myPosMon, SLOT(startPosMon()));
    connect(this, SIGNAL(stopPM()), &myPosMon, SLOT(stopPosMon()));

    v3dhandle curwin = callback.currentImageWindow();
    /*if (!curwin)
    {
        v3d_msg("Please open an image.");
        return;
    }*/

    mainLayout = new QGridLayout();
    mainLayout->addWidget(s2Label, 0, 0);
    mainLayout->addWidget(s2LineEdit, 0, 1);
    mainLayout->addWidget(buttonBox1,1,0,2,4);
    mainLayout->addWidget(startPosMonButton,3,0);
    // add fields with data...  currently hardcoding the number of parameters...
    for (int jj=0; jj<=7; jj++){
        QLabel * labeli = new QLabel(tr("test"));
        labeli->setText(QString::number(jj));
        labeli->setObjectName(QString::number(jj));
        mainLayout->addWidget(labeli,jj+4,0,1,4);
    }



    setLayout(mainLayout);
    setWindowTitle(tr("smartScope2 Interface"));



}


void S2UI::startS2()
{
    myController.show();
    myPosMon.show();
    myPosMon.setWindowTitle(tr("posMon"));
}

void S2UI::startScan()
{
    lastFile=getFileString();
    waitingForFile = true;
}


void S2UI::loadScan(){
    //myController.getROIData(); // this should really be a signal to myController,
    // not an explicit call

    //QString latestString = getFileString();
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

void S2UI::displayScan(){ // this will listen for a signal from myController
    //containing either a filename or  eventually an address

}

void S2UI::pmStatusHandler(bool pmStatus){
    posMonStatus = pmStatus;
    s2LineEdit->setText(tr("pmstatus updated"));
}

void S2UI::posMonButtonClicked(){
    // if it's not running, start it
    // and change button text to 'stop pos mon'
    if (!posMonStatus){
        emit startPM();
        s2LineEdit->setText(tr("pm stop"));
        startPosMonButton->setText(tr("stop position monitor"));
    }
    else{
        emit stopPM();
        startPosMonButton->setText(tr("start position monitor"));
    }
    // if it's running, stop it
    // and change text to start pos mon


}
void S2UI::updateS2Data( QMap<int, S2Parameter> currentParameterMap){
    for (int i: currentParameterMap.keys()){
        QString parameterStringi = currentParameterMap[i].getParameterName();
        float parameterValuei = currentParameterMap[i].getCurrentValue();
        QString iString = QString::number(i);
        if (currentParameterMap[i].getExpectedType().contains("string")){
            parameterStringi.append(" = ").append(currentParameterMap[i].getCurrentString());
        }
        if (currentParameterMap[i].getExpectedType().contains("float")){
            parameterStringi.append(" = ").append(QString::number(parameterValuei));
        }
        if (currentParameterMap[i].getExpectedType().contains("list")){
            QString fString = currentParameterMap[i].getCurrentString().split(".xml").first();
            parameterStringi.append(" = ").append(fString);
            updateFileString(fString);
        }
        QLabel* item = this->findChild<QLabel*>( iString);
        if (item){
            item->setText(parameterStringi);
        }
    }

}


void S2UI::updateString(QString broadcastedString){
}

void S2UI::updateFileString(QString inputString){
    fileString = inputString;
    fileString.replace("Z:\\","\Volumes").replace("\\","/").append("_Cycle00001_Ch2_000001.ome.tif");
    if ((!QString::compare(fileString, lastFile))& (waitingForFile)){
        waitingForFile = false;
        loadScan();
        qDebug()<<fileString;
    }
    lastFile = fileString;
}

QString S2UI::getFileString(){
    return fileString;
}
