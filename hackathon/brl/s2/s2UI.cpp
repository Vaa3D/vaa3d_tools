#include "v3d_message.h"
#include <QWidget>
#include <QDialogButtonBox>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include <QThread>
#include <QDateTime>
#include "s2Controller.h"
#include "s2UI.h"
#include "s2plot.h"
#include "stackAnalyzer.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"

S2UI::S2UI(V3DPluginCallback2 &callback, QWidget *parent):   QDialog(parent)
{
    fileString =QString("");
    lastFile = QString("");
    allROILocations = new LandmarkList;
    cb = &callback;
    myStackAnalyzer = new StackAnalyzer(callback);
    s2Label = new QLabel(tr("smartScope 2"));
    s2LineEdit = new QLineEdit("");


    startPosMonButton = new QPushButton(tr("start monitor"));
    startSmartScanPB = new QPushButton(tr("SmartScan"));
    startSmartScanPB->setEnabled(false);
    startStackAnalyzerPB = new QPushButton(tr("trace single stack"));

    myNotes = new NoteTaker;

    rhTabs = new QTabWidget();
    rhTabs->addTab(   createROIMonitor(), "ROI Monitor");
    rhTabs->addTab(myNotes, "status and notes");

    lhTabs = new QTabWidget();
    lhTabs->addTab(createS2Monitors(), "s2 Monitor");
    lhTabs->addTab(&myPosMon, "monCOM" );
    lhTabs->addTab(&myController, "s2COM");
    lhTabs->addTab(createTracingParameters(),"tracing");
    lhTabs->addTab(createROIControls(),"ROI Controls");
    localRemoteCB = new QCheckBox;
    localRemoteCB->setText(tr("Local PrairieView"));
    mainLayout = new QGridLayout();
    //mainLayout->addWidget(s2Label, 0, 0);
    //mainLayout->addWidget(s2LineEdit, 0, 1);
    createButtonBox1();
    mainLayout->addWidget(startS2PushButton, 0,0, 1, 2);
    mainLayout->addWidget(startScanPushButton, 2,0);
    mainLayout->addWidget(loadScanPushButton, 3,0);
    mainLayout->addWidget(startZStackPushButton,2,1);
    mainLayout->addWidget(collectOverviewPushButton,3,1);
    //mainLayout->addWidget(startPosMonButton,3,0);
    mainLayout->addWidget(startSmartScanPB, 1,0,1,2);
    mainLayout->addWidget(localRemoteCB,4,0,1,2);
    mainLayout->addWidget(lhTabs, 5,0, 4, 3);
    mainLayout->addWidget(rhTabs,0,5,9,4);
    //    mainLayout->addWidget(startStackAnalyzerPB, 9, 0,1,2);
    roiGroupBox->show();
    hookUpSignalsAndSlots();
    //workerThread = new QThread;
    //myStackAnalyzer->moveToThread(workerThread);
    posMonStatus = false;
    waitingForFile = false;
    isLocal = false;
    smartScanStatus = 0;
    setLayout(mainLayout);
    setWindowTitle(tr("smartScope2 Interface"));
    updateLocalRemote(isLocal);
    //workerThread->start();
    startSmartScanPB->resize(50,40);

}

void S2UI::hookUpSignalsAndSlots(){
    //internal communication
    connect(startS2PushButton, SIGNAL(clicked()), this, SLOT(startS2()));
    connect(loadScanPushButton, SIGNAL(clicked()), this, SLOT(loadScan()));
    connect(startPosMonButton,SIGNAL(clicked()), this, SLOT(posMonButtonClicked()));
    connect(roiXEdit, SIGNAL(textChanged(QString)), this, SLOT(updateROIPlot(QString)));
    connect(roiYEdit, SIGNAL(textChanged(QString)), this, SLOT(updateROIPlot(QString)));
    connect(roiZEdit, SIGNAL(textChanged(QString)), this, SLOT(updateROIPlot(QString)));
    connect(roiXWEdit, SIGNAL(textChanged(QString)), this, SLOT(updateROIPlot(QString)));
    connect(roiYWEdit, SIGNAL(textChanged(QString)), this, SLOT(updateROIPlot(QString)));
    connect(roiZWEdit, SIGNAL(textChanged(QString)), this, SLOT(updateROIPlot(QString)));
    connect(roiClearPB, SIGNAL(clicked()),this,SLOT(clearROIPlot()));

    connect(localRemoteCB, SIGNAL(clicked(bool)), this, SLOT(updateLocalRemote(bool)));
    connect(overlapSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateOverlap(int)));
    // communication with myController to send commands
    connect(startScanPushButton, SIGNAL(clicked()), this, SLOT(startScan()));
    connect(&myController,SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(centerGalvosPB, SIGNAL(clicked()), &myController, SLOT(centerGalvos()));
    connect(startZStackPushButton, SIGNAL(clicked()), &myController, SLOT(startZStack()));
    connect(startZStackPushButton, SIGNAL(clicked()), this, SLOT(startingZStack()));
    connect(&myController, SIGNAL(statusSig(QString)), myNotes, SLOT(status(QString)));

    connect(startSmartScanPB, SIGNAL(clicked()), this, SLOT(startingSmartScan()));
    connect(collectOverviewPushButton, SIGNAL(clicked()),this, SLOT(collectOverview()));
    connect(collectOverviewPushButton, SIGNAL(clicked()),&myController, SLOT(overviewSetup()));



    // communication with myPosMon to monitor parameters
    connect(&myPosMon, SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(&myPosMon, SIGNAL(pmStatus(bool)), this, SLOT(pmStatusHandler(bool)));
    connect(&myPosMon, SIGNAL(newS2Parameter(QMap<int,S2Parameter>)), this, SLOT(updateS2Data(QMap<int,S2Parameter>)));
    connect(this, SIGNAL(startPM()), &myPosMon, SLOT(startPosMon()));
    connect(this, SIGNAL(stopPM()), &myPosMon, SLOT(stopPosMon()));



    // communication with  myStackAnalyzer
    connect(startStackAnalyzerPB, SIGNAL(clicked()),this, SLOT(toLoad()));
    connect(this, SIGNAL(newImageData(Image4DSimple)), myStackAnalyzer, SLOT(processStack(Image4DSimple)) );
    connect(myStackAnalyzer, SIGNAL(analysisDone(LandmarkList)), this, SLOT(handleNewLocation(LandmarkList)));
    connect(this, SIGNAL(moveToNext(LocationSimple)), &myController, SLOT(initROI(LocationSimple)));
    // connect(this, SIGNAL(callSALoad(QString)), myStackAnalyzer, SLOT(loadScan(QString)));
    connect(runSAStuff, SIGNAL(clicked()),this,SLOT(runSAStuffClicked()));
    connect(this, SIGNAL(processSmartScanSig(QString)), myStackAnalyzer, SLOT(processSmartScan(QString)));




    //communicate with NoteTaker:
    connect(this, SIGNAL(noteStatus(QString)), myNotes, SLOT(status(QString)));
}



QGroupBox *S2UI::createROIMonitor(){
    roiGroupBox = new QGroupBox(tr("ROI Monitor"));
    gl = new QGridLayout();
    roiGS = new QGraphicsScene();
    roiGS->setObjectName("roiGS");
    roiGV = new QGraphicsView();
    roiGV->setObjectName("roiGV");
    roiGV->setScene(roiGS);
    roiRect = QRectF(-400, -400, 800, 800);
    roiGS->addRect(roiRect,QPen::QPen(Qt::gray, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin), QBrush::QBrush(Qt::gray));
    newRect = roiGS->addRect(0,0,50,50);
    //roiGV->setViewportUpdateMode(QGraphicsView::FullViewportUpdate)  ;
    roiGV->adjustSize();
    roiGV->setDragMode(QGraphicsView::ScrollHandDrag);
    gl->addWidget(roiGV,0,0,4,4);
    roiClearPB = new QPushButton(tr("clear ROIs"));
    gl->addWidget(roiClearPB, 4,0);

    roiGroupBox->setLayout(gl);
    return roiGroupBox;
}

void S2UI::updateROIPlot(QString ignore){
    //roiRect.moveLeft(roiXEdit->text().toFloat());
    //roiRect.setY(roiYEdit->text().toFloat());
    //qDebug()<<"y="<<roiYEdit->text().toFloat();
    roiGS->removeItem(newRect);
    newRect =  roiGS->addRect(roiXEdit->text().toFloat(),roiYEdit->text().toFloat(),roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat());
    //newRect =  roiGS->addRect(uiS2ParameterMap[1].getCurrentValue()*10,uiS2ParameterMap[2].getCurrentValue()*10,uiS2ParameterMap[13].getCurrentValue(),uiS2ParameterMap[14].getCurrentValue());

}

void S2UI::updateLocalRemote(bool state){
    isLocal = state;
    status(QString("isLocal ").append(QString::number(isLocal)));
    QString timeString = QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm");
    QString topDirStr = QString("F:/testData/");

    if (isLocal){
        myController.hostLineEdit->setText(QString("local"));
        myPosMon.hostLineEdit->setText(QString("local"));
        topDirStr = QString("F:/testData/");


    }else{
        myController.hostLineEdit->setText(QString("10.128.50.5"));
        myPosMon.hostLineEdit->setText(QString("10.128.50.5"));
        topDirStr = QFileDialog::getExistingDirectory(this, tr("Choose save directory..."),
                                                      "/",
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);


    } QDir topDir = QDir(topDirStr);
    topDir.mkdir(timeString);
    saveDir =QDir(topDir.absolutePath().append("/").append(timeString));

    myNotes->setSaveDir(saveDir);

}


void S2UI::createButtonBox1(){
    startS2PushButton = new QPushButton(tr("Initialize SmartScope2"));
    startScanPushButton = new QPushButton(tr("single scan"));
    loadScanPushButton = new QPushButton(tr("load last scan"));
    startZStackPushButton = new QPushButton(tr("single z stack"));
    collectOverviewPushButton = new QPushButton(tr("collect &preview"));

}

QGroupBox *S2UI::createS2Monitors(){
    // add fields with data...  currently hardcoding the number of parameters...
    QFont newFont = QFont("Times", 8, QFont::Normal);
    QGroupBox *gMonBox = new QGroupBox(tr("&smartScope Monitor"));

    QGridLayout *gbMon = new QGridLayout;

    for (int jj=0; jj<=19; jj++){
        QLabel * labeli = new QLabel(tr("test"));
        labeli->setText(QString::number(jj));
        labeli->setObjectName(QString::number(jj));
        labeli->setWordWrap(true);
        labeli->setFont(newFont);
        gbMon->addWidget(labeli, jj%11, jj/11);
    }
    gMonBox->setLayout(gbMon);
    return gMonBox;
}


QGroupBox *S2UI::createTracingParameters(){
    // add fields with data...  currently hardcoding the number of parameters...
    QGroupBox *tPBox = new QGroupBox(tr("Tracing"));

    QGridLayout *tPL = new QGridLayout;
    runSAStuff = new QPushButton(tr("run stackAnalyzer code"));
    QLabel * labeli = new QLabel(tr("background threshold = "));
    QSpinBox *bkgSpnBx = new QSpinBox(0);
    s2Label->setText("input file path:");
    bkgSpnBx->setMaximum(255);
    bkgSpnBx->setMinimum(0);
    bkgSpnBx->setValue(10);
    bkgSpnBx->setObjectName("bkgSpinBox");


    overlapSpinBox = new QSpinBox;
    overlapSpinBox->setSuffix(" percent ");
    overlapSpinBox->setMinimum(0);
    overlapSpinBox->setMaximum(20 );
    overlapSpinBox->setValue(5);
    overlapSBLabel = new QLabel;
    overlapSBLabel->setText(tr("tile &overlap: "));
    overlapSBLabel->setBuddy(overlapSpinBox);


    tPL->addWidget(labeli,0,0);
    tPL->addWidget(bkgSpnBx,0,1);
    tPL->addWidget(s2Label,1,0);
    tPL->addWidget(s2LineEdit,1,1);
    tPL->addWidget(startStackAnalyzerPB,2,0,1,2);
    tPL->addWidget(runSAStuff,3,0,1,2);
    tPL->addWidget(overlapSpinBox,4,2);
    tPL->addWidget(overlapSBLabel,4,0);
    tPBox->setLayout(tPL);
    return tPBox;
}


QGroupBox *S2UI::createROIControls(){
    QGroupBox *gROIBox = new QGroupBox(tr("&ROI Controls"));
    gROIBox->setCheckable(true);
    gROIBox->setChecked(true);
    QLabel *roiXLabel = new QLabel(tr("ROI x ="));
    roiXEdit = new QLineEdit("0.0");
    roiXLabel->setBuddy(roiXEdit);
    roiXEdit->setObjectName("roiX");
    QLabel *roiYLabel = new QLabel(tr("ROI y ="));
    roiYEdit = new QLineEdit("0.0");
    roiYLabel->setBuddy(roiYEdit);
    roiYEdit->setObjectName("roiY");

    QLabel *roiZLabel = new QLabel(tr("ROI z ="));
    roiZEdit = new QLineEdit("0.0");
    roiZLabel->setBuddy(roiZEdit);
    roiZEdit->setObjectName("roiZ");

    QLabel *roiXWLabel = new QLabel(tr("size ="));
    roiXWEdit = new QLineEdit("0.0");
    roiXWLabel->setBuddy(roiXWEdit);
    roiXWEdit->setObjectName("roiXW");
    QLabel *roiYWLabel = new QLabel(tr(" size ="));
    roiYWEdit = new QLineEdit("0.0");
    roiYWLabel->setBuddy(roiYWEdit);
    roiYWEdit->setObjectName("roiYW");
    QLabel  *roiZWLabel = new QLabel(tr("size ="));
    roiZWEdit = new QLineEdit("0.0");
    roiZWLabel->setBuddy(roiZWEdit);
    roiZWEdit->setObjectName("roiZW");

    centerGalvosPB = new QPushButton(tr("center galvos"));


    QGridLayout *glROI = new QGridLayout;
    glROI->addWidget(roiXLabel, 1, 0);
    glROI->addWidget(roiXEdit, 1, 1);
    glROI->addWidget(roiXWLabel, 1, 2);
    glROI->addWidget(roiXWEdit, 1, 3);
    glROI->addWidget(roiYLabel, 2, 0);
    glROI->addWidget(roiYEdit, 2, 1);
    glROI->addWidget(roiYWLabel, 2, 2);
    glROI->addWidget(roiYWEdit, 2, 3);
    glROI->addWidget(roiZLabel, 3, 0);
    glROI->addWidget(roiZEdit, 3, 1);
    glROI->addWidget(roiZWLabel, 3, 2);
    glROI->addWidget(roiZWEdit, 3, 3);
    glROI->addWidget(centerGalvosPB, 4, 1, 1,2);

    gROIBox->setLayout(glROI);
    return gROIBox;
}



void S2UI::startS2()
{
    localRemoteCB->setEnabled(false);
    myController.initializeS2();
    myPosMon.initializeS2();
    QTimer::singleShot(1000,this, SLOT(posMonButtonClicked()));
    startS2PushButton->setText("s2 running");// should check something..?
}

void S2UI::startScan()
{
    lastFile=getFileString();
    //status(QString("lastFile = ").append(lastFile));
    waitingForFile = true;
    QTimer::singleShot(0, &myController, SLOT(startScan()));
    roiGS->addRect(roiXEdit->text().toFloat(),roiYEdit->text().toFloat(),roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen::QPen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));

}


void S2UI::loadScan(){
    QTimer::singleShot(0, this, SLOT(loadLatest()));
}

void S2UI::loadLatest(){
    loadScanFromFile(getFileString());
}

void S2UI::toLoad(){
    loadScanFromFile(s2LineEdit->text());
}
void S2UI::loadScanFromFile(QString fileString){

    QString latestString = fileString;


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
            //status(fileList[f]);
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
                status(QString(imageDir.absoluteFilePath(fileList[f])).append(" failed!"));
            }

        }


        total4DImage = new Image4DSimple;
        total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);
        total4DImage->setFileName(imageFileInfo.absoluteFilePath().toLatin1().data());


        status(QString("loaded file ").append(imageFileInfo.fileName()));
        status(QString("total4DImage is valid: ").append(QString::number(total4DImage->valid())));
        cb->setImage(newwin, total4DImage);
        cb->open3DWindow(newwin);


        NeuronTree nt;
        LandmarkList newTargetList;
        overlap = (float) overlapSpinBox->value();
        if (smartScanStatus ==1){
            total4DImage->setOriginX(scanList.value(scanNumber).x);// this is in pixels, using the expected origin
            total4DImage->setOriginY(scanList.value(scanNumber).y);
            status(QString("total4DImage is valid? ").append(QString::number(total4DImage->valid())));

            if (total4DImage->valid()){
                QString swcString = saveDir.absolutePath().append("/").append(QString::number(scanNumber)).append("test.swc");

                V3DLONG mysz[4];
                mysz[0] = total4DImage->getXDim();
                mysz[1] = total4DImage->getYDim();
                mysz[2] = total4DImage->getZDim();
                mysz[3] = total4DImage->getCDim();
                //simple_saveimage_wrapper(*cb, swcString.append(".v3draw").toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);
                outputStream<<scanList.value(scanNumber).x<<" "<<scanList.value(scanNumber).y<<" "<<swcString<<"\n";
                PARA_APP2 p;
                p.is_gsdt = false;
                p.is_coverage_prune = true;
                p.is_break_accept = false;
                p.bkg_thresh = this->findChild<QSpinBox*>("bkgSpinBox")->value();
                p.length_thresh = 5;
                p.cnn_type = 2;
                p.channel = 0;
                p.SR_ratio = 3.0/9.9;
                p.b_256cube = 1;
                p.b_RadiusFrom2D = true;
                p.b_resample = 1;
                p.b_intensity = 0;
                p.b_brightfiled = 0;
                p.outswc_file =swcString.toLatin1().data();

                //p.outswc_file =QString(total4DImage->getFileName()).append("test.swc").toLatin1().data();

                p.p4dImage = total4DImage;
                p.xc0 = p.yc0 = p.zc0 = 0;
                p.xc1 = p.p4dImage->getXDim()-1;
                p.yc1 = p.p4dImage->getYDim()-1;
                p.zc1 = p.p4dImage->getZDim()-1;

                QString versionStr = "v2.621";
                proc_app2(*cb, p, versionStr);

                nt = readSWC_file(p.outswc_file);
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
                if (!newTargetList.empty()){
                    for (int i = 0; i<newTargetList.length(); i++){
                        newTargetList[i].x = (1.0-overlap)*newTargetList[i].x+p.p4dImage->getOriginX();
                        newTargetList[i].y=(1.0-overlap)*newTargetList[i].y+p.p4dImage->getOriginY();
                        newTargetList[i].z =(1.0-overlap)*newTargetList[i].z+p.p4dImage->getOriginZ();
                    }
                }
                handleNewLocation(newTargetList);

            }


            cb->setSWC(newwin,nt);
            cb->setLandmark(newwin,newTargetList);
            cb->pushObjectIn3DWindow(newwin);

        }else{status("invalid imagedata");}
        cb->updateImageWindow(newwin);


    }else{
        status(QString("invalid image path: ").append(latestString));
    }
}



void S2UI::displayScan(){ // this will listen for a signal from myController
    //containing either a filename or  eventually an address

}



void S2UI::pmStatusHandler(bool pmStatus){
    posMonStatus = pmStatus;
    status("pmstatus updated");
}

void S2UI::posMonButtonClicked(){
    // if it's not running, start it
    // and change button text to 'stop pos mon'
    if (!posMonStatus){
        emit startPM();
        status("Position Monitor started");
        startPosMonButton->setText(tr("stop position monitor"));
        startSmartScanPB->setEnabled(true);
    }
    else{
        emit stopPM();
        startPosMonButton->setText(tr("start position monitor"));
        startSmartScanPB->setEnabled(false);

    }
    // if it's running, stop it
    // and change text to start pos mon


}
void S2UI::updateS2Data( QMap<int, S2Parameter> currentParameterMap){


    int minVal = 0;
    int maxVal = currentParameterMap.keys().length();
    for (int i= 0; i <maxVal ; i++){
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
            item->setText(parameterStringi.split("\\").last());
        }


    }

    checkParameters(currentParameterMap);

}

void S2UI::checkParameters(QMap<int, S2Parameter> currentParameterMap){
    int minVal = 0;
    int maxVal = currentParameterMap.keys().length();
    for (int i= 0; i <maxVal ; i++){
        if (currentParameterMap[i].getExpectedType().contains("float")){
            if (currentParameterMap[i].getCurrentValue() != uiS2ParameterMap[i].getCurrentValue())
                uiS2ParameterMap[i].setCurrentValue(currentParameterMap[i].getCurrentValue());
            if (i==18){
                //updateROIPlot(QString("ignore"));
                roiXEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
            }else if (i==19){
                roiYEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
            }else if (i==13){
                roiXWEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
            }else if (i==14){
                roiYWEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
            }
        }
    }
}



void S2UI::updateString(QString broadcastedString){
}


//  -------  smart scanning stuffs   --------
//  -----------------------------------------
void S2UI::startingSmartScan(){

    if (smartScanStatus==1){
        smartScanStatus=0;
        startSmartScanPB->setText("smartScan");
        allROILocations->clear();
        saveTextFile.close();
        return;
    }
    smartScanStatus = 1;

    scanDataFileString = saveDir.absolutePath().append("/").append("scanData.txt");
    status(scanDataFileString);
    saveTextFile.setFileName(scanDataFileString);// add currentScanFile
    if (!saveTextFile.isOpen()){
        if (!saveTextFile.open(QIODevice::Text|QIODevice::WriteOnly)){
            return;}     }

    outputStream.setDevice(&saveTextFile);

    startSmartScanPB->setText("cancel smartScan");
    if (allROILocations->isEmpty()){
        scanList.clear();
        scanNumber = 0;
        status("starting smartScan...");
        LocationSimple startLocation = LocationSimple(uiS2ParameterMap[18].getCurrentValue()/uiS2ParameterMap[8].getCurrentValue(),
                uiS2ParameterMap[19].getCurrentValue()/uiS2ParameterMap[9].getCurrentValue(),
                0);
        startLocation.mass = 0;
        allROILocations->append(startLocation);
        QTimer::singleShot(10,this, SLOT(smartScanHandler()));
    }
    // append text to noteTaker
    //

}

void S2UI::handleNewLocation(LandmarkList newLandmarks){
    status(QString("got ").append(QString::number( newLandmarks.length())).append("  new landmarks associated with ROI "). append(QString::number(scanNumber)));
    for (int i = 0; i<newLandmarks.length(); i++){
        status(QString("x= ").append(QString::number(newLandmarks.value(i).x)).append(" y = ").append(QString::number(newLandmarks.value(i).y)).append(" z= ").append(QString::number(newLandmarks.value(i).z)));
        if (!isDuplicateROI(newLandmarks.value(i))){
            allROILocations->append(newLandmarks.value(i));
        }else{
            status("already scanned here!");
        }
    }
    scanNumber++;
    smartScanHandler();
}

bool S2UI::isDuplicateROI(LocationSimple inputLocation){
    for (int i=0; i<scanList.length(); i++){
        if ((inputLocation.x == scanList[i].x)&(inputLocation.y == scanList[i].y))
            return true;
    }
    return false;
}
void S2UI::smartScanHandler(){
    if (smartScanStatus!=1){
        status("smartScan aborted");
        scanNumber = 0;
        saveTextFile.close();
        emit processSmartScanSig(scanDataFileString);
        return;
    }
    if (allROILocations->isEmpty()){
        v3d_msg("Finished with smartscan !",true);
        saveTextFile.close();
        smartScanStatus = 0;
        emit processSmartScanSig(scanDataFileString);
    }

    status(QString("we now have a total of ").append(QString::number( allROILocations->length())).append(" target ROIs..."));
    for (int i = 0; i<allROILocations->length(); i++){
        status(QString("x= ").append(QString::number(allROILocations->value(i).x)).append(" y = ").append(QString::number(allROILocations->value(i).y)).append(" z= ").append(QString::number(allROILocations->value(i).z)));
    }

    if (!allROILocations->isEmpty()){
        LocationSimple nextLocation = allROILocations->first();
        allROILocations->removeFirst();
        moveToROI(nextLocation);
        if (smartScanStatus==1){
            QGraphicsTextItem* sequenceNumberText;

            sequenceNumberText = new QGraphicsTextItem;
            sequenceNumberText->setPos(roiXEdit->text().toFloat()+10,roiYEdit->text().toFloat());
            sequenceNumberText->setPlainText(QString::number(scanNumber));
            roiGS->addItem(sequenceNumberText);
        }
        waitingForFile = true;
        scanList.append(nextLocation);
        QTimer::singleShot(100, &myController, SLOT(startZStack())); //hardcoded delay here... not sure
        // how to make this more eventdriven. maybe  wait for move to finish.
        status(QString("start next ROI at x = ").append(QString::number(nextLocation.x)).append("  y = ").append(QString::number(nextLocation.y)));

        //outputStream<<notes->toPlainText();
        // saveTextFile.close();
    }

}


void S2UI::moveToROI(LocationSimple nextROI){
    // convert from pixels to microns:
    if( posMonStatus){
        float nextXMicrons = nextROI.x * uiS2ParameterMap[8].getCurrentValue();
        float nextYMicrons = nextROI.y* uiS2ParameterMap[9].getCurrentValue();
        // and now to galvo voltage:
        float nextGalvoX = nextXMicrons/uiS2ParameterMap[17].getCurrentValue();
        float nextGalvoY = nextYMicrons/uiS2ParameterMap[17].getCurrentValue();
        LocationSimple newLoc;
        newLoc.x = -nextGalvoX;
        newLoc.y = nextGalvoY;
        roiGS->addRect(-nextXMicrons,nextYMicrons,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen::QPen(Qt::blue, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));


        emit moveToNext(newLoc);
    }else{
        status("start PosMon before moving galvos");
        smartScanStatus = -1;
    }
}

// ------------------------------------


void S2UI::collectOverview(){
    // collect overview stack at lowest mag and high spatial resolution. requires galvo mode.
    // this is called at the same time that a signal is sent to the controller to setup for an overview

    // so start my monitor to see when it's in ready state:
    overviewCycles = 0;
    status("start overview");
    QTimer::singleShot(0, this, SLOT(overviewHandler()));
}

void S2UI::overviewHandler(){
    bool readyForOverview = !uiS2ParameterMap[0].getCurrentString().contains("Resonant") &&
            ((int) uiS2ParameterMap[12].getCurrentValue() ==1)&&
            ((int) uiS2ParameterMap[10].getCurrentValue() == 1024)&&
            ((int) uiS2ParameterMap[11].getCurrentValue() == 1024);
    bool overViewTimedOut = overviewCycles >50;

    if (overViewTimedOut){
        status("overview timeout!");
        return;
    }
    if (readyForOverview){
        QTimer::singleShot(100, startScanPushButton, SLOT(click()));
        status("starting single scan");
        return;
    }
    if (!readyForOverview&&!overViewTimedOut){
        overviewCycles++;
        QTimer::singleShot(100, this, SLOT(overviewHandler()));
    }
}


void S2UI::startingZStack(){
    waitingForFile = true;
    QTimer::singleShot(100, &myController, SLOT(startZStack()));
    status("start single z Stack");
    roiGS->addRect(roiXEdit->text().toFloat(),roiYEdit->text().toFloat(),roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen::QPen(Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

}

void S2UI::updateFileString(QString inputString){
    //this means a new file has been created.. it can be late by up to 1 full cycle of s2parametermap updating
    // but it guarantees that the acquisition is done
    // a separate poller of updated filename could be much faster.
    // final version will require much more rigorous timing- it's not clear how we'll parse out
    // the streamed image data into files...
    fileString = inputString;
    if (!isLocal){
        fileString.replace("\\AIBSDATA","\\Volumes").replace("\\","/");
    }
    fileString.append("_Cycle00001_Ch2_000001.ome.tif");
    if ((QString::compare(fileString,lastFile, Qt::CaseInsensitive)!=0)&(waitingForFile)){
        waitingForFile = false;
        QTimer::singleShot(0, this, SLOT(loadScan()));

    }
    lastFile = fileString;
}

void S2UI::s2ROIMonitor(){ // future version will work like this...

    if ((!allROILocations->isEmpty())&(!waitingForFile)){
        LocationSimple nextLocation = allROILocations->first();
        allROILocations->removeFirst();
        moveToROI(nextLocation);
        QTimer::singleShot(100, &myController, SLOT(startZStack()));

    }

    QTimer::singleShot(10, this, SLOT(s2ROIMonitor()));

}

void S2UI::clearROIPlot(){
    roiGS->clear();
    roiRect = QRectF(-400, -400, 800, 800);
    roiGS->addRect(roiRect,QPen::QPen(Qt::gray, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin), QBrush::QBrush(Qt::gray));
    newRect = roiGS->addRect(0,0,10,10);
}

QString S2UI::getFileString(){
    return fileString;
}

void S2UI::status(QString statString){
    emit noteStatus(statString);
}

QDir S2UI::getSaveDirectory(){

    return saveDir;
}


void S2UI::runSAStuffClicked(){
    emit processSmartScanSig(s2LineEdit->text());
}

void S2UI::updateOverlap(int value){
    overlap = (float) value;
}


void S2UI::getCurrentParameters(){
    emit currentParameters(uiS2ParameterMap);
}

//  set filename in Image4DSimple* using  ->setFileName
//  BEFORE PASSING TO StackAnalyzer slot.


// get voltage-to-pixel-to-micron resolution and add methods to
// move to pan to micron positions using -sts currentScanCenter

// need an origin at the beginning

// need a new 'start smartscan' button as path to eventual handler

//


// HAVE TO FIGURE OUT HOW TO PASS THE IMAGE4DSIMPLE THROUGH A SLOT!
