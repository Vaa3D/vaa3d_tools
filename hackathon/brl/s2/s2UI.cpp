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
    qRegisterMetaType<LandmarkList>("LandmarkList");
    qRegisterMetaType<LocationSimple>("LocationSimple");
    qRegisterMetaType<QList<LandmarkList> >("QList<LandmarkList>");
    fileString =QString("");
    lastFile = QString("");
    allROILocations = new LandmarkList;
    allTipsList = new QList<LandmarkList>;

    previewWindow = new v3dhandle;
    havePreview = false;
    QList<LandmarkList>    allScanLocations ;

    cb = &callback;
    myStackAnalyzer = new StackAnalyzer(callback);
    s2Label = new QLabel(tr("smartScope 2"));
    s2LineEdit = new QLineEdit("");


    startPosMonButton = new QPushButton(tr("start monitor"));
    startSmartScanPB = new QPushButton(tr("SmartScan"));
    startSmartScanPB->setEnabled(false);
    startStackAnalyzerPB = new QPushButton(tr("trace single stack"));
    resetToOverviewPB = new QPushButton(tr("&RESET to preview"));
    resetToScanPB = new QPushButton(tr("RESET to &SmartScan"));
    pickTargetsPB = new QPushButton(tr("pick smartScan starting points"));

    myNotes = new NoteTaker;
    myEventLogger  = new EventLogger();


    rhTabs = new QTabWidget();
    rhTabs->addTab(   createROIMonitor(), "ROI Monitor");
    rhTabs->addTab(myNotes, "status and notes");


    createTargetList();
    lhTabs = new QTabWidget();
    lhTabs->addTab(createS2Monitors(), "s2 Monitor");
    lhTabs->addTab(&myPosMon, "monCOM" );
    lhTabs->addTab(&myController, "s2COM");
    lhTabs->addTab(createTracingParameters(),"tracing");
    lhTabs->addTab(createROIControls(),"ROI Controls");
    localRemoteCB = new QCheckBox;
    localRemoteCB->setText(tr("Local PrairieView"));
    runAllTargetsPB = new QPushButton;
    runAllTargetsPB->setText(tr("Scan All Targets"));
    mainLayout = new QGridLayout();
    //mainLayout->addWidget(s2Label, 0, 0);
    //mainLayout->addWidget(s2LineEdit, 0, 1);
    createButtonBox1();
    mainLayout->addWidget(startS2PushButton, 0,0, 1, 3);
    mainLayout->addWidget(startScanPushButton, 2,0);
    mainLayout->addWidget(loadScanPushButton, 3,0);
    mainLayout->addWidget(startZStackPushButton,2,1);
    mainLayout->addWidget(collectOverviewPushButton,3,1);
    mainLayout->addWidget(resetToOverviewPB,2,2);
    mainLayout->addWidget(resetToScanPB,3,2);
    mainLayout->addWidget(pickTargetsPB,4,1);

    //mainLayout->addWidget(startPosMonButton,3,0);
    mainLayout->addWidget(startSmartScanPB, 1,0,1,3);
    mainLayout->addWidget(localRemoteCB,5,0,1,1);
    mainLayout->addWidget(runAllTargetsPB,5,2);
    mainLayout->addWidget(lhTabs, 6,0, 4, 3);
    mainLayout->addWidget(rhTabs,0,5,9,4);
    //    mainLayout->addWidget(startStackAnalyzerPB, 9, 0,1,2);
    roiGroupBox->show();



    myTargetTable.show();
    targetIndex = 0;
    overViewPixelToScanPixel = (1.0/16.0)*(256.0/512.0);
    overviewMicronsPerPixel = 1.8;
    hookUpSignalsAndSlots();


    workerThread = new QThread;
    myStackAnalyzer->moveToThread(workerThread);
    workerThread->start();


    posMonStatus = false;
    waitingForFile = 0;
    isLocal = false;
    smartScanStatus = 0;
    gridScanStatus = 0;
    allTargetStatus = 0;
    setLayout(mainLayout);
    setWindowTitle(tr("smartScope2 Interface"));
    updateLocalRemote(isLocal);
    channelChoiceComboB->setCurrentIndex(1);


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

    connect(resetToOverviewPB, SIGNAL(clicked()),this, SLOT(resetToOverviewPBCB()));
    connect(resetToScanPB, SIGNAL(clicked()), this, SLOT(resetToScanPBCB()));

    connect(pickTargetsPB,SIGNAL(clicked()),this, SLOT(pickTargets()));
    connect(runAllTargetsPB,SIGNAL(clicked()),this,SLOT(startAllTargets()));


    connect(runContinuousCB,SIGNAL(clicked()), this, SLOT(s2ROIMonitor()));


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
    connect(resetToScanPB, SIGNAL(clicked()), &myController, SLOT(stackSetup()));



    // communication with myPosMon to monitor parameters
    connect(&myPosMon, SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
    connect(&myPosMon, SIGNAL(pmStatus(bool)), this, SLOT(pmStatusHandler(bool)));
    connect(&myPosMon, SIGNAL(newS2Parameter(QMap<int,S2Parameter>)), this, SLOT(updateS2Data(QMap<int,S2Parameter>)));
    connect(this, SIGNAL(startPM()), &myPosMon, SLOT(startPosMon()));
    connect(this, SIGNAL(stopPM()), &myPosMon, SLOT(stopPosMon()));



    // communication with  myStackAnalyzer
    connect(startStackAnalyzerPB, SIGNAL(clicked()),this, SLOT(loadForSA()));
    connect(this, SIGNAL(newImageData(Image4DSimple)), myStackAnalyzer, SLOT(processStack(Image4DSimple)) );
    connect(myStackAnalyzer, SIGNAL(analysisDone(QList<LandmarkList>, LandmarkList,Image4DSimple*)), this, SLOT(handleNewLocation(QList<LandmarkList>,LandmarkList, Image4DSimple*)));
    connect(this, SIGNAL(moveToNext(LocationSimple)), &myController, SLOT(initROI(LocationSimple)));
    connect(this, SIGNAL(callSALoad(QString,float,int,bool, LandmarkList, LocationSimple,QString,bool,bool)), myStackAnalyzer, SLOT(loadScan(QString,float,int,bool,LandmarkList, LocationSimple, QString,bool,bool)));
    connect(this, SIGNAL(callSAGridLoad(QString,LocationSimple,QString)), myStackAnalyzer,SLOT(loadGridScan(QString,LocationSimple,QString)));
    connect(runSAStuff, SIGNAL(clicked()),this,SLOT(runSAStuffClicked()));
    connect(this, SIGNAL(processSmartScanSig(QString)), myStackAnalyzer, SLOT(processSmartScan(QString)));
    connect(myStackAnalyzer, SIGNAL(combinedSWC(QString)),this, SLOT(combinedSmartScan(QString)));
    connect(myStackAnalyzer,SIGNAL(loadingDone(Image4DSimple*)),this,SLOT(loadingDone(Image4DSimple*)));
    connect(this, SIGNAL(callSALoadMOST(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool)), myStackAnalyzer, SLOT(loadScan_MOST(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool)));
    connect(channelChoiceComboB,SIGNAL(currentIndexChanged(QString)),myStackAnalyzer,SLOT(updateChannel(QString)));
    //communicate with NoteTaker:
    connect(this, SIGNAL(noteStatus(QString)), myNotes, SLOT(status(QString)));


    // communication with targetList:
    connect(this,SIGNAL(updateTable(LandmarkList,QList<LandmarkList>)),&myTargetTable, SLOT(updateTargetTable(LandmarkList,QList<LandmarkList>)));

    // communicate with eventLogger:

    connect(this,SIGNAL(eventSignal(QString)), myEventLogger, SLOT(logEvent(QString)));

}


void S2UI::createTargetList(){

    //  myTargetTable.show();


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
    float leftEdge = roiXEdit->text().toFloat() -roiXWEdit->text().toFloat()/2.0;
    float topEdge = roiYEdit->text().toFloat() - roiYWEdit->text().toFloat()/2.0;
    newRect =  roiGS->addRect(leftEdge,topEdge,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat());
    //newRect =  roiGS->addRect(uiS2ParameterMap[1].getCurrentValue()*10,uiS2ParameterMap[2].getCurrentValue()*10,uiS2ParameterMap[13].getCurrentValue(),uiS2ParameterMap[14].getCurrentValue());

}

void S2UI::updateLocalRemote(bool state){
    isLocal = state;
    status(QString("isLocal ").append(QString::number(isLocal)));
    QString timeString = QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm");
    QString topDirStr = QString("F:/testData/");
    QSettings settings("HHMI", "Vaa3D");

    if (isLocal){
        myController.hostLineEdit->setText(QString("local"));
        myPosMon.hostLineEdit->setText(QString("local"));
    }else{
        myController.hostLineEdit->setText(QString("10.128.48.53"));
        myPosMon.hostLineEdit->setText(QString("10.128.48.53"));

        if (!settings.contains("s2_topDir")){
            topDirStr = QFileDialog::getExistingDirectory(this, tr("Choose save directory..."),
                                                          "/",
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);
            settings.setValue("s2_topDir",topDirStr);
        }else{
            topDirStr =  settings.value("s2_topDir").value<QString>();
        }



    }


    if (resetDir){
    topDirStr = QFileDialog::getExistingDirectory(this, tr("Choose save directory..."),
                                                  "/",
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);
    settings.setValue("s2_topDir",topDirStr);
    }

    QDir topDir = QDir(topDirStr);
    topDir.mkdir(timeString);



    saveDir =QDir(topDir.absolutePath().append("/").append(timeString));
    sessionDir = saveDir;
    scanDataFileString = saveDir.absolutePath().append("/").append("_0_scanData.txt");
    eventLogString = QFileInfo(scanDataFileString).absoluteDir().absolutePath().append(QDir::separator()).append( QFileInfo(scanDataFileString).baseName()).append("eventData.txt");

    myNotes->setSaveDir(saveDir); // this gets saved in the parent directory.  scanDataFileString will get modified for each scan
    resetDir=true;
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

    for (int jj=0; jj<=20; jj++){
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
    runSAStuff = new QPushButton(tr("process smartScan files"));
    QLabel * labeli = new QLabel(tr("background threshold = "));
    QSpinBox *bkgSpnBx = new QSpinBox(0);
    s2Label->setText("input file path:");
    bkgSpnBx->setMaximum(255);
    bkgSpnBx->setMinimum(0);
    bkgSpnBx->setValue(30);
    bkgSpnBx->setObjectName("bkgSpinBox");

    QLabel * labelInterrupt = new QLabel(tr("&notify after each trace"));
    QCheckBox *interruptCB = new QCheckBox;
    useGSDTCB = new QCheckBox;
    QLabel * labelGSDT = new QLabel(tr("use &GSDT in APP2"));
    labelGSDT->setBuddy(useGSDTCB);
    useGSDTCB->setChecked(true);


    runContinuousCB = new QCheckBox;
    runContinuousCB->setChecked(true);
    QLabel* runContinuousCBLabel = new QLabel(tr("&continuous imaging"));
    runContinuousCBLabel->setBuddy(runContinuousCB);


    gridScanCB = new QCheckBox;
    gridScanCB->setChecked(false);
    QLabel * gridScanCBLabel = new QLabel(tr("&Grid Scan"));
    gridScanCBLabel->setBuddy(gridScanCB);


    QLabel * gridSizeSBLabel = new QLabel(tr("Grid Size"));
    gridSizeSB = new QSpinBox;
    gridSizeSB->setMinimum(3);
    gridSizeSB->setMaximum(7);
    gridSizeSB->setSingleStep(2);
    gridSizeSBLabel->setBuddy(gridSizeSB);


    labelInterrupt->setBuddy(interruptCB);
    interruptCB->setObjectName("interruptCB");
    overlapSpinBox = new QSpinBox;
    overlapSpinBox->setSuffix(" percent ");
    overlapSpinBox->setMinimum(0);
    overlapSpinBox->setMaximum(20 );
    overlapSpinBox->setValue(10);
    overlapSBLabel = new QLabel;
    overlapSBLabel->setText(tr("tile &overlap: "));
    overlapSBLabel->setBuddy(overlapSpinBox);
    overlap = 0.01* ((float) overlapSpinBox->value());




    tracingMethodComboB = new QComboBox;
    tracingMethodComboB->addItem("MOST");
    tracingMethodComboB->addItem("APP2");
    tracingMethodComboB->setCurrentIndex(0);
    QLabel * tracingMethodComboBLabel = new QLabel(tr("Tracing Method: "));




    channelChoiceComboB = new QComboBox;
    channelChoiceComboB->addItem("Ch1");
    channelChoiceComboB->addItem("Ch2");
    channelChoiceComboB->setCurrentIndex(1);
    QLabel * channelChoiceComboBLabel = new QLabel(tr("Color Channel: "));


    analysisRunning = new QLabel(tr("0"));
    QLabel * analysisRunningLable = new QLabel(tr("tiles being analyzed"));



    tPL->addWidget(labeli,0,0);
    tPL->addWidget(bkgSpnBx,0,1);
    tPL->addWidget(s2Label,1,0);
    tPL->addWidget(s2LineEdit,1,1);
    tPL->addWidget(startStackAnalyzerPB,2,0,1,2);
    tPL->addWidget(runSAStuff,3,0,1,2);
    tPL->addWidget(overlapSpinBox,4,1);
    tPL->addWidget(overlapSBLabel,4,0);
    tPL->addWidget(labelInterrupt,5,0);
    tPL->addWidget(interruptCB, 5,1);
    tPL->addWidget(labelGSDT,6,0);
    tPL->addWidget(useGSDTCB,6,1);
    tPL->addWidget(runContinuousCBLabel,7,0);
    tPL->addWidget(runContinuousCB,7,1);

    tPL->addWidget(gridScanCBLabel,8,0);
    tPL->addWidget(gridScanCB,8,1);
    tPL->addWidget(gridSizeSBLabel,9,0);
    tPL->addWidget(gridSizeSB,9,1);

    tPL->addWidget(analysisRunningLable,10,1);
    tPL->addWidget(analysisRunning,10,0);
    tPL->addWidget(tracingMethodComboBLabel,11,0);
    tPL->addWidget(tracingMethodComboB, 11, 1);
    tPL->addWidget(channelChoiceComboBLabel,12,0);
    tPL->addWidget(channelChoiceComboB,12,1);
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
    waitingForFile = 1;
    QTimer::singleShot(0, &myController, SLOT(startScan()));
    float leftEdge = roiXEdit->text().toFloat() -roiXWEdit->text().toFloat()/2.0;
    float topEdge = roiYEdit->text().toFloat() - roiYWEdit->text().toFloat()/2.0;
    roiGS->addRect(leftEdge,topEdge,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen::QPen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));

}


void S2UI::loadScan(){
    QTimer::singleShot(0, this, SLOT(loadLatest()));
}


void S2UI::toLoad(){
    loadScanFromFile(s2LineEdit->text());
}

void S2UI::loadForSA(){
    LandmarkList seedList;
    LocationSimple tileLocation;

    if (smartScanStatus == 1){
        seedList = tipList.at(loadScanNumber);
        tileLocation.x = scanList.value(loadScanNumber).x;// this is in pixels, using the expected origin
        tileLocation.y = scanList.value(loadScanNumber).y;
    }else{
        tileLocation.x = 0;
        tileLocation.y = 0;
        seedList.clear();
    }
    bool isSoma = scanNumber==0;
    qDebug()<<workerThread->currentThreadId();
    QTimer::singleShot(0,this, SLOT(processingStarted()));
    if (tracingMethodComboB->currentIndex()==0){
        emit callSALoadMOST(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma);

    }
    if (tracingMethodComboB->currentIndex()==1){
        emit callSALoad(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma);

    }
}

void S2UI::loadScanFromFile(QString fileString){
    QString latestString = fileString;
    QFileInfo imageFileInfo = QFileInfo(latestString);
    QString windowString = latestString;
    NeuronTree nt;
    LandmarkList newTargetList;

    if (imageFileInfo.isReadable()){


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


        Image4DSimple* total4DImage = new Image4DSimple;
        total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);

        LocationSimple tileLocation;
        tileLocation.x = scanList.value(loadScanNumber).x;// this is in pixels, using the expected origin
        tileLocation.y = scanList.value(loadScanNumber).y;

        QString swcString = saveDir.absolutePath();
        swcString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".swc");


        QFile saveTextFile;
        saveTextFile.setFileName(scanDataFileString);// add currentScanFile
        if (!saveTextFile.isOpen()){
            if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
                qDebug()<<"unable to save file!";
                return;}     }
        QTextStream outputStream;
        outputStream.setDevice(&saveTextFile);


        total4DImage->setOriginX(tileLocation.x);
        total4DImage->setOriginY(tileLocation.y);
        qDebug()<<total4DImage->getOriginX();

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDir.absolutePath();

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);



        if (waitingForOverview){windowString = "s2 Preview Image";
            previewWindow = cb->newImageWindow();
            cb->setImageName(previewWindow,windowString);
            cb->setImage(previewWindow, total4DImage);
            cb->open3DWindow(previewWindow);
            cb->updateImageWindow(previewWindow);
            havePreview = true;
            waitingForOverview = false;
        }else{
            v3dhandle newwin = cb->newImageWindow();
            cb->setImageName(newwin,windowString);
            cb->setImage(newwin, total4DImage);
            if (smartScanStatus==1){
                cb->open3DWindow(newwin);
                cb->setSWC(newwin,nt);
                cb->setLandmark(newwin,newTargetList);
                cb->pushObjectIn3DWindow(newwin);}

            cb->updateImageWindow(newwin);
        }



    }else{  //initial string is not readable
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
    overViewPixelToScanPixel =  overviewMicronsPerPixel/uiS2ParameterMap[8].getCurrentValue();

}



void S2UI::updateString(QString broadcastedString){
}


//  -------  smart scanning stuffs   --------
//  -----------------------------------------


void S2UI::startAllTargets(){
    qDebug()<<"startAllTargets";
    if (allTargetStatus ==1){
        allTargetStatus =0;
        runAllTargetsPB->setText("Scan All Targets");
        smartScanStatus = 0;
        startSmartScanPB->setText("smartScan");
        allROILocations->clear();
        scanList.clear();
        allTipsList->clear();
        tipList.clear();
        saveTextFile.close();
        emit eventSignal("finishedMultiTarget");
        return;
    }else{
        emit eventSignal("startMultiTarget");

        runAllTargetsPB->setText("cancel All Targets");
        targetIndex = -1;
        allTargetStatus = 1;// running alltargetscan
        QTimer::singleShot(0, this, SLOT(handleAllTargets()));
    }
}
void S2UI::handleAllTargets(){
    qDebug()<<"handleAllTargets";
    targetIndex++;
    allROILocations->clear();
    if (targetIndex>=allTargetLocations.length()){
        v3d_msg("finished with multi-target scan",true);
        runAllTargetsPB->setText("Scan All Targets");
        smartScanStatus = 0;
        allTargetStatus = 0;
        startSmartScanPB->setText("smartScan");
        allROILocations->clear();
        allTipsList->clear();
        saveTextFile.close();
        emit eventSignal("finishedMultiTarget");
        myEventLogger->processEvents(eventLogString);
        return;
    }
    status("starting all targets");
    QTimer::singleShot(1000, this, SLOT(startingSmartScan()));}


void S2UI::startingSmartScan(){
    numProcessing=0;
    scanVoltageConversion = uiS2ParameterMap[8].getCurrentValue()/uiS2ParameterMap[17].getCurrentValue();  // convert from pixels to microns and to galvo voltage:

    if (gridScanCB->isChecked()){
        gridScanStatus = 1;

        emit eventSignal("startGridScan");
        waitingForLast = false;
        QString timeString = QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm");
        sessionDir.mkdir(timeString);
        saveDir = QDir(sessionDir.absolutePath().append("/").append(timeString));

        scanDataFileString = saveDir.absolutePath().append("/").append("scanDataGrid.txt");
        eventLogString = QFileInfo(scanDataFileString).absoluteDir().absolutePath().append(QDir::separator()).append( QFileInfo(scanDataFileString).baseName()).append("eventData.txt");

        status(scanDataFileString);
        saveTextFile.setFileName(scanDataFileString);// add currentScanFile
        if (!saveTextFile.isOpen()){
            if (!saveTextFile.open(QIODevice::Text|QIODevice::WriteOnly)){
                qDebug()<<"couldnt open file"<<scanDataFileString;
                return;}     }

        outputStream.setDevice(&saveTextFile);
        if (allROILocations->isEmpty()){
            scanList.clear();
            scanNumber = 0;
            loadScanNumber = 0;
            status("starting smartScan...");
            LocationSimple startLocation ;
            if (allTargetStatus ==0){
                startLocation = LocationSimple(uiS2ParameterMap[18].getCurrentValue()/uiS2ParameterMap[8].getCurrentValue(),
                        uiS2ParameterMap[19].getCurrentValue()/uiS2ParameterMap[9].getCurrentValue(),
                        0);

            }else{
                startLocation = allTargetLocations[targetIndex];
            }




            startLocation.mass = 0;
            startLocation.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
            startLocation.ev_pc2 = uiS2ParameterMap[12].getCurrentValue();
            float tileSize = uiS2ParameterMap[11].getCurrentValue();
            int minGrid = -(gridSizeSB->value()-1)/2;
            int maxGrid = -minGrid+1;
            for (int i=minGrid; i<maxGrid;i++){
                for (int j=minGrid; j<maxGrid;j++){
                    LocationSimple gridLoc;
                    gridLoc.x = startLocation.x+ ((float)i * (1.0-overlap))* tileSize;
                    gridLoc.y = startLocation.y+ ((float)j * (1.0-overlap))* tileSize;
                    gridLoc.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
                    gridLoc.ev_pc2 = uiS2ParameterMap[12].getCurrentValue();
                    qDebug()<<"grid x = "<< gridLoc.x<<" grid y = "<<gridLoc.y;
                    allROILocations->append(gridLoc);

                }
            }


            //allScanLocations.append(allROILocations);
            if (allTargetStatus ==0)   allTargetLocations.append(startLocation); // keep track of targets, even when not using the multi-target sequence

            if (runContinuousCB->isChecked()){
                qDebug()<<"allROILocations length "<<allROILocations->length();
                s2ROIMonitor();
            }else{        qDebug()<<"headed to smartscanHandler";
                QTimer::singleShot(10,this, SLOT(smartScanHandler()));}
        }
        return;
    }

    qDebug()<<"starting smartscan";
    if (smartScanStatus==1){
        smartScanStatus=0;
        waitingForLast = false;
        startSmartScanPB->setText("smartScan");
        allROILocations->clear();
        allTipsList->clear();
        saveTextFile.close();
        emit eventSignal("finishedSmartScan");

        return;
    }


    emit eventSignal("startSmartScan");
    smartScanStatus = 1;
    waitingForLast = false;
    QString timeString = QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm");
    sessionDir.mkdir(timeString);
    saveDir = QDir(sessionDir.absolutePath().append("/").append(timeString));

    scanDataFileString = saveDir.absolutePath().append("/").append("scanData.txt");
    eventLogString = QFileInfo(scanDataFileString).absoluteDir().absolutePath().append(QDir::separator()).append( QFileInfo(scanDataFileString).baseName()).append("eventData.txt");

    status(scanDataFileString);
    saveTextFile.setFileName(scanDataFileString);// add currentScanFile
    if (!saveTextFile.isOpen()){
        if (!saveTextFile.open(QIODevice::Text|QIODevice::WriteOnly)){
            qDebug()<<"couldnt open file"<<scanDataFileString;
            return;}     }

    outputStream.setDevice(&saveTextFile);

    startSmartScanPB->setText("cancel smartScan");
    if (allROILocations->isEmpty()){
        scanList.clear();
        tipList.clear();
        scanNumber = 0;
        loadScanNumber = 0;
        status("starting smartScan...");
        LocationSimple startLocation ;
        if (allTargetStatus ==0){
            startLocation = LocationSimple(uiS2ParameterMap[18].getCurrentValue()/uiS2ParameterMap[8].getCurrentValue(),
                    uiS2ParameterMap[19].getCurrentValue()/uiS2ParameterMap[9].getCurrentValue(),
                    0);

        }else{
            startLocation = allTargetLocations[targetIndex];
        }




        startLocation.mass = 0;
        startLocation.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
        startLocation.ev_pc2 = uiS2ParameterMap[12].getCurrentValue();
        allROILocations->append(startLocation);
        //allScanLocations.append(allROILocations);
        if (allTargetStatus ==0)   allTargetLocations.append(startLocation); // keep track of targets, even when not using the multi-target sequence

        if (runContinuousCB->isChecked()){
            s2ROIMonitor();
        }else{        qDebug()<<"headed to smartscanHandler";
            QTimer::singleShot(10,this, SLOT(smartScanHandler()));}
    }
    // append text to noteTaker
    //

}

void S2UI::handleNewLocation(QList<LandmarkList> newTipsList, LandmarkList newLandmarks,  Image4DSimple* mip){

    emit eventSignal("finishedAnalysis");
     QTimer::singleShot(0,this, SLOT(processingFinished()));
    qDebug()<<"back in S2UI with new locations";

    status(QString("got ").append(QString::number( newLandmarks.length())).append("  new landmarks associated with ROI "). append(QString::number(scanNumber)));
    for (int i = 0; i<newLandmarks.length(); i++){
        if (!newTipsList.value(i).empty()){
            status(QString("x= ").append(QString::number(newLandmarks.value(i).x)).append(" y = ").append(QString::number(newLandmarks.value(i).y)).append(" z= ").append(QString::number(newLandmarks.value(i).z)));
//            for (int k=0; k<newTipsList.value(i).length();k++){
//                status(QString("tipList point ").append(QString::number(k)).append(" x =").append(QString::number(newTipsList.value(i).value(k).x)).append(" y = ").append(QString::number(newTipsList.value(i).value(k).y)));
//            }

            if (!isDuplicateROI(newLandmarks.value(i))){
                allROILocations->append(newLandmarks.value(i));
                allTipsList->append(newTipsList.value(i));
            }else{
                status("already scanned here!");
            }}
    }
    loadMIP(scanNumber, mip);
    scanNumber++;
    myNotes->save();

    QTimer::singleShot(10,this, SLOT(smartScanHandler()));
}

bool S2UI::isDuplicateROI(LocationSimple inputLocation){
    //check against locations already scanned
    for (int i=0; i<scanList.length(); i++){
        if ((qAbs(inputLocation.x - scanList[i].x)<50.0) && (qAbs(inputLocation.y - scanList[i].y)<50.0)){
            return true;}
    }
    // and locations already queued!
    for (int i=0; i< allROILocations->length(); i++){
        if ((qAbs(inputLocation.x - allROILocations->at(i).x)<50.0) && (qAbs(inputLocation.y - allROILocations->at(i).y)<50.0)){
            return true;}
    }
    return false;
}
void S2UI::smartScanHandler(){

    if (smartScanStatus!=1){
        status("smartScan aborted");
        scanNumber = 0;
        loadScanNumber = 0;
        saveTextFile.close();
        emit processSmartScanSig(scanDataFileString);
        if (allTargetStatus ==1){
            QTimer::singleShot(0, this, SLOT(handleAllTargets()));
        }else{        myEventLogger->processEvents(eventLogString);}
        return;
    }
    if ((allROILocations->isEmpty())&&(!waitingForLast)&&(scanList.length()==(scanNumber))){//scanNumber is incremented AFTER the tracing results come in
        if (allTargetStatus !=1){  v3d_msg("Finished with smartscan !",true);}
        saveTextFile.close();
        smartScanStatus = 0;
        emit processSmartScanSig(scanDataFileString);
        emit eventSignal("finishedSmartScan");
        myEventLogger->processEvents(eventLogString);
        if ((allTargetStatus ==1)&&(targetIndex<allTargetLocations.length())){
            QTimer::singleShot(0, this, SLOT(handleAllTargets()));
            return;
        }

    }

    status(QString("we now have a total of ").append(QString::number( allROILocations->length())).append(" target ROIs..."));
    qDebug()<<QString("we now have a total of ").append(QString::number( allROILocations->length())).append(" target ROIs...");
    for (int i = 0; i<allROILocations->length(); i++){
        status(QString("x= ").append(QString::number(allROILocations->value(i).x)).append(" y = ").append(QString::number(allROILocations->value(i).y)).append(" z= ").append(QString::number(allROILocations->value(i).z)));
    }

    if ((!allROILocations->isEmpty()) || (waitingForLast)){


        if (runContinuousCB->isChecked()){
            qDebug()<<"letting s2ROIMonitor initiate scans";
        }else{
            LocationSimple nextLocation = allROILocations->first();
            LandmarkList  nextLandmarkList;
            if (allTipsList->isEmpty()){
                qDebug()<<"no incoming tip locations";
                // leave nextLandmarkList empty and don't touch allTipsList
            }else{
                nextLandmarkList = allTipsList->first();
                allTipsList->removeFirst();
            }
            tipList.append(nextLandmarkList);
            allROILocations->removeFirst();
            qDebug()<<nextLocation.x;
            moveToROI(nextLocation);
            nextLocation.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
            nextLocation.ev_pc2 = uiS2ParameterMap[12].getCurrentValue();
            waitingForFile = 1;
            scanList.append(nextLocation);
            if (targetIndex < allScanLocations.length()){
                allScanLocations[targetIndex].append(nextLocation);
            }else{
                LandmarkList starterList;
                starterList.append(nextLocation);
                allScanLocations.append(starterList);
            }
            emit updateTable(allTargetLocations,allScanLocations);
            emit eventSignal("startZStack");
            QTimer::singleShot(100, &myController, SLOT(startZStack())); //hardcoded delay here... not sure
            // how to make this more eventdriven. maybe  wait for move to finish.
            status(QString("start next ROI at x = ").append(QString::number(nextLocation.x)).append("  y = ").append(QString::number(nextLocation.y)));
        }
    }

}

void S2UI::s2ROIMonitor(){ // this is continuous acquisition mode
    if (!runContinuousCB->isChecked()) return;

    waitingForLast = allROILocations->length()==1;

    if ((!allROILocations->isEmpty())&&(waitingForFile<1)){
        LandmarkList  nextLandmarkList;
        if (allTipsList->isEmpty()){
            qDebug()<<"no incoming tip locations";
            // leave nextLandmarkList empty and don't touch allTipsList
        }else{
            nextLandmarkList = allTipsList->first();
            allTipsList->removeFirst();
        }
        tipList.append(nextLandmarkList);
        LocationSimple nextLocation = allROILocations->first();
        allROILocations->removeFirst();
        qDebug()<<nextLocation.x;
        moveToROI(nextLocation);
        nextLocation.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
        nextLocation.ev_pc2 = uiS2ParameterMap[12].getCurrentValue();
        waitingForFile = 1;
        scanList.append(nextLocation);
        if (targetIndex < allScanLocations.length()){
            allScanLocations[targetIndex].append(nextLocation);
        }else{
            LandmarkList starterList;
            starterList.append(nextLocation);
            allScanLocations.append(starterList);
        }


        emit updateTable(allTargetLocations,allScanLocations);
        status(QString("start next ROI at x = ").append(QString::number(nextLocation.x)).append("  y = ").append(QString::number(nextLocation.y)));
        emit eventSignal("startZStack");
        QTimer::singleShot(100, &myController, SLOT(startZStack()));

    }
    if ((gridScanStatus ==1) && (allROILocations->length() == 0)){gridScanStatus = -1; return;}

    if (((smartScanStatus ==1)&&(runContinuousCB->isChecked()))||(gridScanStatus==1)) {
        QTimer::singleShot(10, this, SLOT(s2ROIMonitor()));
    }
}
void S2UI::moveToROI(LocationSimple nextROI){

    if( posMonStatus){
        float nextXMicrons = nextROI.x * uiS2ParameterMap[8].getCurrentValue();  // convert from pixels to microns:
        float nextYMicrons = nextROI.y* uiS2ParameterMap[9].getCurrentValue();
        // and now to galvo voltage:
        float nextGalvoX = nextXMicrons/uiS2ParameterMap[17].getCurrentValue();
        float nextGalvoY = nextYMicrons/uiS2ParameterMap[17].getCurrentValue();

        if ((gridScanStatus==1)||(smartScanStatus==1)){
            nextGalvoX = nextROI.x*scanVoltageConversion;
            nextGalvoY = nextROI.y*scanVoltageConversion;
        }
        LocationSimple newLoc;
        newLoc.x = -nextGalvoX;
        newLoc.y = nextGalvoY;
        float leftEdge = nextXMicrons -roiXWEdit->text().toFloat()/2.0;
        float topEdge = nextYMicrons - roiYWEdit->text().toFloat()/2.0;


        emit moveToNext(newLoc);
    }else{
        status("start PosMon before moving galvos");
        smartScanStatus = -1;
    }
}


void S2UI::combinedSmartScan(QString saveFilename){
    V3dR_MainWindow * new3DWindow = NULL;
    new3DWindow = cb->createEmpty3DViewer();
    QList<NeuronTree> * new_treeList = cb->getHandleNeuronTrees_Any3DViewer (new3DWindow);
    if (!new_treeList)
    {
        v3d_msg(QString("New 3D viewer has invalid neuron tree list"));
        return;
    }
    NeuronTree resultTree;
    resultTree = readSWC_file(saveFilename);
    new_treeList->push_back(resultTree);
    cb->setWindowDataTitle(new3DWindow, "Final reconstruction");
    cb->update_NeuronBoundingBox(new3DWindow);
}




void S2UI::loadLatest(){
    if ((smartScanStatus ==1)||(gridScanStatus!=0)){
        LandmarkList seedList;
        qDebug()<<"loadlatest smartscan";
        qDebug()<<"tipList length "<<tipList.length()<<" loadScanNumber "<<loadScanNumber;
        if (!tipList.isEmpty()){

            seedList = tipList.at(loadScanNumber);
            qDebug()<<"seedList length "<<seedList.length();
        }
        LocationSimple tileLocation;
        tileLocation.x = scanList.value(loadScanNumber).x;// this is in pixels, using the expected origin
        tileLocation.y = scanList.value(loadScanNumber).y;
        bool isSoma = scanNumber==0;
        if (gridScanStatus!=0){
            emit eventSignal("startGridLoad");

            emit  callSAGridLoad(getFileString(),   tileLocation, saveDir.absolutePath() );

        }else{
            emit eventSignal("startAnalysis");
            QTimer::singleShot(0,this, SLOT(processingStarted()));
            if (tracingMethodComboB->currentIndex()==0){
                emit callSALoadMOST(getFileString(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma);

            }
            if (tracingMethodComboB->currentIndex()==1){
                emit callSALoad(getFileString(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma);

            }        }

        loadScanNumber++;
    }else{
        loadScanFromFile(getFileString());
    }
    // if there's an .xml file in the filestring directory, copy it to the save directory:
    QDir xmlDir = QFileInfo(getFileString()).absoluteDir();
    QStringList filterList;
    filterList.append(QString("*.xml"));
    status("xml directory: "+xmlDir.absolutePath());
    xmlDir.setNameFilters(filterList);
    QStringList fileList = xmlDir.entryList();
    if (!fileList.isEmpty()){
        QFileInfo xmlInfo = QFileInfo(xmlDir.absoluteFilePath( fileList.at(0)));
        QFile::copy( xmlInfo.absoluteFilePath(),saveDir.absolutePath().append(QDir::separator()).append(xmlInfo.fileName()));
        qDebug()<<"copy finished from "<<xmlInfo.absoluteFilePath()<<" to "<< saveDir.absolutePath().append(QDir::separator()).append(xmlInfo.fileName());

    }else{
        qDebug()<<"no xml file available";
    }



}


void S2UI::loadingDone(Image4DSimple *mip){


    emit eventSignal("finishedGridLoad");

    if (gridScanStatus>0){
    loadMIP(loadScanNumber, mip);
}
    if ((gridScanStatus ==-1)&&(waitingForFile<1)){
        emit eventSignal("finishedGridScan");
        gridScanStatus = 0;
        emit processSmartScanSig(scanDataFileString);
        QTimer::singleShot(0, this, SLOT(handleAllTargets()));

    }

}

// ------------------------------------


void S2UI::collectOverview(){
    // collect overview stack at lowest mag and high spatial resolution. requires galvo mode.
    // this is called at the same time that a signal is sent to the controller to setup for an overview

    // so start my monitor to see when it's in ready state:
    overviewCycles = 0;
    status("start overview");
    emit eventSignal("startZStack");

    QTimer::singleShot(0, this, SLOT(overviewHandler()));
}

void S2UI::overviewHandler(){
    bool readyForOverview =
            ((int) uiS2ParameterMap[12].getCurrentValue() ==1)&&
            ((int) uiS2ParameterMap[10].getCurrentValue() == 512)&&
            ((int) uiS2ParameterMap[11].getCurrentValue() == 512);


    bool overViewTimedOut = overviewCycles >50;

    if (overViewTimedOut){
        status("overview timeout!");
        return;
    }
    if (readyForOverview){
        // set up 3-plane z stack here?
        overviewMicronsPerPixel = uiS2ParameterMap[8].getCurrentValue();
        waitingForOverview = true;
        QTimer::singleShot(100, startZStackPushButton, SLOT(click()));
        status("starting single scan");
        return;
    }
    if (!readyForOverview&&!overViewTimedOut){
        overviewCycles++;
        QTimer::singleShot(100, this, SLOT(overviewHandler()));
    }
}


void S2UI::startingZStack(){
    waitingForFile = 1;
    QTimer::singleShot(100, &myController, SLOT(startZStack()));
    status("start single z Stack");
    float leftEdge = roiXEdit->text().toFloat() -roiXWEdit->text().toFloat()/2.0;
    float topEdge = roiYEdit->text().toFloat() - roiYWEdit->text().toFloat()/2.0;
    roiGS->addRect(leftEdge,topEdge,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen::QPen(Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (smartScanStatus==1){
        QGraphicsTextItem* sequenceNumberText;

        sequenceNumberText = new QGraphicsTextItem;
        sequenceNumberText->setPos(leftEdge+10,topEdge);
        sequenceNumberText->setPlainText(QString::number(loadScanNumber));
        roiGS->addItem(sequenceNumberText);
    }

}

void S2UI::updateFileString(QString inputString){
    //this means a new file has been created.. it can be late by up to 1 full cycle of s2parametermap updating
    // but it guarantees that the acquisition is done
    // a separate poller of updated filename could be much faster.
    // final version will require much more rigorous timing- it's not clear how we'll parse out
    // the streamed image data into files...
    fileString = inputString;
    if (!isLocal){
        if (fileString.contains("Z:")){
            fileString.replace("Z:","\\AIBSDATA\\mat\\BRL\\testData");
        }
        fileString.replace("\\AIBSDATA","\\data").replace("\\","/");
    }
    fileString.append("_Cycle00001_Ch2_000001.ome.tif");
    if ((QString::compare(fileString,lastFile, Qt::CaseInsensitive)!=0)&(waitingForFile>0)){
        emit eventSignal("finishedZStack");
        waitingForFile = 0;
        QTimer::singleShot(0, this, SLOT(loadScan()));

    }
    lastFile = fileString;
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
    overlap = 0.01* ((float) value);

}


void S2UI::getCurrentParameters(){
    emit currentParameters(uiS2ParameterMap);
}



void S2UI::closeEvent(QCloseEvent *){
    myNotes->close();
    myPosMon.close();
    myController.close();

}


void S2UI::resetToOverviewPBCB(){
    status("resetting to overview mode");
    // this could also reset smartscan parameters, markers, etc.
}


void S2UI::resetToScanPBCB(){
    // so start my monitor to see when it's in ready state:
    centerGalvosPB->click();
    scanStatusWaitCycles = 0;
    QTimer::singleShot(0, this, SLOT(scanStatusHandler()));

}


void S2UI::scanStatusHandler(){
    bool readyForStack =
            ((int) uiS2ParameterMap[12].getCurrentValue() >=12.0)&&
            ((int) uiS2ParameterMap[10].getCurrentValue() == 256)&&
            ((int) uiS2ParameterMap[11].getCurrentValue() == 256);
    bool scanStatusTimedOut = scanStatusWaitCycles >50;

    if (scanStatusTimedOut){
        status("scan status timeout!");
        return;
    }

    if (!readyForStack&&!scanStatusTimedOut){
        scanStatusWaitCycles++;
        QTimer::singleShot(100, this, SLOT(scanStatusHandler()));
    }
}


void S2UI::pickTargets(){
    qDebug()<<"in pickTargets...";
    if (!havePreview){
        v3d_msg("please collect a preview image");
        return;
    }
    LandmarkList previewTargets =  cb->getLandmark(previewWindow);
    if (previewTargets.isEmpty()){
        v3d_msg("please select a target");
        return;
    }
    resetToScanPB->click();
    LocationSimple startCenter;
    QList<LandmarkList>  startingROIList;
    LandmarkList targets;
    for (int i =0; i<previewTargets.length();i++){
        LocationSimple newTarget;
        newTarget.x = (previewTargets.at(i).x-256.0)*overViewPixelToScanPixel;
        newTarget.y  = (previewTargets.at(i).y-256.0)*overViewPixelToScanPixel;
        startCenter.x = 0.0+newTarget.x;
        startCenter.y = 0.0+newTarget.y;
        startCenter.ev_pc1 = uiS2ParameterMap[10].getCurrentValue();
        startCenter.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
        targets.append(newTarget);
        LandmarkList startList;
        startList.append(startCenter);
        startingROIList.append(startList);
    }
    allTargetLocations = targets;
    emit updateTable(targets,startingROIList);
}




void S2UI::loadMIP(int imageNumber, Image4DSimple* mip){
    scaleintensity(mip,0,0,8000,double(0),double(255));
    scale_img_and_convert28bit(mip, 0, 255) ;
    QImage myMIP;
    int x = mip->getXDim();
    int y = mip->getYDim();
    V3DLONG total  =0;
    Image4DProxy<Image4DSimple> mipProx(mip);
    myMIP = QImage(x, y, QImage::Format_RGB16);
    for (V3DLONG i=0; i<x; i++){
        for (V3DLONG j=0; j<y;j++){
            myMIP.setPixel(i,j,mipProx.value8bit_at(i,j,0,0));
            total++;
        }
    }
    QGraphicsPixmapItem* mipPixmap = new QGraphicsPixmapItem(QPixmap::fromImage(myMIP));
    float xPix = scanList.value(imageNumber).x;// this is in pixels, using the expected origin
    float yPix  = scanList.value(imageNumber).y;
    mipPixmap->setScale(uiS2ParameterMap[8].getCurrentValue());
    mipPixmap->setPos((xPix-((float) x )/2.0)*uiS2ParameterMap[8].getCurrentValue(),
            (yPix-((float) x )/2.0)*uiS2ParameterMap[9].getCurrentValue());
    roiGS->addItem(mipPixmap);
}


void S2UI::processingStarted(){
    numProcessing++;
    analysisRunning->setText(QString::number(numProcessing));
}

void S2UI::processingFinished(){
    numProcessing--;
    analysisRunning->setText(QString::number(numProcessing));
}
