#include "v3d_message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <QWidget>
#include <QDialogButtonBox>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include <QThread>
#include <QDateTime>
#include <QBitArray>
#include "s2Controller.h"
#include "s2UI.h"
#include "stackAnalyzer.h"
#include "vn_app2.h"
#include <QMutex>
#include <cstring>
#include "VirtualVolume.h"
#include <QList>
#include <QString>
#include "basic_surf_objs.h"
#include <QDir>
#include "Tiff3DMngr.h"
#include <qregexp.h>
#include <math.h>
#include <qstringlist.h>

using namespace iim;
using namespace std;

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
#include <sys/time.h>
#include <unistd.h>
#endif

#if defined (Q_OS_WIN32)
#include <io.h>
#endif

S2UI::S2UI(V3DPluginCallback2 &callback, QWidget *parent):   QDialog(parent)
{
	qRegisterMetaType<LandmarkList>("LandmarkList");
	qRegisterMetaType<LocationSimple>("LocationSimple");
	qRegisterMetaType<QList<LandmarkList> >("QList<LandmarkList>");
	qRegisterMetaType<unsigned short int>("unsignedShortInt");
	qRegisterMetaType<TileInfo>("TileInfo");
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
	versionString =QString("%1").arg(GIT_CURRENT_SHA1);
#endif

	qDebug()<<"S2 git repo hash at build time = "<<versionString;
	fileString =QString("");
	lastFile = QString("");
	allROILocations = new QList<TileInfo>;
	allTipsList = new QList<LandmarkList>;

	tileSizeChoices = new QList<TileInfo>;


	previewWindow = new v3dhandle;
	havePreview = false;

	cb = &callback;

	myScanMonitor = new S2Monitor();
	myScanData = new S2ScanData();

	myStackAnalyzer = new StackAnalyzer(callback);

	myStackAnalyzer0 = new StackAnalyzer(callback);
	myStackAnalyzer1 = new StackAnalyzer(callback);
	myStackAnalyzer2 = new StackAnalyzer(callback);
	myStackAnalyzer3 = new StackAnalyzer(callback);


	myTileInfoMonitor = new TileInfoMonitor();



	s2Label = new QLabel(tr("smartScope 2"));
	s2LineEdit = new QLineEdit("");


	startPosMonButton = new QPushButton(tr("start monitor"));
	startSmartScanPB = new QPushButton(tr("SmartScan"));
	startSmartScanPB->setEnabled(false);
	startStackAnalyzerPB = new QPushButton(tr("trace single stack"));
	resetToOverviewPB = new QPushButton(tr("&RESET to preview"));
	resetToScanPB = new QPushButton(tr("RESET to &SmartScan"));
	pickTargetsPB = new QPushButton(tr("pick smartScan starting points"));
	collectZoomStackPB = new QPushButton(tr("collect zoom stack"));
	myNotes = new NoteTaker;
	myEventLogger  = new EventLogger();


	rhTabs = new QTabWidget();
	rhTabs->addTab(   createROIMonitor(), "ROI Monitor");
	rhTabs->addTab(myNotes, "status and notes");


   // createTargetList();
	zoomPixelsProduct = 2048.0; //13.0*256;
	currentTileInfo = TileInfo(zoomPixelsProduct);

	lhTabs = new QTabWidget();
	lhTabs->addTab(createS2Monitors(), "s2 Monitor");
	lhTabs->addTab(&myPosMon, "monCOM" );
	lhTabs->addTab(&myController, "s2COM");
	lhTabs->addTab(createTracingParameters(),"tracing");
	lhTabs->addTab(createROIControls(),"ROI Controls");
	lhTabs->addTab(createConfigPanel(),"Configure");
	lhTabs->addTab(createSimulator(),"Simulator");



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
	mainLayout->addWidget(collectZoomStackPB,4,0);
	//mainLayout->addWidget(startPosMonButton,3,0);
	mainLayout->addWidget(startSmartScanPB, 1,0,1,3);
	mainLayout->addWidget(runAllTargetsPB,4,2);
	mainLayout->addWidget(lhTabs, 6,0, 4, 3);
	mainLayout->addWidget(rhTabs,0,5,12,4);
	for (int ii=0; ii<mainLayout->rowCount(); ii++){
		mainLayout->setRowMinimumHeight(ii,1);
	}
	//    mainLayout->addWidget(startStackAnalyzerPB, 9, 0,1,2);
	roiGroupBox->show();


	myTargetTable = new TargetList;
	myTargetTable->show();
	targetIndex = 0;
	colorIndex = 0;
	overViewPixelToScanPixel = (.915/.229);
	overviewMicronsPerPixel  = .915;
	zStepSize = 1.0;

	hookUpSignalsAndSlots();


	workerThread = new QThread;
	myStackAnalyzer->moveToThread(workerThread);
	workerThread->start();

	workerThread0 = new QThread;
	myStackAnalyzer0->moveToThread(workerThread0);
	workerThread0->start();

	workerThread1 = new QThread;
	myStackAnalyzer1->moveToThread(workerThread1);
	workerThread1->start();

	workerThread2 = new QThread;
	myStackAnalyzer2->moveToThread(workerThread2);
	workerThread2->start();

	swcWatchThread = new QThread;
	myTileInfoMonitor->moveToThread(swcWatchThread);
	swcWatchThread->start();
	traceThreadNumber =0;

	posMonStatus = false;
	waitingForFile = 0;
	haventRunBoundingBox = true;
	waitingToStartStack = false;
	isLocal = false;
	resetDir = false;
	smartScanStatus = 0;
	gridScanStatus = 0;
	allTargetStatus = 0;    QLineEdit *roiXEdit;
	QLineEdit *roiYEdit;
	QLineEdit *roiZEdit;
	QLineEdit *roiXWEdit ;
	zoomStateOK = true;
	setLayout(mainLayout);
	setWindowTitle(tr("smartScope2 Interface"));
	updateLocalRemote(isLocal);
	updateZoomPixelsProduct(1);
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
	connect(zoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateGVZoom(int)));

	connect(localRemoteCB, SIGNAL(clicked(bool)), this, SLOT(updateLocalRemote(bool)));

	connect(resetDirPB, SIGNAL(clicked()), this, SLOT(resetDirectory()));

	connect(setLocalPathToData, SIGNAL(clicked()), this, SLOT(resetDataDir()));
	connect(overlapSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateOverlap(int)));

	connect(resetToOverviewPB, SIGNAL(clicked()),this, SLOT(resetToOverviewPBCB()));
	connect(resetToScanPB, SIGNAL(clicked()), this, SLOT(resetToScanPBCB()));

	connect(pickTargetsPB,SIGNAL(clicked()),this, SLOT(pickTargets()));
	connect(runAllTargetsPB,SIGNAL(clicked()),this,SLOT(startAllTargets()));
	connect(collectZoomStackPB, SIGNAL(clicked()), this, SLOT(collectZoomStack()));

	connect(runContinuousCB,SIGNAL(clicked()), this, SLOT(s2ROIMonitor()));

	connect(tileSizeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentZoom(int)));


	connect(zoomSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateZoomPixelsProduct(int)));
	connect(pixelsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateZoomPixelsProduct(int)));

	connect(runSAStuff, SIGNAL(clicked()),this,SLOT(runSAStuffClicked()));
	connect(startStackAnalyzerPB, SIGNAL(clicked()),this, SLOT(loadForSA()));


	connect(lipoFactorSlider,SIGNAL(valueChanged(int)), this, SLOT(updateLipoFactor(int)));
	connect(redThresholdSlider, SIGNAL(valueChanged(int)),this,SLOT(updateRedThreshold(int)));

	connect(minBlockSizeSB, SIGNAL(valueChanged(int)), this, SLOT(updateMinMaxBlock(int)));
	connect(maxBlockSizeSB, SIGNAL(valueChanged(int)), this, SLOT(updateMinMaxBlock(int)));

	connect(stackZStepSizeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateZStepSize(int)));


	connect(tryStageMove,SIGNAL(clicked()),this, SLOT(tryXYMove()));
	connect(searchPixelRadiusSB, SIGNAL(valueChanged(int)), this, SLOT(updateSearchRadiusCallback(int)));


	connect(this, SIGNAL(loadMIPSignal(double,Image4DSimple*,QString)), this, SLOT(loadMIP(double,Image4DSimple*,QString)));

	connect(this, SIGNAL(loadLatestSig(QString)),this,SLOT(loadLatest(QString)));

	// Initiate microscope simulator, MK, July 2017
	connect(tracePB, SIGNAL(clicked()), this, SLOT(initSimScope()));

	// communication with myController to send commands
	connect(startScanPushButton, SIGNAL(clicked()), this, SLOT(startScan()));
	connect(&myController,SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
	connect(centerGalvosPB, SIGNAL(clicked()), &myController, SLOT(centerGalvos()));
	connect(startZStackPushButton, SIGNAL(clicked()), this, SLOT(startingZStack()));
	connect(&myController, SIGNAL(statusSig(QString)), myNotes, SLOT(status(QString)));
	connect(this, SIGNAL(moveToNext(LocationSimple)), &myController, SLOT(initROI(LocationSimple)));
	connect(this, SIGNAL(moveToNextWithStage(LocationSimple,float,float)), &myController, SLOT(initROIwithStage(LocationSimple,float,float)));


	connect(startSmartScanPB, SIGNAL(clicked()), this, SLOT(startingSmartScan()));
	connect(collectOverviewPushButton, SIGNAL(clicked()),this, SLOT(collectOverview()));
	connect(collectOverviewPushButton, SIGNAL(clicked()),&myController, SLOT(overviewSetup()));
	// connect(resetToScanPB, SIGNAL(clicked()), &myController, SLOT(stackSetup()));
	connect(this, SIGNAL(stackSetupSig(float,float,int,int)), &myController, SLOT(stackSetup(float,float,int,int)));
	connect(this, SIGNAL(startZStackSig()), &myController, SLOT(startZStack()));

	// communication with myPosMon to monitor parameters
	connect(&myPosMon, SIGNAL(newBroadcast(QString)), this, SLOT(updateString(QString)));
	connect(&myPosMon, SIGNAL(pmStatus(bool)), this, SLOT(pmStatusHandler(bool)));
	connect(&myPosMon, SIGNAL(newS2Parameter(QMap<int,S2Parameter>)), this, SLOT(updateS2Data(QMap<int,S2Parameter>)));
	connect(this, SIGNAL(startPM()), &myPosMon, SLOT(startPosMon()));
	connect(this, SIGNAL(stopPM()), &myPosMon, SLOT(stopPosMon()));

	// communication with myPosMon and myController for SimScope
	connect(&myController, SIGNAL(shootFakeScope(LocationSimple, float, float)), &fakeScope, SLOT(paramShotFromController(LocationSimple, float, float)));
	//connect(mysimscope, SIGNAL(newcrap), myposmon, SLOT(recceives2parametermap))
	

	// communication with  myStackAnalyzer

	connect(myStackAnalyzer,  SIGNAL(analysisDone(QList<LandmarkList>, LandmarkList,Image4DSimple*, double, QString, int)), this, SLOT(handleNewLocation(QList<LandmarkList>,LandmarkList, Image4DSimple*, double, QString, int)));
	connect(myStackAnalyzer0, SIGNAL(analysisDone(QList<LandmarkList>, LandmarkList,Image4DSimple*, double, QString, int)), this, SLOT(handleNewLocation(QList<LandmarkList>,LandmarkList, Image4DSimple*, double, QString, int)));
	connect(myStackAnalyzer1, SIGNAL(analysisDone(QList<LandmarkList>, LandmarkList,Image4DSimple*, double, QString, int)), this, SLOT(handleNewLocation(QList<LandmarkList>,LandmarkList, Image4DSimple*, double, QString, int)));
	connect(myStackAnalyzer2, SIGNAL(analysisDone(QList<LandmarkList>, LandmarkList,Image4DSimple*, double, QString, int)), this, SLOT(handleNewLocation(QList<LandmarkList>,LandmarkList, Image4DSimple*, double, QString, int)));
	connect(myStackAnalyzer3, SIGNAL(analysisDone(QList<LandmarkList>, LandmarkList,Image4DSimple*, double, QString, int)), this, SLOT(handleGlobalVariables(QList<LandmarkList>,LandmarkList, Image4DSimple*, double, QString, int)));

	connect(myStackAnalyzer ,SIGNAL(loadingDone(Image4DSimple*)),this,SLOT(loadingDone(Image4DSimple*)));
	connect(myStackAnalyzer0,SIGNAL(loadingDone(Image4DSimple*)),this,SLOT(loadingDone(Image4DSimple*)));
	connect(myStackAnalyzer1,SIGNAL(loadingDone(Image4DSimple*)),this,SLOT(loadingDone(Image4DSimple*)));
	connect(myStackAnalyzer2,SIGNAL(loadingDone(Image4DSimple*)),this,SLOT(loadingDone(Image4DSimple*)));

	connect(myStackAnalyzer ,SIGNAL(bail()),this,SLOT(processingFinished()));
	connect(myStackAnalyzer0,SIGNAL(bail()),this,SLOT(processingFinished()));
	connect(myStackAnalyzer1,SIGNAL(bail()),this,SLOT(processingFinished()));
	connect(myStackAnalyzer2,SIGNAL(bail()),this,SLOT(processingFinished()));


	connect(this, SIGNAL(callSATrace(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)), myStackAnalyzer, SLOT(startTracing(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)));
	connect(this, SIGNAL(callSATrace0(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)), myStackAnalyzer0, SLOT(startTracing(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)));
	connect(this, SIGNAL(callSATrace1(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)), myStackAnalyzer1, SLOT(startTracing(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)));
	connect(this, SIGNAL(callSATrace2(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)), myStackAnalyzer2, SLOT(startTracing(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int,int)));

	connect(this, SIGNAL(updateSearchRadius(double)), myStackAnalyzer, SLOT(updateSearchRadius(double)));
	connect(this, SIGNAL(updateSearchRadius(double)), myStackAnalyzer0, SLOT(updateSearchRadius(double)));
	connect(this, SIGNAL(updateSearchRadius(double)), myStackAnalyzer1, SLOT(updateSearchRadius(double)));
	connect(this, SIGNAL(updateSearchRadius(double)), myStackAnalyzer2, SLOT(updateSearchRadius(double)));


	connect(this, SIGNAL(callSAGridLoad(QString,LocationSimple,QString)), myStackAnalyzer,SLOT(loadGridScan(QString,LocationSimple,QString)));



	connect(this, SIGNAL(processSmartScanSig(QString)), myStackAnalyzer, SLOT(processSmartScan(QString)));
	connect(myStackAnalyzer, SIGNAL(combinedSWC(QString)),this, SLOT(combinedSmartScan(QString)));



	connect(channelChoiceComboB,SIGNAL(currentIndexChanged(QString)),myStackAnalyzer,SLOT(updateChannel(QString)));
	connect(this,SIGNAL(updateLipoFactorInSA(float)), myStackAnalyzer, SLOT(updateRedAlpha(float)));
	connect(this, SIGNAL(updateRedThreshInSA(int)),myStackAnalyzer,SLOT(updateRedThreshold(int)));
	connect(chooseLipoMethod, SIGNAL(currentIndexChanged(int)), myStackAnalyzer, SLOT(updateLipoMethod(int)));
	connect(this,SIGNAL(updateMinMaxBlockSizes(int,int)), myStackAnalyzer, SLOT(updateGlobalMinMaxBlockSizes(int,int)));




	connect(channelChoiceComboB,SIGNAL(currentIndexChanged(QString)),myStackAnalyzer0,SLOT(updateChannel(QString)));
	connect(this,SIGNAL(updateLipoFactorInSA(float)), myStackAnalyzer0, SLOT(updateRedAlpha(float)));
	connect(this, SIGNAL(updateRedThreshInSA(int)),myStackAnalyzer0,SLOT(updateRedThreshold(int)));
	connect(chooseLipoMethod, SIGNAL(currentIndexChanged(int)), myStackAnalyzer0, SLOT(updateLipoMethod(int)));
	connect(this,SIGNAL(updateMinMaxBlockSizes(int,int)), myStackAnalyzer0, SLOT(updateGlobalMinMaxBlockSizes(int,int)));





	connect(channelChoiceComboB,SIGNAL(currentIndexChanged(QString)),myStackAnalyzer1,SLOT(updateChannel(QString)));
	connect(this,SIGNAL(updateLipoFactorInSA(float)), myStackAnalyzer1, SLOT(updateRedAlpha(float)));
	connect(this, SIGNAL(updateRedThreshInSA(int)),myStackAnalyzer1,SLOT(updateRedThreshold(int)));
	connect(chooseLipoMethod, SIGNAL(currentIndexChanged(int)), myStackAnalyzer1, SLOT(updateLipoMethod(int)));
	connect(this,SIGNAL(updateMinMaxBlockSizes(int,int)), myStackAnalyzer1, SLOT(updateGlobalMinMaxBlockSizes(int,int)));





	connect(channelChoiceComboB,SIGNAL(currentIndexChanged(QString)),myStackAnalyzer2,SLOT(updateChannel(QString)));
	connect(this,SIGNAL(updateLipoFactorInSA(float)), myStackAnalyzer2, SLOT(updateRedAlpha(float)));
	connect(this, SIGNAL(updateRedThreshInSA(int)),myStackAnalyzer2,SLOT(updateRedThreshold(int)));
	connect(chooseLipoMethod, SIGNAL(currentIndexChanged(int)), myStackAnalyzer2, SLOT(updateLipoMethod(int)));
	connect(this,SIGNAL(updateMinMaxBlockSizes(int,int)), myStackAnalyzer2, SLOT(updateGlobalMinMaxBlockSizes(int,int)));









// communicate with my swc file monitor
	connect(this,SIGNAL(waitForDuplicate(TileInfo,LandmarkList,int,int,int,QString)),myTileInfoMonitor,SLOT(addTileData(TileInfo,LandmarkList,int,int,int,QString)));
	connect(myTileInfoMonitor,SIGNAL(foundTile(TileInfo,LandmarkList,int,int,int)),this, SLOT(loadDuplicateTile(TileInfo,LandmarkList,int,int,int)));













	//communicate with NoteTaker:
	connect(this, SIGNAL(noteStatus(QString)), myNotes, SLOT(status(QString)));


	// communication with targetList:
	connect(this,SIGNAL(updateTable(QList<TileInfo>,QList<LandmarkList>)),myTargetTable, SLOT(updateTargetTable(QList<TileInfo>,QList<LandmarkList>)));

	// communicate with eventLogger:

	connect(this,SIGNAL(eventSignal(QString)), myEventLogger, SLOT(logEvent(QString)));


}

/*
void S2UI::createTargetList(){

	//  myTargetTable.show();
	qDebug()<<"empty S2UI::createTargetList method";

}

*/

void S2UI::initializeROISizes(){
	tileSizeChoices = new QList<TileInfo>;
	TileInfo myTileInfo = TileInfo(zoomPixelsProduct);
	myTileInfo.setZoomPos(1);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(2);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(4);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(6);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(8);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(10);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(13);
	tileSizeChoices->append(myTileInfo);

	myTileInfo.setZoomPos(16);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(24);
	tileSizeChoices->append(myTileInfo);
	myTileInfo.setZoomPos(32);
	tileSizeChoices->append(myTileInfo);

}


QGroupBox *S2UI::createROIMonitor(){
	roiGroupBox = new QGroupBox(tr("ROI Monitor"));
	gl = new QGridLayout();
	roiGS = new QGraphicsScene();
	roiGS->setObjectName("roiGS");
	roiGS->setBackgroundBrush(Qt::black);
	roiGV = new QGraphicsView();
	roiGV->setObjectName("roiGV");
	roiGV->setScene(roiGS);
	roiRect = QRectF(-250, -250, 500, 500);
	//roiGS->addRect(roiRect,QPen::QPen(Qt::gray, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin), QBrush::QBrush(Qt::gray));
	newRect = roiGS->addRect(0,0,50,50);
	//roiGV->setViewportUpdateMode(QGraphicsView::FullViewportUpdate)  ;
	roiGV->adjustSize();
	roiGV->setDragMode(QGraphicsView::ScrollHandDrag);

	originalTransform = roiGV->transform();

	gl->addWidget(roiGV,0,0,5,4);
	roiClearPB = new QPushButton(tr("clear ROIs"));



	gl->addWidget(roiClearPB, 5,0);

	zoomSlider = new QSlider();
	zoomSlider->setOrientation(Qt::Horizontal);
	zoomSlider->setMaximum(500);
	zoomSlider->setMinimum(1);
	zoomSlider->setValue(50);

	gl->addWidget(zoomSlider,5,1);


	roiGroupBox->setLayout(gl);
	return roiGroupBox;
}

void S2UI::updateROIPlot(QString ignore){
	//roiRect.moveLeft(roiXEdit->text().toFloat());
	//roiRect.setY(roiYEdit->text().toFloat());
	//qDebug()<<"y="<<roiYEdit->text().toFloat();
	roiGS->removeItem(newRect);
	float leftEdge = roiXEdit->text().toFloat() -roiXWEdit->text().toFloat()/2.0+uiS2ParameterMap[5].getCurrentValue();
	float topEdge = roiYEdit->text().toFloat() - roiYWEdit->text().toFloat()/2.0+uiS2ParameterMap[6].getCurrentValue();
	newRect =  roiGS->addRect(leftEdge,topEdge,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen(Qt::blue));

	//newRect =  roiGS->addRect(uiS2ParameterMap[1].getCurrentValue()*10,uiS2ParameterMap[2].getCurrentValue()*10,uiS2ParameterMap[13].getCurrentValue(),uiS2ParameterMap[14].getCurrentValue());


	//this value for x and y stage is correct (i.e. it moves to the right location in the sample as the microscope galvo or stage changes.
	//however, the x position of the overview tile location is mirrored in x about the stage x = 0 axis.  this error propagates into subsequent locations some of the time!

}

void S2UI::updateGVZoom(int sliderValue){
	qreal xyscale =1.0;
	xyscale = qreal( sliderValue) / 50.0;
	QTransform newTransform;
	newTransform = originalTransform;
	newTransform.scale(xyscale,xyscale);
	roiGV->setTransform(newTransform);
}

void S2UI::resetDirectory(){
	resetDir = true;
	updateLocalRemote(isLocal);
}


void S2UI::resetDataDir(){
	QSettings settings("HHMI", "Vaa3D");

	QString localDataString = QFileDialog::getExistingDirectory(this, tr("Choose local data directory..."),
																"/",
																QFileDialog::ShowDirsOnly
																| QFileDialog::DontResolveSymlinks);
	settings.setValue("s2_dataDir",localDataString);

	localDataDirectory = QDir(localDataString);
	localDataDir->setText(localDataString);

}

void S2UI::handleGlobalVariables(QList<LandmarkList> newTipsList, LandmarkList newLandmarks,  Image4DSimple* mip, double scanIndex, QString tileSaveString, int tileStatus){
	int i=0;
	for (i =0; i<newTipsList.length(); i++){
		LandmarkList iList = newTipsList[i];
		if(sqrt(pow((allTargetList.at(0).x-newLandmarks.at(i).x),2)+pow((allTargetList.at(0).y-newLandmarks.at(i).y),2))>1)
		{
			allTargetList.push_back(newLandmarks.at(i));
			myallTipsList.push_back(iList);
		}

	}
}


void S2UI::initSimScope()
{
	myController.mode = offline;

	QStringList initialParam;

	QString m_InputfolderName = QFileDialog::getExistingDirectory(this, QObject::tr("Choose the directory of the input images"), QDir::currentPath(), QFileDialog::ShowDirsOnly);
	if (m_InputfolderName.isEmpty()) return;
	else initialParam.push_back(m_InputfolderName);

	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker File"), "", QObject::tr("Supported file (*.marker *.MARKER)"));
	if (fileOpenName.isEmpty()) return;
	else initialParam.push_back(fileOpenName);

#if defined (Q_OS_WIN32)
	struct tm* newTime;
	time_t szClock;
	time( &szClock );
	newTime = localtime(&szClock);
	string str = asctime(newTime);
	for (int i=0; i<str.length(); ++i)
	{
		if (str[i] == ' ') str[i] = '_';
		if (str[i] == ':') str[i] = '-';
	}

	QDir outputDir;
	QString saveDir = outputDir.currentPath();
	QString outputFolder = saveDir + "/s2_simulator_results/";
	QString qstr = QString::fromStdString(str);
	qstr.remove(QRegExp("[\\n\\t\\r]"));
	outputFolder = outputFolder + qstr;
	//qDebug() << outputFolder;

	QString swcOutput = outputFolder + "/swc/";
	QString cubeOutput = outputFolder + "/cube/";
	outputDir.mkpath(swcOutput);
	outputDir.mkpath(cubeOutput);
#endif
	
	QString cubeSideLength = sizeEdit->text();
	initialParam.push_back(cubeSideLength);
	QString overlap = overlapEdit->text();
	initialParam.push_back(overlap);
	QString bkg = backgroundEdit->text();
	initialParam.push_back(bkg);

	fakeScope.data1d = VirtualVolume::instance(initialParam[0].toStdString().c_str());
	QList<ImageMarker> inputSeed = readMarker_file(initialParam[1]);
	float x = inputSeed[0].x;
	float y = inputSeed[0].y;
	float z = inputSeed[0].z;
	fakeScope.seedLocation.x = x;
	fakeScope.seedLocation.y = y;
	fakeScope.seedLocation.z = z;
	fakeScope.cubeSize = initialParam[2].toFloat();
	fakeScope.overlap = initialParam[3].toFloat();
	fakeScope.bkgThres = initialParam[4].toInt();

	LocationSimple startLoc;

	emit moveToNextWithStage(startLoc, x, y);
}

void S2UI::traceData(){
	
   // This function aims to simulate how the s2 works in an actual already accquired data of TeraFly format.
   // It needs the user to select the input TeraFly data as well as a marker file which specify the soma coordinates.
   // The current version is only a prototypical demo which may subject to further revisions.
   // S2 connector plug-in is required for this function.

   // The first part of this function reads in the input image file for tracing as well as the marker file of soma coordinates
   // Open a window for the user to select input image file
   QString m_InputfolderName = QFileDialog::getExistingDirectory(this, QObject::tr("Choose the directory of the input images"),QDir::currentPath(), QFileDialog::ShowDirsOnly);
   if (m_InputfolderName.isEmpty())
			   return;

   // load the input image file
   VirtualVolume* data1d = VirtualVolume::instance(m_InputfolderName.toStdString().c_str());
   V3DLONG in_zz[4];
   in_zz[0] = data1d->getDIM_H();
   in_zz[1] = data1d->getDIM_V();
   in_zz[2] = data1d->getDIM_D();
   in_zz[3] = data1d->getDIM_C();
  //qDebug() << in_zz[0];
  // qDebug() << in_zz[1];
  // qDebug() << in_zz[2];
  // qDebug() << in_zz[3];

   // Open a window for the user to select input the marker file of soma coordinates
   QString fileOpenName;
   fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker File"),"", QObject::tr("Supported file (*.marker *.MARKER)"));
   if (fileOpenName.isEmpty())
			   return;
   // obtain timestamp
#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
   struct timeval tp;
   gettimeofday(&tp, NULL);
   int timestamp=tp.tv_sec;

   char create_output_folder[255] = {0};
   sprintf(create_output_folder, "mkdir -p s2_simulator_results_%d", timestamp);
   system(create_output_folder);
   char cd_output_folder[255] = {0};
   sprintf(cd_output_folder, "./s2_simulator_results_%d", timestamp);
   chdir(cd_output_folder);
   system("mkdir -p swc");
   //system("mkdir -p cube");
   system("mkdir -p swc_global");
#endif
   //QDebug() << timestamp;
   //v3d_msg("check");

#if defined (Q_OS_WIN32)
	struct tm* newTime;
	time_t szClock;
	time( &szClock );
	newTime = localtime(&szClock);
	string str = asctime(newTime);
	for (int i=0; i<str.length(); ++i)
	{
		if (str[i] == ' ') str[i] = '_';
		if (str[i] == ':') str[i] = '-';
	}

	QDir currDir;
	QString saveDir = currDir.currentPath();
	QString outputFolder = saveDir + "/s2_simulator_results/";
	QString qstr = QString::fromStdString(str);
	qstr.remove(QRegExp("[\\n\\t\\r]"));
	outputFolder = outputFolder + qstr;
	//qDebug() << outputFolder;

	QString swcOutput = outputFolder + "/swc/";
	QString cubeOutput = outputFolder + "/cube/";
	currDir.mkpath(swcOutput);
	currDir.mkpath(cubeOutput);
#endif


   // Suppose a simple case, only one point is provided in this marker
   QList<ImageMarker> initial_markerList;
   initial_markerList = readMarker_file(fileOpenName);

   // implement a "queue" using a QList
   //QList<ImageMarker> myqueue;
   //LandmarkList myqueue;
   //QList<LandmarkList> TipsList_queue;

   // append the intial marker first, the new coordinates will be appended in the loop later
   //myqueue.append(initial_markerList.at(0));

   //LandmarkList firstip;
  // firstip.append(initialTarget);
  //printf("%f", initialTarget.size);
   //QDebug() << initialTarget.y;
   //QDebug() << initialTarget.z;
   //v3d_msg("test2");

   //myStackAnalyzer3->allTipsList.append(firstip);

   //v3d_msg("test");
   int count=0;
   int i=0;

   // obtain the input parameters values from the edit boxes
   float cubeSideLength=sizeEdit->text().toFloat();
   float overlap=overlapEdit->text().toFloat();
   int background=int(round(backgroundEdit->text().toFloat()));
   if (cubeSideLength<0)
   {cubeSideLength=50;
	v3d_msg("Tile size cannot be smaller than 0. Default value of 50 is used instead.");
   }
   if ((cubeSideLength>in_zz[0]) || (cubeSideLength>in_zz[1]))
   {if (in_zz[0]>=in_zz[1])
		   cubeSideLength=in_zz[0];
	   else
		   cubeSideLength=in_zz[1];
	v3d_msg("Tile size cannot be larger than the input image size. The image size is used instead.");
   }
   if (overlap<0)
   {overlap=0.05;
	v3d_msg("Overlap cannot be smaller than 0. Default value of 0.05 is used instead.");
   }
   if (overlap>1)
   {overlap=0.05;
	v3d_msg("Overlap cannot be larger than 1. Default value of 0.05 is used instead.");
   }
   if (background<0)
   {background=35;
	v3d_msg("Background value cannot be smaller than 0. Default value of 35 is used instead.");
   }
   if (background>100)
   {background=35;
	v3d_msg("Background value cannot be larger than 100. Default value of 35 is used instead.");
   }
   qDebug() << cubeSideLength;
   qDebug() << overlap;
   qDebug() << background;

   LocationSimple initialSeed;
   initialSeed.x=floor(initial_markerList.at(0).x);
   initialSeed.y=floor(initial_markerList.at(0).y);
   initialSeed.z=floor(0.5*in_zz[2]);

   LocationSimple initialTarget;
   initialTarget.x =floor(initial_markerList.at(0).x-0.5*cubeSideLength);
   initialTarget.y =floor(initial_markerList.at(0).y-0.5*cubeSideLength);
   initialTarget.z = 0;
   initialTarget.ev_pc1 = cubeSideLength;
   initialTarget.ev_pc2 = cubeSideLength;

   allTargetList.append(initialTarget);


   //v3d_msg("check");
   int tileStatus=0;

   //char new_coor_marker_filename[255] = {0};
   //char boundary_marker_filename[255] = {0};
   char swc_filename[255] = {0};
   char swc_filename_global[255] = {0};
   char cube_filename[255] = {0};
//#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
//   system("mkdir -p swc");
//   system("mkdir -p cube");
//#endif

   // keep a list of the coordinates of the cubes which have already been traced
   LandmarkList alreadytracedcube_markerList;
   alreadytracedcube_markerList.clear();
   //QList<ImageMarker> alreadytracedcube_markerList;
   float x_start,x_end,y_start,y_end;

  // QDir currDir;
  // QString saveDir = currDir.currentPath();
  // QString timelabel = QString::number(timestamp);
   //QString outputFolder = saveDir + "/s2_simulator_results"+timelabel+'/';
  // QString scanDataFileString;
   //scanDataFileString=outputFolder;
   //scanDataFileString.append("/").append("scanData.txt");
   //qDebug()<<scanDataFileString;
   QFile saveTextFile;
   saveTextFile.setFileName("scanData.txt");// add currentScanFile
   if (!saveTextFile.isOpen()){
	   if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
		   qDebug()<<"unable to save file!";
   //        total4DImage_mip->deleteRawDataAndSetPointerToNull();
	//       emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave,imageSaveString, tileStatus);
		  return;}     }
   QTextStream outputStream;
   outputStream.setDevice(&saveTextFile);

   // The second part of this function traces neighboring cubes (if tract presents in the boundary) in a breadth-first manner
   while (allTargetList.size()>0)
   { //count=count+1;
	   // For debug purpose only
	   //for(i=0;i<myqueue.size();i++){
	   //    printf("iter %d, x= %f , y= %f\n", count, myqueue.at(i).x, myqueue.at(i).y);

	   //}

	   if(alreadytracedcube_markerList.contains(allTargetList.at(0)))
	   {  //v3d_msg("This cube has already been traced");
		   qDebug() << allTargetList.size();
		   allTargetList.removeFirst();
		   myallTipsList.removeFirst();
		   count=count+1;
		   qDebug() << allTargetList.size();
		   continue;
	   }
	 //v3d_msg("test");
	 //cubeSideLength = in_zz[2];
	 x_start= allTargetList.at(0).x;
	 x_end=x_start+cubeSideLength;
	 y_start= allTargetList.at(0).y;
	 y_end=y_start+cubeSideLength;


   if (x_start<0)
	   x_start=0;
   if (y_start<0)
	   y_start=0;

   if(x_start>in_zz[0])
	  {allTargetList.removeFirst();
	   myallTipsList.removeFirst();
	   count=count+1;
	   continue;
	  }
   if(y_start>in_zz[1])
	  {allTargetList.removeFirst();
	   myallTipsList.removeFirst();
	   count=count+1;
	   continue;
	  }
   if (x_end>in_zz[0])
	   x_end=in_zz[0];
   if (y_end>in_zz[1])
	   y_end=in_zz[1];
   if (x_end<0)
   {allTargetList.removeFirst();
	   myallTipsList.removeFirst();
	   count=count+1;
	   continue;
   }
   if (y_end<0)
   {allTargetList.removeFirst();
	   myallTipsList.removeFirst();
	   count=count+1;
	   continue;
   }

   // crop the image
   unsigned char * cropped_image = 0;
   qDebug() << y_start;
   qDebug() << y_end;
   qDebug() << x_start;
   qDebug() << x_end;
   qDebug() << in_zz[2];
   cropped_image = data1d->loadSubvolume_to_UINT8(y_start, y_end,
												  x_start, x_end,
												  0, in_zz[2]);

   V3DLONG in_sz[4];
   in_sz[1] = y_end-y_start;
   in_sz[0] = x_end-x_start;
   in_sz[2] = in_zz[2];
   in_sz[3] = data1d->getDIM_C();
   
#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
   //sprintf(cube_filename, "./swc/x_%d_y_%d_cube.v3draw", int(x_start),int(y_start));
   sprintf(cube_filename, "./swc/x_%d_y_%d_Ch2.v3draw", int(x_start),int(y_start));
   //printf("cube %d",count);
   QString saveName = cube_filename;
   const char* fileName = saveName.toAscii();
   simple_saveimage_wrapper(*cb, fileName, cropped_image, in_sz, 1);
   //v3d_msg("Cropping complete.");

   if( access( cube_filename, R_OK ) == -1 )
   {allTargetList.removeFirst();
	myallTipsList.removeFirst();
	count=count+1;
	continue;
   }
#endif

#if defined (Q_OS_WIN32)
	QString cubeLabel = QString::number(count);
	cubeOutput = cubeOutput + cubeLabel + "_cube.v3draw";
	const char* temp = cubeOutput.toStdString().c_str();
	strcpy(cube_filename, temp);
   printf("cube %d",count);
   QString saveName = cube_filename;
   const char* fileName = saveName.toAscii();
   simple_saveimage_wrapper(*cb, fileName, cropped_image, in_sz, 1);
   //v3d_msg("Cropping complete.");

	if( access( cube_filename, 4 ) == -1 )
   {allTargetList.removeFirst();
	myallTipsList.removeFirst();
	   count=count+1;
	continue;
   }
#endif

   // prepare the inputs for APP2Tracing
   Image4DSimple * pNewImage = cb->loadImage(cube_filename);
   pNewImage->setOriginX(x_start);
   pNewImage->setOriginY(y_start);
   pNewImage->setOriginZ(0);

   Image4DSimple * total4DImage_mip;
   LandmarkList seedList;
   LocationSimple tileLocation;
   tileLocation = allTargetList.at(0);
   if (count>0)
   { seedList = myallTipsList.at(0);
   }
   else
   {seedList.clear();}

   loadScanNumber = count;
   bool isSoma = loadScanNumber==0;
   //bool isAdaptive = false;
   bool useGSDT = true;
   bool interrupt = true;

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
   QString currDir = QDir().currentPath();
   QString SWClabelcount = QString::number(count);
   QString SWClabelX = QString::number(int(x_start));
   QString SWClabelY = QString::number(int(y_start));
   QString currDir2 = currDir + "/swc";
   //currDir = currDir + "/swc/" + SWClabelcount+"_x_"+SWClabelX+"_y_"+SWClabelY+"_Ch2.swc";

   currDir = currDir + "/swc/x_"+SWClabelX+"_y_"+SWClabelY+"_Ch2.swc";
   const char* temp2 = currDir.toStdString().c_str();
   strcpy(swc_filename, temp2);
   QString swcString = swc_filename;
   printf("swc %d",count);
   sprintf(swc_filename_global, "./swc_global/%d_x_%d_y_%d.swc", count,int(x_start),int(y_start));
#endif

#if defined(Q_OS_WIN32)
   QString SWClabel = QString::number(count);
   QString swcFilename = swc_filename;
   swcFilename = swcOutput + SWClabel + ".swc";
   const char* temp1 = swcFilename.toStdString().c_str();
   strcpy(swc_filename, temp1);
   QString swcString= swcFilename;
   cout << "swc " << count << endl;
#endif

   QString tileSaveString="file2.swc";

   // use APP2 for tracing
   //bool s2Mode = true;

   if (tracingMethodComboC->currentIndex()==0){
	//APP2
		   myStackAnalyzer3->APP2Tracing(pNewImage, total4DImage_mip, swcString, overlap, background, interrupt, seedList, useGSDT, isSoma, tileLocation,tileSaveString,tileStatus);



	   }
	   if (tracingMethodComboC->currentIndex()==1){
	 //MOST
		   //v3d_msg(currDir2);
		   if (count==0)
		   {  seedList.append(initialSeed);


		   }

		   //myStackAnalyzer3->SubtractiveTracing('\0',cube_filename, pNewImage, total4DImage_mip, swcString, overlap, background,interrupt, seedList, tileLocation, currDir2,useGSDT, isSoma, 0, tileStatus);
	   }
	   if (tracingMethodComboC->currentIndex()==2){
	  // NeuTube
		   //myStackAnalyzer3->SubtractiveTracing('\0',cube_filename, pNewImage, total4DImage_mip, swcString, overlap, background,interrupt, seedList, tileLocation, currDir2,useGSDT, isSoma, 1, tileStatus);

	   }




	 // v3d_msg("test");
  // v3d_msg("test1");


   int x_print=int(x_start);
   int y_print=int(y_start);
   int tilesize=int(cubeSideLength);

outputStream<< (int) x_print<<" "<< (int) y_print<<" "<<swcString<<" "<< (int) tilesize<<" "<< (int) tilesize<<" "<< 0<<" "<< 0<<"\n";



   // remove the cubes that havee already been processed
   alreadytracedcube_markerList.push_back(allTargetList.at(0));
	allTargetList.removeAt(0);
   if (count>0)
   {
	myallTipsList.removeFirst();}

   count=count+1;

/*
   // add outputs to the queue
   for (i=0;i<newTargetList.size();i++)
   {myqueue.push_back(newTargetList[i]);
	TipsList_queue.push_back(newTipsList.at(i));
   }
*/

   //newTipsList=readLandmarkList("newTipsList.landmarklist");
   //newTargetList=readLandmarkList("newTargetList.landmarklist");


   // convert swc coordinates to world coordinates
   NeuronTree myswc;
   myswc=readSWC_file(swc_filename);
   QList<NeuronSWC> newSWC;
   for(i=0;i<myswc.listNeuron.size();i++)
   {
	   NeuronSWC singleNeuron;
	   singleNeuron.type = 2;  //myswc.listNeuron.at(i).type;
	   singleNeuron.n = myswc.listNeuron.at(i).n;
	   singleNeuron.x = myswc.listNeuron.at(i).x+x_start;
	   singleNeuron.y = myswc.listNeuron.at(i).y+y_start;
	   singleNeuron.z = myswc.listNeuron.at(i).z;
	   singleNeuron.parent = myswc.listNeuron.at(i).parent;
	   newSWC.append(singleNeuron);
  }
   
   NeuronTree saveSWC;
   saveSWC.listNeuron = newSWC;
   writeSWC_file(swc_filename_global, saveSWC);


 /*
   // read in the marker of boundary vertices
   QList<ImageMarker> boundary_markerList;

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
   sprintf(boundary_marker_filename, "./swc/%d.swcfinal.marker", count);
   if( access( boundary_marker_filename, R_OK ) == -1 )
   {myqueue.removeFirst();
	   count=count+1;
	continue;
   }
#endif

#if defined (Q_OS_WIN32)
   QString Markerlabel = QString::number(count);
   QString boundary_markerName = swcOutput + Markerlabel + ".swcfinal.marker";
   const char* tempBoundaryMarker = boundary_markerName.toStdString().c_str();
   strcpy (boundary_marker_filename, tempBoundaryMarker);
   if( access( boundary_marker_filename, 4 ) == -1 )
   {myqueue.removeFirst();
	   count=count+1;
	continue;
   }
#endif

   
   boundary_markerList = readMarker_file(boundary_marker_filename);
   QList<ImageMarker> neighbor_cube_markerList;
   ImageMarker new_center_coor;
   new_center_coor.x=0;
   new_center_coor.y=0;
   
   // determine which neighboring cubes should be traced in the next iteration (8 possible candidates each time)
   for (i=0;i<boundary_markerList.size();i++)
   {if((boundary_markerList.at(i).x< cubeSideLength*thrs) &&
	   (boundary_markerList.at(i).y< cubeSideLength*thrs)) {
		   new_center_coor.x=xcenter-cubeSideLength;
		   new_center_coor.y=ycenter-cubeSideLength;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
	 }
	 else if((boundary_markerList.at(i).x>cubeSideLength*thrs) &&
			 (boundary_markerList.at(i).x< cubeSideLength*(1-thrs)) &&
			 (boundary_markerList.at(i).y< cubeSideLength*thrs)) {
		   new_center_coor.x=xcenter;
		   new_center_coor.y=ycenter-cubeSideLength;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
	   }
		else if((boundary_markerList.at(i).x> cubeSideLength*(1-thrs)) &&
				(boundary_markerList.at(i).y< cubeSideLength*thrs)) {
		   new_center_coor.x=xcenter+cubeSideLength;
		   new_center_coor.y=ycenter-cubeSideLength;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
		}
	 else if((boundary_markerList.at(i).x>cubeSideLength*(1-thrs)) &&
			 (boundary_markerList.at(i).y< cubeSideLength*(1-thrs)) &&
			 (boundary_markerList.at(i).y> cubeSideLength*thrs)) {
		   new_center_coor.x=xcenter+cubeSideLength;
		   new_center_coor.y=ycenter;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
	 }
	 else if((boundary_markerList.at(i).x> cubeSideLength*(1-thrs)) &&
			 (boundary_markerList.at(i).y> cubeSideLength*(1-thrs))) {
		   new_center_coor.x=xcenter+cubeSideLength;
		   new_center_coor.y=ycenter+cubeSideLength;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
	 }
	 else if((boundary_markerList.at(i).x>cubeSideLength*thrs) &&
			 (boundary_markerList.at(i).x< cubeSideLength*(1-thrs)) &&
			 (boundary_markerList.at(i).y> cubeSideLength*(1-thrs))) {
			 new_center_coor.x=xcenter;
			 new_center_coor.y=ycenter+cubeSideLength;
			 if(!neighbor_cube_markerList.contains(new_center_coor))
			 {
				 neighbor_cube_markerList.append(new_center_coor);
			 }
	 }
	 else if((boundary_markerList.at(i).x<cubeSideLength*thrs) &&
			 (boundary_markerList.at(i).y> cubeSideLength*(1-thrs))) {
		   new_center_coor.x=xcenter-cubeSideLength;
		   new_center_coor.y=ycenter+cubeSideLength;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
	 }
		else {
		   new_center_coor.x=xcenter-cubeSideLength;
		   new_center_coor.y=ycenter;
		   if(!neighbor_cube_markerList.contains(new_center_coor))
		   {
			   neighbor_cube_markerList.append(new_center_coor);
		   }
		}
   }

   if(neighbor_cube_markerList.size()<1)
   {myqueue.removeFirst();
	   count=count+1;
	continue;
   }

   printf("marker %d",count);

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
   sprintf(new_coor_marker_filename, "./cube/%d.marker", count);
#endif

#if defined (Q_OS_WIN32)
   QString newCoorMarkerlabel = QString::number(count);
   QString newCoor_marker_filename = cubeOutput + newCoorMarkerlabel + ".marker";
   const char* new_coor_marker_fileName = newCoor_marker_filename.toStdString().c_str();
   strcpy(new_coor_marker_filename, new_coor_marker_fileName);
#endif

   writeMarker_file(new_coor_marker_filename, neighbor_cube_markerList);

   // add the targeted cubes into the queue for next iteration of cropping and tracing
   for(i=0;i<neighbor_cube_markerList.size();i++){
	   if((!alreadytracedcube_markerList.contains(neighbor_cube_markerList.at(i))) && (!myqueue.contains(neighbor_cube_markerList.at(i))))
	   {myqueue.append(neighbor_cube_markerList.at(i));
		}
   }
*/

   // remove the current finished cube from the queue
   //myqueue.removeFirst();
  //TipsList_queue.removeFirst();

}
saveTextFile.close();

	// combine swc files of the cubes into a single swc file (global coordinates)
#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
   system("vaa3d -x S2_tracing_connector -f combineSWC -i ./swc_global -o ./swc_global/combined.swc -p linux");
   system("cp ./swc_global/combined_connected.swc .");

   system("cd ..");
   v3d_msg("Tracing complete! Please check out the output file 'combined_connected.swc' in your results folder");
#endif


   /*
#if defined (Q_OS_WIN32)
   char* command;
   QString preCommand = "vaa3d_msvc.exe /x S2_tracing_connector /f combineSWC /i " + swcOutput + " /o " + swcOutput + "combined.swc /p windows";
   const char* CpreCommand = preCommand.toStdString().c_str();
   strcpy(command, CpreCommand);
   //cout << command << endl;
   system(command);
   v3d_msg("Tracing complete! Please check out the output file 'combined_connected.swc' in your results folder");
#endif
*/

   return;
}

void S2UI::updateLocalRemote(bool state){
	isLocal = state;
	status(QString("isLocal ").append(QString::number(isLocal)));
	QString timeString = QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm");
	QString topDirStr = QString("F:/testData/");
	QSettings settings("HHMI", "Vaa3D");
	QString localDataString  = QString("");


	if (isLocal){
		myController.hostLineEdit->setText(QString("local"));
		myPosMon.hostLineEdit->setText(QString("local"));
		localDataDirectory = QDir("testData");
	}else{
		myController.hostLineEdit->setText(QString("10.128.48.53"));
		myPosMon.hostLineEdit->setText(QString("10.128.48.53"));
		localDataDirectory = QDir("testData");

		if (!settings.contains("s2_topDir")|| !settings.contains("s2_dataDir")){
			v3d_msg("S2 directories have not been configured. You will next be prompted for a local directory to save s2 scans and for a path to the microscope image data");
		}

		if (!settings.contains("s2_topDir")){
			topDirStr = QFileDialog::getExistingDirectory(this, tr("Choose save directory..."),
														  "/",
														  QFileDialog::ShowDirsOnly
														  | QFileDialog::DontResolveSymlinks);
			settings.setValue("s2_topDir",topDirStr);
		}else{
			topDirStr =  settings.value("s2_topDir").value<QString>();
		}
		if (!settings.contains("s2_dataDir")){
			localDataString = QFileDialog::getExistingDirectory(this, tr("Choose local data directory..."),
																"/",
																QFileDialog::ShowDirsOnly
																| QFileDialog::DontResolveSymlinks);
			settings.setValue("s2_dataDir",localDataString);
		}else{
			localDataString =  settings.value("s2_dataDir").value<QString>();
		}

		localDataDirectory = QDir(localDataString);


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

	localDataDir->setText(localDataString);

	saveDir =QDir(topDir.absolutePath().append("/").append(timeString));

	sessionDir = saveDir;
	scanDataFileString = saveDir.absolutePath().append("/").append("_0_scanData.txt");
	eventLogString = QFileInfo(scanDataFileString).absoluteDir().absolutePath().append(QDir::separator()).append( QFileInfo(scanDataFileString).baseName()).append("eventData.txt");

	myNotes->setSaveDir(saveDir); // this gets saved in the parent directory.  scanDataFileString will get modified for each scan
	resetDir=false;
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

	for (int jj=0; jj<=23; jj++){
		QLabel * labeli = new QLabel(tr("test"));
		labeli->setText(QString::number(jj));
		labeli->setObjectName(QString::number(jj));
		labeli->setWordWrap(true);
		labeli->setFont(newFont);
		gbMon->addWidget(labeli, jj%12, jj/12);
	}
	gMonBox->setLayout(gbMon);
	return gMonBox;
}



QGroupBox *S2UI::createTracingParameters(){
	QGroupBox *tPBox = new QGroupBox(tr("Tracing"));

	QGridLayout *tPL = new QGridLayout;
	runSAStuff = new QPushButton(tr("process smartScan files"));
	QLabel * labeli = new QLabel(tr("background threshold = "));
	labeli->setAlignment(Qt::AlignRight);
	QSpinBox *bkgSpnBx = new QSpinBox(0);
	s2Label->setText("input file path:");
	bkgSpnBx->setMaximum(255);
	bkgSpnBx->setMinimum(-1);
	bkgSpnBx->setValue(30);
	bkgSpnBx->setObjectName("bkgSpinBox");



	gridScanCB = new QCheckBox;
	gridScanCB->setChecked(false);
	gridScanCB->setText("Grid Scan");


	QLabel * gridSizeSBLabel = new QLabel(tr("Grid Size"));
	gridSizeSBLabel->setAlignment(Qt::AlignRight);
	gridSizeSB = new QSpinBox;
	gridSizeSB->setMinimum(3);
	gridSizeSB->setMaximum(7);
	gridSizeSB->setSingleStep(2);
	gridSizeSBLabel->setBuddy(gridSizeSB);



	overlapSpinBox = new QSpinBox;
	overlapSpinBox->setSuffix(" percent ");
	overlapSpinBox->setMinimum(0);
	overlapSpinBox->setMaximum(25 );
	overlapSpinBox->setValue(10);
	overlapSBLabel = new QLabel;
	overlapSBLabel->setText(tr("tile &overlap: "));
	overlapSBLabel->setBuddy(overlapSpinBox);
	overlapSBLabel->setAlignment(Qt::AlignRight);
	overlap = 0.01* ((float) overlapSpinBox->value());


	addBoundingBoxScan = new QCheckBox;
	addBoundingBoxScan->setChecked(true);
	addBoundingBoxScan->setText("scan &bounding box");

	tracingMethodComboB = new QComboBox;
	tracingMethodComboB->addItem("MOST");
	tracingMethodComboB->addItem("APP2");
	tracingMethodComboB->addItem("NeuTube");
	tracingMethodComboB->addItem("adaptive MOST");
	tracingMethodComboB->addItem("adaptive APP2");
	tracingMethodComboB->addItem("adaptive NeuTube");
	tracingMethodComboB->addItem("automatic");
	tracingMethodComboB->addItem("debugging mode");
	tracingMethodComboB->setCurrentIndex(2);
	methodChoice = 2;
	QLabel * tracingMethodComboBLabel = new QLabel(tr("Tracing Method: "));
	tracingMethodComboBLabel->setAlignment(Qt::AlignRight);



	channelChoiceComboB = new QComboBox;
	channelChoiceComboB->addItem("Ch1");
	channelChoiceComboB->addItem("Ch2");
	channelChoiceComboB->addItem("G-R");

	channelChoiceComboB->setCurrentIndex(1);
	QLabel * channelChoiceComboBLabel = new QLabel(tr("Color Channel: "));




	tileSizeCB = new QComboBox;
	initializeROISizes();
	for (int i = 0; i<tileSizeChoices->length(); i++){
		TileInfo ti0 = tileSizeChoices->at(i);
		tileSizeCB->addItem(QString::number(ti0.getTileZoom()));
	}
	tileSizeCB->setCurrentIndex(6);
	QLabel * tileSizeCBLabel = new QLabel(tr("smartScan Zoom: "));


	analysisRunning = new QLabel(tr("0"));
	analysisRunning->setAlignment(Qt::AlignRight);
	QLabel * analysisRunningLable = new QLabel(tr("tiles being analyzed"));



	minBlockSizeSB = new QSpinBox;
	minBlockSizeSB->setMaximum(512);
	minBlockSizeSB->setMinimum(50);
	minBlockSizeSB->setValue(100);
	minBlockSizeSBLabel = new QLabel(tr("minimum block size"));

	maxBlockSizeSB = new QSpinBox;
	maxBlockSizeSB->setMaximum(512);
	maxBlockSizeSB->setMinimum(50);
	maxBlockSizeSB->setValue(180);
	maxBlockSizeSBLabel = new QLabel(tr("maximum block size"));




	redThresholdSlider = new QSlider;
	redThresholdSlider->setOrientation(Qt::Horizontal);

	redThresholdSlider->setMaximum(100);
	redThresholdSlider->setMinimum(0);
	redThresholdSlider->setValue(100);
	redThresholdSliderLabel = new QLabel(tr("red threshold"));



	lipoFactorSlider = new QSlider;
	lipoFactorSlider->setOrientation(Qt::Horizontal);
	lipoFactorSlider->setMaximum(100);
	lipoFactorSlider->setMinimum(0);
	lipoFactorSlider->setValue(50);
	lipoFactorSliderLabel = new QLabel(tr("alpha for G-alphaR"));


	chooseLipoMethod = new QComboBox;
	chooseLipoMethod->addItem("subtraction: image=G-alpha*R");
	chooseLipoMethod->addItem("obscuration: image=G if R<thresh");
	chooseLipoMethod->setCurrentIndex(0);
	chooseLipoMethodLabel = new QLabel(tr("Lipofuscin method"));



	stackZStepSizeSlider = new QSlider;
	stackZStepSizeSlider->setOrientation(Qt::Horizontal);
	stackZStepSizeSlider->setMinimum(1);
	stackZStepSizeSlider->setMaximum(50);
	stackZStepSizeSlider->setValue(10);
	stackZStepSizeLabel = new QLabel(tr("z stack step size = 1.0 um"));

	stageOnlyCB = new QCheckBox;
	stageOnlyCB->setChecked(true);
	stageOnlyCB->setText("stage-only S2scan");

	multiThreadTracingCB = new QCheckBox;
	multiThreadTracingCB->setChecked(true);
	multiThreadTracingCB->setText("multiThread tracing");

	searchPixelRadiusSB = new QSpinBox;
	searchPixelRadiusSB->setMaximum(1000);
	searchPixelRadiusSB->setMinimum(0);
	searchPixelRadiusSB->setValue(10);
	searchPixelRadiusSB->setSuffix(" pixels");
	searchPixelRadiusSBLabel = new QLabel(tr("segment search radius "));

	tileNotes = new QLabel(tr("tracing info: "));

	tPL->addWidget(labeli,0,0);
	tPL->addWidget(bkgSpnBx,0,1);

	tPL->addWidget(s2Label,1,0);
	tPL->addWidget(s2LineEdit,1,1,1,3);
	tPL->addWidget(startStackAnalyzerPB,2,0,1,2);
	tPL->addWidget(runSAStuff,2,2,1,2);
	tPL->addWidget(overlapSpinBox,3,3);
	tPL->addWidget(overlapSBLabel,3,2);

	tPL->addWidget(addBoundingBoxScan,3,1);


	tPL->addWidget(gridScanCB,4,1);
	tPL->addWidget(gridSizeSBLabel,4,2);
	tPL->addWidget(gridSizeSB,4,3);


	tPL->addWidget(tracingMethodComboBLabel,12,0);
	tPL->addWidget(tracingMethodComboB, 12, 1);
	tPL->addWidget(channelChoiceComboBLabel,12,2);
	tPL->addWidget(channelChoiceComboB,12,3);
	tPL->addWidget(tileSizeCBLabel,14,0);
	tPL->addWidget(tileSizeCB,14,1);
	tPL->addWidget(minBlockSizeSB,15,1);
	tPL->addWidget(minBlockSizeSBLabel,15,0);
	tPL->addWidget(maxBlockSizeSB,16,1);
	tPL->addWidget(maxBlockSizeSBLabel,16,0) ;


	tPL->addWidget(chooseLipoMethod,17,0);
	tPL->addWidget(chooseLipoMethodLabel,17,1);
	tPL->addWidget(lipoFactorSlider,18,0);
	tPL->addWidget(lipoFactorSliderLabel,18,1);
	tPL->addWidget(redThresholdSlider,19,0);
	tPL->addWidget(redThresholdSliderLabel,19,1);
	tPL->addWidget(stackZStepSizeSlider,20,0);
	tPL->addWidget(stackZStepSizeLabel,20,1);

	tPL->addWidget(stageOnlyCB,14,2);
	tPL->addWidget(multiThreadTracingCB, 15, 2);

	tPL->addWidget(searchPixelRadiusSB, 16, 2);
	tPL->addWidget(searchPixelRadiusSBLabel,16,3);
	tPL->addWidget(tileNotes,22,0,1,2);
	tPL->addWidget(analysisRunningLable,21,1);
	tPL->addWidget(analysisRunning,21,0);
	for (int ii; ii<tPL->rowCount(); ii++) tPL->setRowMinimumHeight(ii,1);
	tPBox->setLayout(tPL);
	return tPBox;
}

QGroupBox *S2UI::createConfigPanel(){
	QGroupBox *configBox = new QGroupBox(tr("Config"));

	QGridLayout *cBL = new QGridLayout;

	localRemoteCB = new QCheckBox;
	localRemoteCB->setText(tr("Local PrairieView"));

	machineSaveDir = new QLabel(tr("microscope not initialized"));
	machineSaveDirLabel = new QLabel(tr("Microscope Save Directory : "));
	resetDirPB = new QPushButton;
	resetDirPB->setText(tr("set Local Save Directory"));

	localDataDir = new QLabel(tr(""));
	localDataDirLabel = new QLabel(tr("local path to data : "));

	setLocalPathToData = new QPushButton;
	setLocalPathToData->setText(tr("set local path to data"));


	QLabel * labelInterrupt = new QLabel(tr("&notify after each trace"));
	QCheckBox *interruptCB = new QCheckBox;
	useGSDTCB = new QCheckBox;
	QLabel * labelGSDT = new QLabel(tr("use &GSDT in APP2"));
	labelGSDT->setBuddy(useGSDTCB);
	useGSDTCB->setChecked(true);
	labelInterrupt->setBuddy(interruptCB);
	interruptCB->setObjectName("interruptCB");
	interruptCB->setChecked(false);
	runContinuousCB = new QCheckBox;
	runContinuousCB->setChecked(true);
	QLabel* runContinuousCBLabel = new QLabel(tr("&continuous imaging"));
	runContinuousCBLabel->setBuddy(runContinuousCB);


	startZStackDelaySB = new QSpinBox;
	startZStackDelaySB->setMinimum(0);
	startZStackDelaySB->setMaximum(2000);
	startZStackDelaySB->setValue(1000);
	startZStackDelaySB->setSuffix("ms");
	startZStackDelayLabel = new QLabel(tr("start stack delay"));


	zoomSpinBox = new QSpinBox;
	zoomSpinBox->setMaximum(64);
	zoomSpinBox->setMinimum(1);
	zoomSpinBox->setValue(1);

	zoomSpinBoxLabel = new QLabel;
	zoomSpinBoxLabel->setText("zoom");


	pixelsSpinBox = new QSpinBox;
	pixelsSpinBox->setMinimum(50);
	pixelsSpinBox->setMaximum(2048);
	pixelsSpinBox->setValue(2048);

	pixelsSpinBoxLabel = new QLabel;
	pixelsSpinBoxLabel->setText("pixels");
	zoomPixelsProductLabel = new QLabel(tr("zoom*pixels = "));



	tryStageMove  = new QPushButton;
	tryStageMove->setText("try stage move");
	tryStageXEdit = new QLineEdit;
	tryStageXEdit->setText("0.0");
	tryStageYEdit = new QLineEdit;
	tryStageYEdit->setText("0.0");




	sendThemAllCB = new QCheckBox;
	sendThemAllCB->setChecked(false);
	sendThemAllCBLabel = new QLabel(tr("send them all!"));



	cBL->addWidget(machineSaveDirLabel,0,0);
	cBL->addWidget(machineSaveDir, 1,0);
	cBL->addWidget(localDataDirLabel,2,0);
	cBL->addWidget(localDataDir,2,1);
	cBL->addWidget(setLocalPathToData,3,1);
	cBL->addWidget(resetDirPB, 3,0);

	cBL->addWidget(startZStackDelayLabel,6,0);
	cBL->addWidget(startZStackDelaySB, 6,1);
	cBL->addWidget(zoomSpinBoxLabel,7,0);
	cBL->addWidget(zoomSpinBox,7,1);
	cBL->addWidget(pixelsSpinBoxLabel,8,0);
	cBL->addWidget(pixelsSpinBox,8,1);
	cBL->addWidget(zoomPixelsProductLabel,9,0);
	cBL->addWidget(tryStageMove,11,0);
	cBL->addWidget(tryStageXEdit,11,1);
	cBL->addWidget(tryStageYEdit,11,2);
	cBL->addWidget(sendThemAllCB, 12, 0);
	cBL->addWidget(sendThemAllCBLabel,12,1);
	cBL->addWidget(labelInterrupt,13,0);
	cBL->addWidget(interruptCB, 13,1);
	cBL->addWidget(labelGSDT,14,0);
	cBL->addWidget(useGSDTCB,14,1);
	cBL->addWidget(runContinuousCBLabel,15,0);
	cBL->addWidget(runContinuousCB,15,1);
	cBL->addWidget(localRemoteCB,16,0);
	configBox->setLayout(cBL);
	return configBox;
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

QGroupBox *S2UI::createSimulator(){
   QGroupBox *gSimulator = new QGroupBox(tr("&Simulator"));
   // gSimulator->setCheckable(true);
   // gSimulator->setChecked(true);

	tracingMethodComboC = new QComboBox;
	tracingMethodComboC->addItem("APP2");
	tracingMethodComboC->addItem("MOST");
	tracingMethodComboC->addItem("NeuTube");
	tracingMethodComboC->setCurrentIndex(0);
	//methodChoice = 2;
	QLabel * tracingMethodComboCLabel = new QLabel(tr("Tracing Method: "));
	tracingMethodComboCLabel->setAlignment(Qt::AlignRight);

	QLabel *sizeLabel = new QLabel(tr("Tile size ="));
	sizeEdit = new QLineEdit("50");
	sizeLabel->setBuddy(sizeEdit);
	sizeEdit->setObjectName("sizeX");

	QLabel *overlapLabel = new QLabel(tr("Overlap ="));
	overlapEdit = new QLineEdit("0.05");
	overlapLabel->setBuddy(overlapEdit);
	overlapEdit->setObjectName("overlapX");

	QLabel *backgroundLabel = new QLabel(tr("Background ="));
	backgroundEdit = new QLineEdit("35");
	backgroundLabel->setBuddy(backgroundEdit);
	backgroundEdit->setObjectName("backgroundX");

	tracePB = new QPushButton(tr("select data to trace"));

	QGridLayout *glROI = new QGridLayout;


	glROI->addWidget(tracingMethodComboCLabel,4,0);
	glROI->addWidget(tracingMethodComboC, 4, 1);

	glROI->addWidget(tracePB, 5, 0, 1,2);
	glROI->addWidget(sizeLabel,1, 0);
	glROI->addWidget(sizeEdit, 1, 1);
	glROI->addWidget(overlapLabel, 2, 0);
	glROI->addWidget(overlapEdit, 2, 1);
	glROI->addWidget(backgroundLabel, 3, 0);
	glROI->addWidget(backgroundEdit, 3, 1);
	gSimulator->setLayout(glROI);
	return gSimulator;
}

void S2UI::startS2()
{
	localRemoteCB->setEnabled(false);
	myController.initializeS2();
	myPosMon.initializeS2();
	QTimer::singleShot(2000,this, SLOT(posMonButtonClicked()));
	startS2PushButton->setText("...initializing...");// should check something..?
}

void S2UI::startScan()
{
	lastFile=getFileString();
	//status(QString("lastFile = ").append(lastFile));
	waitingForFile = 1;
	QTimer::singleShot(0, &myController, SLOT(startScan()));
	float leftEdge = roiXEdit->text().toFloat() -roiXWEdit->text().toFloat()/2.0;
	float topEdge = roiYEdit->text().toFloat() - roiYWEdit->text().toFloat()/2.0;
	roiGS->addRect(leftEdge,topEdge,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));

}


void S2UI::loadScan(){
	loadLatest(getFileString());
}


void S2UI::toLoad(){
	loadScanFromFile(s2LineEdit->text());
}

void S2UI::loadForSA(){
	LandmarkList seedList;
	LocationSimple tileLocation;

	if (smartScanStatus == 1){
		seedList = tipList.at(loadScanNumber);
		tileLocation = scanList.value(loadScanNumber).getPixelLocation();
	}else{
		tileLocation.x = 0;
		tileLocation.y = 0;
		seedList.clear();
	}
	bool isSoma = loadScanNumber==0;
	qDebug()<<workerThread->currentThreadId();
	QTimer::singleShot(0,this, SLOT(processingStarted()));
	bool isAdaptive = false;
	methodChoice = 2;
	int tileStatus = 0;
	if (tracingMethodComboB->currentIndex()==0){
		methodChoice = 0;
		isAdaptive = false;
		//emit callSALoadSubtractive(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma, methodChoice);

	}
	if (tracingMethodComboB->currentIndex()==1){
		//        emit callSALoad(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma);
		methodChoice = 2;
		isAdaptive = false;
	}
	if (tracingMethodComboB->currentIndex()==2){
		methodChoice = 1;
		//        emit callSALoadSubtractive(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma, methodChoice);

	}
	if (tracingMethodComboB->currentIndex()==3){
		methodChoice = 0;
		isAdaptive  = true;
		//        emit callSALoadAdaSubtractive(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma, methodChoice);

	}
	if (tracingMethodComboB->currentIndex()==4){
		methodChoice = 2;
		isAdaptive = true;
		//emit callSALoadAda(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma);

	}
	if (tracingMethodComboB->currentIndex()==5){
		methodChoice= 1;
		isAdaptive = true;
		//   emit callSALoadAdaSubtractive(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma, methodChoice);

	}
	if (tracingMethodComboB->currentIndex()==6){
		methodChoice= -1;
		isAdaptive = true;

		//   emit callSALoadAdaSubtractive(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma, methodChoice);

	}

	if (tracingMethodComboB->currentIndex()==7){
		methodChoice= 3;
		isAdaptive = false;

		//   emit callSALoadAdaSubtractive(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(), this->findChild<QCheckBox*>("interruptCB")->isChecked(), seedList, tileLocation, saveDir.absolutePath(),useGSDTCB->isChecked()  , isSoma, methodChoice);

	}
	qDebug()<<"methodChoice "<<methodChoice;
	qDebug()<<"comboboxCurrentIndex  "<<tracingMethodComboB->currentIndex();


	if (multiThreadTracingCB->isChecked()){    if (traceThreadNumber==0){
			emit callSATrace(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							 this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
		}else if (traceThreadNumber==1){
			emit callSATrace0(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
		}else if (traceThreadNumber==2){
			emit callSATrace1(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);

		}else if (traceThreadNumber==3){
			emit callSATrace2(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);;

		}
		status(QString("traceThreadNumber =").append(QString::number(traceThreadNumber)));
		traceThreadNumber++;
		traceThreadNumber = traceThreadNumber%4;
		qDebug()<<"traceThreadNUmber="<<traceThreadNumber;
		status(QString("traceThreadNumber =").append(QString::number(traceThreadNumber)));

	}else{
		emit callSATrace(s2LineEdit->text(),overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
						 this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);

	}
}




void S2UI::loadScanFromFile(QString fileString){
	QString latestString = fileString;
	QFileInfo imageFileInfo = QFileInfo(latestString);
	QString windowString = latestString;
	NeuronTree nt;
	LandmarkList newTargetList;


	float redAlpha =    ((float) lipoFactorSlider->value() )*2.0/100.0;
	int redThreshold = ((int) redThresholdSlider->value()*80);


	if (imageFileInfo.isReadable()){
		QStringList fileList;
		Image4DSimple * pNewImage = cb->loadImage(latestString.toLatin1().data());
		QDir imageDir =  imageFileInfo.dir();
		QStringList filterList;
		filterList.append(QString("*").append("Ch1").append("*.tif"));
		imageDir.setNameFilters(filterList);
		QStringList fileList1 = imageDir.entryList();

		QStringList filterList2;
		filterList2.append(QString("*").append("Ch2").append("*.tif"));
		imageDir.setNameFilters(filterList2);
		QStringList fileList2 = imageDir.entryList();

		if (channelChoiceComboB->currentText()=="Ch1"){
			fileList = fileList1;
		}else if (channelChoiceComboB->currentText()=="Ch2"){
			fileList = fileList2;
		}
		//get the parent dir and the list of ch1....ome.tif files
		//use this to id the number of images in the stack (in one channel?!)
		V3DLONG x = pNewImage->getXDim();
		V3DLONG y = pNewImage->getYDim();
		V3DLONG nFrames = fileList1.length();

		V3DLONG tunits = x*y*nFrames;
		unsigned short int * total1dData = new unsigned short int [tunits];
		unsigned short int * total1dData_mip= new unsigned short int [x*y];
		for(V3DLONG i =0 ; i < x*y; i++)
			total1dData_mip[i] = 0;
		V3DLONG totalImageIndex = 0;
		double p_vmax=0;
		qDebug()<<channelChoiceComboB->currentText();
		qDebug()<<"nFrames = "<<nFrames;
		for (int f=0; f<nFrames; f++){
			if (channelChoiceComboB->currentText()=="G-R") {
				Image4DSimple * pNewImage1 = cb->loadImage(imageDir.absoluteFilePath(fileList1[f]).toLatin1().data());
				Image4DSimple * pNewImage2 = cb->loadImage(imageDir.absoluteFilePath(fileList2[f]).toLatin1().data());

				if (pNewImage1->valid()){
					unsigned short int * data1d1 = 0;
					data1d1 = new unsigned short int [x*y];
					data1d1 = (unsigned short int*)pNewImage1->getRawData();
					unsigned short int * data1d2 = 0;
					data1d2 = new unsigned short int [x*y];
					data1d2 = (unsigned short int*)pNewImage2->getRawData();
					if (chooseLipoMethod->currentIndex() ==0){  // Green - alpha*Red
						for (V3DLONG i = 0; i< (x*y); i++)
						{
							if (data1d1[i] >= data1d2[i]){
								total1dData[totalImageIndex]= 0;

							}else{
								float tmp = (float)(data1d2[i])-(float)(data1d1[i])*redAlpha;

								if (tmp<0) total1dData[totalImageIndex]=0;
								else total1dData[totalImageIndex]= (unsigned short int) tmp;

							}
							if(data1d2[i] > p_vmax) p_vmax = data1d2[i];
							if(total1dData_mip[i] < data1d2[i]) total1dData_mip[i] = data1d2[i];
							totalImageIndex++;
						}

					} else if (chooseLipoMethod->currentIndex()==1){ //obscuration

						for (V3DLONG i = 0; i< (x*y); i++)
						{
							if ((data1d1[i] >= data1d2[i])|(data1d1[i]> redThreshold)){
								total1dData[totalImageIndex]= 0;

							}else{
								total1dData[totalImageIndex]= data1d2[i];

							}
							if(data1d2[i] > p_vmax) p_vmax = data1d2[i];
							if(total1dData_mip[i] < data1d2[i]) total1dData_mip[i] = data1d2[i];
							totalImageIndex++;
						}



					}
					if(data1d1) {delete []data1d1; data1d1 = 0;}
					if(data1d2) {delete []data1d2; data1d2 = 0;}

				}else{
					qDebug()<<imageDir.absoluteFilePath(fileList[f])<<" failed!";
				}

			}else{

				Image4DSimple * pNewImage = cb->loadImage(imageDir.absoluteFilePath(fileList[f]).toLatin1().data());
				if (pNewImage->valid()){
					unsigned short int * data1d = 0;
					data1d = new unsigned short int [x*y];
					data1d = (unsigned short int*)pNewImage->getRawData();
					for (V3DLONG i = 0; i< (x*y); i++)
					{
						total1dData[totalImageIndex]= data1d[i];
						if(data1d[i] > p_vmax) p_vmax = data1d[i];
						if(total1dData_mip[i] < data1d[i]) total1dData_mip[i] = data1d[i];
						totalImageIndex++;
					}
					if(data1d) {delete []data1d; data1d = 0;}
				}else{
					qDebug()<<imageDir.absoluteFilePath(fileList[f])<<" failed!";
				}

			}

		}


		Image4DSimple* total4DImage = new Image4DSimple;
		total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);

		if (!total4DImage->valid()){qDebug()<<"invalid preview image pointer!"; return;}

		LocationSimple tileLocation;
		tileLocation.x = scanList.value(loadScanNumber).getPixelLocation().x;// this is in pixels, using the expected origin
		tileLocation.y = scanList.value(loadScanNumber).getPixelLocation().y;

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
		QString imageSaveString = sessionDir.absolutePath();

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

		QDir xmlDir = QFileInfo(getFileString()).absoluteDir();
		QStringList newFilterList;
		newFilterList.append(QString("*.xml"));
		status("xml directory: "+xmlDir.absolutePath());
		xmlDir.setNameFilters(newFilterList);
		QStringList newFileList = xmlDir.entryList();
		if (!newFileList.isEmpty()){
			QFileInfo xmlInfo = QFileInfo(xmlDir.absoluteFilePath( newFileList.at(0)));
			QFile::copy( xmlInfo.absoluteFilePath(),sessionDir.absolutePath().append(QDir::separator()).append(xmlInfo.fileName()));
			qDebug()<<"copy finished from "<<xmlInfo.absoluteFilePath()<<" to "<< sessionDir.absolutePath().append(QDir::separator()).append(xmlInfo.fileName());

		}else{
			qDebug()<<"no xml file available";
		}



	}else{  //initial string is not readable
		status(QString("invalid image path: ").append(latestString));
	}
}



void S2UI::displayScan(){ // this will listen for a signal from myController
	//containing either a filename or  eventually an address

}

//----------  position monitor stuffs    ---------------
//
//======================================================
void S2UI::pmStatusHandler(bool pmStatus){
	posMonStatus = pmStatus;
	status("pmstatus updated");
}

void S2UI::posMonButtonClicked(){
	// if it's not running, start it
	// and change button text to 'stop pos mon'
	if (!posMonStatus){
		emit startPM();
		startS2PushButton->setText("s2 running");
		status("Position Monitor started");
		startPosMonButton->setText(tr("stop position monitor"));
		startSmartScanPB->setEnabled(true);
		qDebug()<<"posMonButtonClicked... about to updateCurrentZoom with tileSizeCB index "<<tileSizeCB->currentIndex();
		updateCurrentZoom(tileSizeCB->currentIndex());

	}else{
		emit stopPM();
		startPosMonButton->setText(tr("start position monitor"));
		startSmartScanPB->setEnabled(false);

	}
	// if it's running, stop it
	// and change text to start pos mon


}
void S2UI::updateS2Data( QMap<int, S2Parameter> currentParameterMap){
	// this updates the text fields in the UI, and ALSO CHECKS ON THE LATEST FILE and calls checkParameters to check for new values
	// not all values are currently updated in uiS2ParameterMap

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
		if (currentParameterMap[i].getExpectedType().contains("list")){ //this is the latest file!
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
		if (i ==0){ uiS2ParameterMap[i].setCurrentString(currentParameterMap[i].getCurrentString());}
		if (currentParameterMap[i].getExpectedType().contains("float")){
			if (currentParameterMap[i].getCurrentValue() != uiS2ParameterMap[i].getCurrentValue()){
				uiS2ParameterMap[i].setCurrentValue(currentParameterMap[i].getCurrentValue());
				if (i==18){
					roiXEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
				}else if (i==19){
					roiYEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
				}else if (i==13){
					roiXWEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
				}else if (i==14){
					roiYWEdit->setText(QString::number( uiS2ParameterMap[i].getCurrentValue()));
				}else if ((i==5)||(i==6)){
					updateROIPlot("");
				}
			}
		}
	}

}



void S2UI::updateString(QString broadcastedString){
}

//===================================================







void S2UI::tryXYMove(){
	LocationSimple testLG, testLS;
	testLG.x = 0;
	testLG.y = 0;
	testLG.ev_pc1 = uiS2ParameterMap[11].getCurrentValue();
	testLG.ev_pc2 = uiS2ParameterMap[12].getCurrentValue();
	testLS.x = tryStageXEdit->text().toFloat() ;
	testLS.y = tryStageYEdit->text().toFloat();
	TileInfo testTI = TileInfo(zoomPixelsProduct);
	testTI.setGalvoLocation(testLG);
	testTI.setStageLocation(testLS);
	moveToROIWithStage(testTI);
}










//  -------  smart scanning stuffs   --------
//
//======================================================


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
		summaryTextFile.close();
		emit eventSignal("finishedMultiTarget");
		return;
	}else{
		emit eventSignal("startMultiTarget");
		scanVoltageConversion = uiS2ParameterMap[8].getCurrentValue()/uiS2ParameterMap[17].getCurrentValue();  // convert from pixels to microns and to galvo voltage:

		runAllTargetsPB->setText("cancel All Targets");
		targetIndex = -1;
		allTargetStatus = 1;// running alltargetscan
		QTimer::singleShot(50, this, SLOT(handleAllTargets()));
	}
}
void S2UI::handleAllTargets(){
	qDebug()<<"handleAllTargets";
	if (waitingForFile==1){
		qDebug()<<"new delay loop in handleAllTargets...";
		QTimer::singleShot(1000, this, SLOT(handleAllTargets()));
		return;
	}




	if ((addBoundingBoxScan->isChecked()) & (targetIndex >=0) & (haventRunBoundingBox) & (!stageOnlyCB->isChecked())){
		smartScanStatus = 0;
		emit runBoundingBox();
		haventRunBoundingBox = false;
		QTimer::singleShot(100, this, SLOT(handleAllTargets()));
		return;
	}




	targetIndex++;
	colorIndex++;
	haventRunBoundingBox = true;
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
		summaryTextFile.close();
		emit eventSignal("finishedMultiTarget");
		myEventLogger->processEvents(eventLogString);
		return;
	}



	status("starting all targets");
	updateCurrentZoom(tileSizeCB->currentIndex());
	QTimer::singleShot(1000, this, SLOT(startingSmartScan()));}


void S2UI::startingSmartScan(){
	numProcessing=0;
	LocationSimple startLocation ;
	TileInfo startTileInfo = TileInfo(zoomPixelsProduct);

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
			//scanNumber = 0;
			loadScanNumber = 0;
			status("starting smartScan...");
			if (allTargetStatus ==0){
				startLocation = LocationSimple(uiS2ParameterMap[18].getCurrentValue()/uiS2ParameterMap[8].getCurrentValue(),
						uiS2ParameterMap[19].getCurrentValue()/uiS2ParameterMap[9].getCurrentValue(),
						0);

			}else{
				startLocation = allTargetLocations[targetIndex].getGalvoLocation();
			}




			startLocation.mass = 0;
			startLocation.ev_pc1 = uiS2ParameterMap[10].getCurrentValue();
			startLocation.ev_pc2 = uiS2ParameterMap[11].getCurrentValue();
			startTileInfo.setGalvoLocation(startLocation);

			float tileSize = uiS2ParameterMap[11].getCurrentValue();
			int minGrid = -(gridSizeSB->value()-1)/2;
			int maxGrid = -minGrid+1;
			for (int i=minGrid; i<maxGrid;i++){
				for (int j=minGrid; j<maxGrid;j++){
					LocationSimple gridLoc;
					gridLoc.x = startLocation.x+ ((float)i * (1.0-overlap))* tileSize;
					gridLoc.y = startLocation.y+ ((float)j * (1.0-overlap))* tileSize;
					gridLoc.ev_pc1 = uiS2ParameterMap[10].getCurrentValue();
					gridLoc.ev_pc2 = uiS2ParameterMap[11].getCurrentValue();
					qDebug()<<"grid x = "<< gridLoc.x<<" grid y = "<<gridLoc.y;
					TileInfo gridTileInfo = TileInfo(zoomPixelsProduct);
					gridTileInfo.setGalvoLocation(gridLoc);
					allROILocations->append(gridTileInfo);

				}
			}





			if (allTargetStatus ==0)   {allTargetLocations.append(startTileInfo); // keep track of targets, even when not using the multi-target sequence

			}
			if (runContinuousCB->isChecked()){
				qDebug()<<"allROILocations length "<<allROILocations->length();
				s2ROIMonitor();
			}else{        qDebug()<<"headed to smartscanHandler";
				QTimer::singleShot(10,this, SLOT(smartScanHandler()));}
		}
		return;
	}
	myScanMonitor->startNewScan();
	qDebug()<<"starting smartscan";
	if (smartScanStatus==1){
		smartScanStatus=0;
		waitingForLast = false;
		startSmartScanPB->setText("smartScan");
		allROILocations->clear();
		allTipsList->clear();
		saveTextFile.close();
		summaryTextFile.close();
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


	// new file with scan summary information

	QString summaryFileString =saveDir.absolutePath().append("/").append("s2Summary.txt");

	summaryTextFile.setFileName(summaryFileString);
	if (!summaryTextFile.isOpen()){
		if (!summaryTextFile.open(QIODevice::Text|QIODevice::WriteOnly)){
			qDebug()<<"couldnt open file"<<summaryFileString;
			return;}     }

	QTextStream summaryTextStream;
	summaryTextStream.setDevice(&summaryTextFile);
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
	summaryTextStream<<"vaa3d_tools git hash at build: "<<";"<<GIT_CURRENT_SHA1<<"\n";
#endif

	summaryTextStream<<"s2Scan start time: "<<";"<< QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm_ss_zzz")<<"\n";
	summaryTextStream<<"s2Scan Save Directory: "<<";"<< saveDir.absolutePath()<<"\n";


	startSmartScanPB->setText("cancel smartScan");
	if (allROILocations->isEmpty()){ // start smartscan of new target
		scanList.clear();
		tipList.clear();
		//scanNumber = 0;
		loadScanNumber = 0;
		status("starting smartScan...");

		if (allTargetStatus ==0){
			startLocation = LocationSimple(uiS2ParameterMap[18].getCurrentValue()/uiS2ParameterMap[8].getCurrentValue(),
					uiS2ParameterMap[19].getCurrentValue()/uiS2ParameterMap[9].getCurrentValue(),
					0);
			startLocation.mass = 0;
			startLocation.ev_pc1 = uiS2ParameterMap[10].getCurrentValue();// size of first block is set here for these non-target scans
			startLocation.ev_pc2 = uiS2ParameterMap[11].getCurrentValue();
			startTileInfo.setGalvoLocation(startLocation);

		}else{
			startTileInfo = allTargetLocations[targetIndex];
		}






		totalImagingTime=0.0;
		totalAnalysisTime=0.0;
		scanStartTime = QDateTime::currentDateTime();
		// add the starting location to the ROI queue:
		int v = startTileInfo.setTimeStamp(QDateTime::currentDateTime());
		allROILocations->append(startTileInfo);






		if (allTargetStatus ==0)   allTargetLocations.append(startTileInfo); // keep track of targets, even when not using the multi-target sequence

		if (runContinuousCB->isChecked()){
			s2ROIMonitor();
		}else{        qDebug()<<"headed to smartscanHandler";
			QTimer::singleShot(10,this, SLOT(smartScanHandler()));}
	}
	// append text to noteTaker
	//

}

void S2UI::handleNewLocation(QList<LandmarkList> newTipsList, LandmarkList newLandmarks,  Image4DSimple* mip, double scanIndex, QString tileSaveString, int tileStatus){


	int incomingTileStatus= tileStatus;

	if (incomingTileStatus!=-1){
		QTimer::singleShot(0,this, SLOT(processingFinished()));
		qDebug()<<"back in S2UI with new locations, incoming tilestatus = "<<incomingTileStatus ;
		status(QString("got ").append(QString::number( newLandmarks.length())).append("  new landmarks associated with ROI "). append(QString::number(scanIndex)));
		qDebug()<<QString("got ").append(QString::number( newLandmarks.length())).append("  new landmarks associated with ROI "). append(QString::number(scanIndex));
	}




	tileNotes->setText(QString("tracing info : "));

	//  add end-time for analysis of tile scanIndex here



	for (int i = 0; i<newLandmarks.length(); i++){
		int incomingX=0;
		int incomingY=0;
		if (!newTipsList.value(i).empty()){
			incomingX = (int) newLandmarks.value(i).x;
			incomingY = (int) newLandmarks.value(i).y;
			TileInfo landmarkTileInfo = TileInfo(zoomPixelsProduct);
			LocationSimple stageLandmark;
			stageLandmark.x = newLandmarks[i].mcenter.x; // this is the stage location of the tile that is the PARENT OF THIS TILE!
			stageLandmark.y = newLandmarks[i].mcenter.y; // this will work fine for the fixed-stage scans because the stage won't move.



			newLandmarks[i].x = newLandmarks[i].x+((float) newLandmarks[i].ev_pc1)/2.0;// shift incoming landmarks from upper left origin back to the tile center
			newLandmarks[i].y = newLandmarks[i].y+((float) newLandmarks[i].ev_pc2)/2.0;//  this is the conversion from upper left (image) coordinates used in stackAnalyzer back to center-of-scan coordinates




			LocationSimple pixelsLandmark = newLandmarks.value(i);   // pixelsLandmark is the tile position in pixels, including the stage information

			// and remove the stage position offset to get the galvo-only position


			newLandmarks[i].x =newLandmarks[i].x - (newLandmarks[i].mcenter.x / uiS2ParameterMap[8].getCurrentValue());
			newLandmarks[i].y = newLandmarks[i].y -(newLandmarks[i].mcenter.y/ uiS2ParameterMap[9].getCurrentValue());


			if (stageOnlyCB->isChecked()){  // but for the stage-only scans, we need the stageLandmark to reflect the center of the tile position
				// and the galvo position to be zero.
				stageLandmark.x = pixelsLandmark.x*uiS2ParameterMap[8].getCurrentValue();
				stageLandmark.y = pixelsLandmark.y*uiS2ParameterMap[9].getCurrentValue();
				newLandmarks[i].x = 0; //galvo position
				newLandmarks[i].y = 0;
				newLandmarks[i].mcenter.x=stageLandmark.x;
				newLandmarks[i].mcenter.y=stageLandmark.y;
			}



			landmarkTileInfo.setGalvoLocation(newLandmarks[i]);   // now without stage info
			landmarkTileInfo.setStageLocation(stageLandmark);   // stage info only, in microns
			landmarkTileInfo.setPixelLocation(pixelsLandmark);  // pixelsLandmark is the tile position in pixels, including the stage information
			landmarkTileInfo.setFileString(tileSaveString); // careful, this is the parent savestring

			myMutex.lock(); // if incoming landmarks from different parent tiles get queued out of order, that's OK, as long as allROILocations is unmodified during  append() and isDuplicateROI() methods for one incoming landmark

			if (!isDuplicateROI(landmarkTileInfo)|sendThemAllCB->isChecked()){ // this currently ONLY checks based on pixelLocation.
				qDebug()<<"NOT duplicate tile "<<scanIndex;

				int v = landmarkTileInfo.setTimeStamp(QDateTime::currentDateTime());  // 1st timestamp is when tile is added to queue
				allROILocations->append(landmarkTileInfo);
				allTipsList->append(newTipsList.value(i));
				// add ROI to ROI plot. by doing this here, we should limit the overhead without having to worry about
				// keeping track of a bunch of ROIs.
				QPen myPen =  QPen(makeQColorFromIndex(10, colorIndex), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
				roiGS->addRect((pixelsLandmark.x-((float)pixelsLandmark.ev_pc1)/2.0)*uiS2ParameterMap[8].getCurrentValue(), (pixelsLandmark.y-((float)pixelsLandmark.ev_pc2)/2.0)*uiS2ParameterMap[9].getCurrentValue(),
						((float)pixelsLandmark.ev_pc1)*uiS2ParameterMap[8].getCurrentValue(), ((float)pixelsLandmark.ev_pc2)*uiS2ParameterMap[9].getCurrentValue(),  myPen);
			}else{



				//check for the .v3draw file
				QString fileFinder;
				QStringList fileFilter;
				QFileInfoList fileInfoList;
				int correctX = incomingX;
				int correctY = incomingY;
				bool foundIt = false;
				fileFilter.clear();
				// need to check for +/- 4 due to vaguely rounded tile locations.

				for (int jj = incomingX-4; jj<=incomingX+4; jj++){
					for (int kk = incomingY-4; kk<= incomingY+4; kk++){
						fileFinder = QString("x_").append(QString::number(jj)).append("_y_").append(QString::number(kk)).append("*.v3draw");
						fileFilter.append(fileFinder);
						fileInfoList = saveDir.entryInfoList(fileFilter);
						if (!fileInfoList.isEmpty()){
							correctX = jj;
							correctY = kk;
							foundIt=true;
							break;
						}else{
							correctX = incomingX;
							correctY = incomingY;
						}
					}
					if ((foundIt)&((incomingX!=correctX)|(incomingY!=correctY))) {
						qDebug()<<"original xy location "<<incomingX<<" "<<incomingY<<" final location "<<correctX<<" "<<correctY;

						break;}
					if (foundIt) break;
				}

				QString putativeV3draw;
				putativeV3draw = QString("x_").append(QString::number(correctX)).append("_y_").append(QString::number(correctY)).append("*.v3draw");
				fileFilter.append(putativeV3draw);
				fileInfoList = saveDir.entryInfoList(fileFilter);
				if (fileInfoList.isEmpty()){
					tileStatus = -1;
					landmarkTileInfo.setFileString(saveDir.absoluteFilePath("unknownV3DRaw*"));
				}else{
					landmarkTileInfo.setFileString(fileInfoList.at(0).absoluteFilePath());
				}
				//check for swc file
				QString putativeSWC;
				fileFilter.clear();
				putativeSWC = QString("x_").append(QString::number(correctX)).append("_y_").append(QString::number(correctY)).append("*").append(channelChoiceComboB->currentText()).append(".swcX");
				fileFilter.append(putativeSWC);
				fileInfoList = saveDir.entryInfoList(fileFilter);


				if (fileInfoList.isEmpty()){
					//  wait until tracing is done
					tileNotes->setText(QString("tracing info : waiting for .swc ").append(putativeSWC));
					tileStatus = -1;
					emit waitForDuplicate(landmarkTileInfo, newTipsList.at(i), tileStatus, correctX, correctY, tracingMethodComboB->currentText());
					qDebug()<<"waitForDuplicate...";
				} else if (fileInfoList.at(0).isReadable()){
					tileStatus = 1;
					//   tracing is done, .swc is readable, send it back for analysis
					loadDuplicateTile(landmarkTileInfo, newTipsList.at(i), tileStatus, correctX, correctY);
				} else{
					qDebug()<<"SWC file exists but isnt readable "<<fileInfoList.at(0).absoluteFilePath();

				}

				if (tileStatus !=-1){
				qDebug()<<"duplicate tile "<<scanIndex;
				qDebug()<<QString("dup: x=").append(QString::number(incomingX)).append(" y=").append(QString::number(incomingY));
				}

			}
			myMutex.unlock();

		}


	}
	//
	if (incomingTileStatus !=-1){
		emit loadMIPSignal(scanIndex, mip, tileSaveString);
		QTimer::singleShot(10,this, SLOT(smartScanHandler()));
	}

	myNotes->save();



}


void S2UI::runBoundingBox(){
	LocationSimple boundingBoxLocation;
	if (scanList.length()==0) return;
	float leftEdge = 10000;
	float topEdge = -10000;
	float bottomEdge = 10000;
	float rightEdge = -10000;
	for (int i = 0; i<scanList.length(); i++){
		if ( (scanList[i].getGalvoLocation().x-(scanList[i].getGalvoLocation().ev_pc1/2.0))<leftEdge ) leftEdge = scanList[i].getGalvoLocation().x-(scanList[i].getGalvoLocation().ev_pc1/2.0);
		if ( (scanList[i].getGalvoLocation().x+(scanList[i].getGalvoLocation().ev_pc1/2.0))>rightEdge) rightEdge = scanList[i].getGalvoLocation().x+(scanList[i].getGalvoLocation().ev_pc1/2.0);
		if ( (scanList[i].getGalvoLocation().y+(scanList[i].getGalvoLocation().ev_pc2/2.0))>topEdge ) topEdge = scanList[i].getGalvoLocation().y+(scanList[i].getGalvoLocation().ev_pc2/2.0);
		if ( (scanList[i].getGalvoLocation().y-(scanList[i].getGalvoLocation().ev_pc2/2.0))<bottomEdge ) bottomEdge = scanList[i].getGalvoLocation().y-(scanList[i].getGalvoLocation().ev_pc2/2.0);


	}

	qDebug()<<"leftEdge = "<<leftEdge;
	qDebug()<<"rightEdge= "<<rightEdge;
	qDebug()<<"topEdge = "<<topEdge;
	qDebug()<<"bottomEdge ="<<bottomEdge;
	boundingBoxLocation.x = (leftEdge +rightEdge)/2.0;
	boundingBoxLocation.y = (topEdge + bottomEdge)/2.0;

	boundingBoxLocation.ev_pc1 = rightEdge -leftEdge;
	boundingBoxLocation.ev_pc2 = topEdge-bottomEdge;
	if (boundingBoxLocation.ev_pc1 > boundingBoxLocation.ev_pc2){
		boundingBoxLocation.ev_pc2 = boundingBoxLocation.ev_pc1;
	}else{
		boundingBoxLocation.ev_pc1 = boundingBoxLocation.ev_pc2;
	}

	LocationSimple boundingBoxStageLocation = scanList[0].getStageLocation();  // boundingboxes are ill-defined for stage scans.  this will just put the stage at the position it was for the first tile.
	LocationSimple boundingBoxPixelLocation = boundingBoxLocation;
	boundingBoxPixelLocation.x = boundingBoxLocation.x+boundingBoxStageLocation.x/uiS2ParameterMap[8].getCurrentValue();
	boundingBoxPixelLocation.y = boundingBoxLocation.y+boundingBoxStageLocation.y/uiS2ParameterMap[9].getCurrentValue();


	qDebug()<<"boundingboxlocation.x "<<boundingBoxLocation.x;
	qDebug()<<"boundingboxlocation.y "<<boundingBoxLocation.y;
	qDebug()<<"boundingboxlocation.ev_pc1 "<<boundingBoxLocation.ev_pc1;
	qDebug()<<"boundingboxlocation.ev_pc2 "<<boundingBoxLocation.ev_pc2;
	TileInfo bbTileInfo = TileInfo(zoomPixelsProduct);
	bbTileInfo.setGalvoLocation(boundingBoxLocation);
	bbTileInfo.setStageLocation(boundingBoxStageLocation);
	bbTileInfo.setPixelLocation(boundingBoxLocation);
	allROILocations->append(bbTileInfo);


}


bool S2UI::isDuplicateROI(TileInfo inputTileInfo){

	LocationSimple inputLocation = inputTileInfo.getPixelLocation();

	bool upperLeft =false;
	bool upperRight = false;
	bool lowerLeft = false;
	bool lowerRight = false;
	bool upperLeftI =false;
	bool upperRightI = false;
	bool lowerLeftI = false;
	bool lowerRightI = false;
	for (int i=0; i<scanList.length(); i++){
		// first check if the xy coordinates are already in scanList  this is IN PIXELS, including stage info.
		if ((qAbs(inputLocation.x - scanList[i].getPixelLocation().x)< (float) 5.0) && (qAbs(inputLocation.y - scanList[i].getPixelLocation().y)< (float) 5.0)){
			return true;
		}else{// then check if all 4 corners and 4 inner points are in any volume in scanList (critical for adaptive scanning)

			upperLeft = upperLeft || ((inputLocation.x-(inputLocation.ev_pc1/2.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/2.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/2.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/2.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));

			upperRight = upperRight || ((inputLocation.x+(inputLocation.ev_pc1/2.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/2.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/2.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/2.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));
			lowerLeft = lowerLeft || ((inputLocation.x-(inputLocation.ev_pc1/2.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/2.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/2.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/2.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));
			lowerRight = lowerRight || ((inputLocation.x+(inputLocation.ev_pc1/2.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/2.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/2.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/2.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));

			// and internal locations at central 1/4 of the tile.  this will work for fixed-tile scans as long as overlap isn't 25% or more.

			upperLeftI = upperLeftI || ((inputLocation.x-(inputLocation.ev_pc1/4.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/4.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/4.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/4.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));

			upperRightI = upperRightI || ((inputLocation.x+(inputLocation.ev_pc1/4.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/4.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/4.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/4.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));
			lowerLeftI = lowerLeftI || ((inputLocation.x-(inputLocation.ev_pc1/4.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/4.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/4.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/4.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));
			lowerRightI = lowerRightI || ((inputLocation.x+(inputLocation.ev_pc1/4.0) <= scanList[i].getPixelLocation().x+(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/4.0) >= scanList[i].getPixelLocation().x-(scanList[i].getPixelLocation().ev_pc1/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/4.0) <= scanList[i].getPixelLocation().y+(scanList[i].getPixelLocation().ev_pc2/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/4.0) >= scanList[i].getPixelLocation().y-(scanList[i].getPixelLocation().ev_pc2/2.0)));


			if (upperLeft&&upperRight&&lowerLeft&&lowerRight&&upperLeftI&&upperRightI&&lowerLeftI&&lowerRightI){
				return true;}
		}
	}
	// repeat on locations already queued!
	for (int i=0; i< allROILocations->length(); i++){
		LocationSimple iPixelLoc = allROILocations->value(i).getPixelLocation();
		if ((qAbs(inputLocation.x - iPixelLoc.x)<5.0) && (qAbs(inputLocation.y - iPixelLoc.y)<(float) 5.1)){
			return true;
		}else{

			upperLeft = upperLeft || ((inputLocation.x-(inputLocation.ev_pc1/2.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/2.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/2.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/2.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));

			upperRight = upperRight || ((inputLocation.x+(inputLocation.ev_pc1/2.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/2.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/2.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/2.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));
			lowerLeft = lowerLeft || ((inputLocation.x-(inputLocation.ev_pc1/2.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/2.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/2.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/2.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));
			lowerRight = lowerRight || ((inputLocation.x+(inputLocation.ev_pc1/2.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/2.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/2.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/2.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));


			upperLeftI = upperLeftI || ((inputLocation.x-(inputLocation.ev_pc1/4.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/4.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/4.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
									  (inputLocation.y-(inputLocation.ev_pc2/4.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));

			upperRightI = upperRightI || ((inputLocation.x+(inputLocation.ev_pc1/4.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/4.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/4.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
										(inputLocation.y-(inputLocation.ev_pc2/4.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));
			lowerLeftI = lowerLeftI || ((inputLocation.x-(inputLocation.ev_pc1/4.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.x-(inputLocation.ev_pc1/4.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/4.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
									  (inputLocation.y+(inputLocation.ev_pc2/4.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));
			lowerRightI = lowerRightI || ((inputLocation.x+(inputLocation.ev_pc1/4.0) <= iPixelLoc.x+(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.x+(inputLocation.ev_pc1/4.0) >= iPixelLoc.x-(iPixelLoc.ev_pc1/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/4.0) <= iPixelLoc.y+(iPixelLoc.ev_pc2/2.0) ) &&
										(inputLocation.y+(inputLocation.ev_pc2/4.0) >= iPixelLoc.y-(iPixelLoc.ev_pc2/2.0)));


			if (upperLeft&&upperRight&&lowerLeft&&lowerRight&&upperLeftI&&upperRightI&&lowerLeftI&&lowerRightI){
				return true;}
		}
	}
	bool outsideOverview = false;
	if (stageOnlyCB->isChecked()){ //ignore overview boundaries for stage-only scans.
		return false;
	}

	LocationSimple    inputGalvoLocation = inputTileInfo.getGalvoLocation();
	//now check if the tile location is outside the original overview volume
	int leftSide = ((inputGalvoLocation.x-(inputGalvoLocation.ev_pc1/2.0))/overViewPixelToScanPixel +256);// this is the left side of the tile in overview Pixels
	qDebug()<<"left side = "<<leftSide;
	if (leftSide  <= 0){outsideOverview = true;};

	int rightSide = ((inputGalvoLocation.x+(inputGalvoLocation.ev_pc1/2.0))/overViewPixelToScanPixel +256);// this is the right side of the tile in overview Pixels
	qDebug()<<"right side = "<<rightSide;
	if (leftSide  > 512){outsideOverview = true;};

	int topSide = ((inputGalvoLocation.y-(inputGalvoLocation.ev_pc2/2.0))/overViewPixelToScanPixel +256);// this is the top side of the tile in overview Pixels
	qDebug()<<"top side = "<<topSide;
	if (topSide  <= 0){outsideOverview = true;};

	int bottomSide = ((inputGalvoLocation.y+(inputGalvoLocation.ev_pc2/2.0))/overViewPixelToScanPixel +256);// this is the bottom side of the tile in overview Pixels
	qDebug()<<"bottom side = "<<bottomSide;
	if (bottomSide  > 512){outsideOverview = true;};


	if (outsideOverview){
		status("tile outside overview scan area");
		return true;
	}


	return false;



}





void S2UI::smartScanHandler(){
	// this method does a bit of flow control for s2scans and is an off-ramp for the deprecated, non-continuous acquisition mode.
	if (smartScanStatus!=1){
		status("smartScan aborted");
		//scanNumber = 0;
		loadScanNumber = 0;
		saveTextFile.close();
		summaryTextFile.close();
		emit processSmartScanSig(scanDataFileString);
		if (allTargetStatus ==1){
			QTimer::singleShot(0, this, SLOT(handleAllTargets()));
		}else{        myEventLogger->processEvents(eventLogString);}
		return;
	}
	if ((allROILocations->isEmpty())&&(!waitingForLast)&&(scanList.length()==(myScanMonitor->allScanData.last().getAllTileInfo().length()))){//scanNumber is incremented AFTER the tracing results come in
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
		LocationSimple iPixelLoc  = allROILocations->value(i).getPixelLocation();
		status(QString("x= ").append(QString::number(iPixelLoc.x)).append(" y = ").append(QString::number(iPixelLoc.y)).append(" z= ").append(QString::number(iPixelLoc.z)));
	}

	if ((!allROILocations->isEmpty()) || (waitingForLast)){


		if (runContinuousCB->isChecked()){
			qDebug()<<"letting s2ROIMonitor initiate scans";
		}else{
			TileInfo nextLocation = allROILocations->first();
			LandmarkList  nextLandmarkList;
			if (allTipsList->isEmpty()){
				qDebug()<<"no incoming tip locations";
				// leave nextLandmarkList empty and don't touch allTipsList
			}else{
				nextLandmarkList = allTipsList->first();
				allTipsList->removeFirst();
			}
			QStringList a = allROILocations->first().getTimeStrings();
			for (int i = 0; i<a.length(); i++) qDebug()<<a.at(i);
			allROILocations->removeFirst();
			moveToROI(nextLocation);


			waitingForFile = 1;
			scanList.append(nextLocation);
			if (targetIndex < allScanLocations.length()){
				allScanLocations[targetIndex].append(nextLocation.getPixelLocation());
			}else{
				LandmarkList starterList;
				starterList.append(nextLocation.getPixelLocation());
				allScanLocations.append(starterList);
			}
			emit updateTable(allTargetLocations,allScanLocations);
			emit eventSignal("startZStack");
			QTimer::singleShot(100, &myController, SLOT(startZStack())); //hardcoded delay here... not sure
			// how to make this more eventdriven. maybe  wait for move to finish.
			status(QString("start next ROI at x = ").append(QString::number(nextLocation.getPixelLocation().x)).append("  y = ").append(QString::number(nextLocation.getPixelLocation().y)));
		}
	}

}

void S2UI::s2ROIMonitor(){ // continuous acquisition mode
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
		TileInfo nextLocation = allROILocations->first();
		allROILocations->removeFirst();


		moveToROI(nextLocation);
		TileInfo nextTileInfo = TileInfo(zoomPixelsProduct);
		nextTileInfo.setPixels((int) nextLocation.getPixelLocation().ev_pc1);


		currentTileInfo = nextTileInfo;
		QString sString =currentTileInfo.getTileInfoString().join(" _ ");
		status("currentTileInfo : "+sString);
		qDebug()<<sString;
		waitingForFile = 1;
		int v = nextLocation.setTimeStamp(QDateTime::currentDateTime());  // 2nd timestamp when tile is sent to the microscope for imaging





		scanList.append(nextLocation);
		if (targetIndex < allScanLocations.length()){
			allScanLocations[targetIndex].append(nextLocation.getPixelLocation());
		}else{
			LandmarkList starterList;
			starterList.append(nextLocation.getPixelLocation());
			allScanLocations.append(starterList);
		}




		emit updateTable(allTargetLocations,allScanLocations);
		status(QString("start next ROI at x = ").append(QString::number(nextLocation.getPixelLocation().x)).append("  y = ").append(QString::number(nextLocation.getPixelLocation().y)));
		waitingToStartStack = true;
		emit updateZoom(); // when waitingToStartStack is true, updateZoom will finish by executing a z stack.
	}
	if ((gridScanStatus ==1) && (allROILocations->length() == 0)){gridScanStatus = -1; return;}

	if (((smartScanStatus ==1)&&(runContinuousCB->isChecked()))||(gridScanStatus==1)) {
		QTimer::singleShot(10, this, SLOT(s2ROIMonitor()));
	}
}
void S2UI::moveToROI(const TileInfo nextROI){
	TileInfo myNextROI = nextROI;

	//  currently the stage info is not actually making it out to the stackAnalyzer, so the code is actually working only based on turning the 'pixel location'
	// sent to stackAnalyzer into the  stage location, with the galvo location = 0  as done here.
	// somehow this is broken for the first location, probably because I'm actually trying to use the stageLocation information.

	// critical: with only galvo scanning, the initial tile is correct but subsequent s2 tiles are inverted in y relative to the original tile
	//           with only stage scanning, the initial tile is wrong in its physical location (the tile is collected roughly 1/2 tile south of the intended
	//            location.)  but all other scans are correctly located.


	// more checking indicates that both schemes are really quite screwed up.  a complete rework is necessary.

	//  1. pass the stage information with each tile to stackAnalyzer.
	//     If I need a new list for this, so be it.  THIS INFORMATION WILL BE IGNORED BY stackAnalyzer
	//  2. there are some sign errors with the stage coordinates that are screwing up pretty much everything.
	//     the discrepancy needs to be fixed in one place, not smeared all over the code.
	//     right now the information (including sign) is correct for the live data (uis2parametermap) and the ROI plotter
	//     right now the stage-based scanning generates correct relation between mip tile and planned location.  galvo version swaps y coordinate around the starting tile.


	// 1. is done, using the .mcenter field of the struct.
	// 2. still needs work.  the 'pixel' stuff basically guarantees correct relative placement now, but the absolute location is wrong.
	//  also, the width parameter of the live thing may need to be corrected in the x direction. if something is being abs() to make a width, but the
	// direction is reversed, the plot needs to show that.

	//  the y sign flip may be fixed.  now it's just x?


	if( posMonStatus){

		if (stageOnlyCB->isChecked()){     // for stage mode, set galvo position to zero and adjust stage position accordingly...
			LocationSimple nextStageLocation;
			LocationSimple nextGalvoLocation = nextROI.getGalvoLocation();
			qDebug()<<"initial stageLocation.x ="<<nextStageLocation.x;
			nextStageLocation.x = nextROI.getStageLocation().x+ nextROI.getGalvoLocation().x*uiS2ParameterMap[8].getCurrentValue(); // there may be a tile size issue here
			nextStageLocation.y = nextROI.getStageLocation().y+ nextROI.getGalvoLocation().y*uiS2ParameterMap[9].getCurrentValue();
			nextGalvoLocation.x = 0.0;
			nextGalvoLocation.y = 0.0;
			qDebug()<<"final stageLocation.x ="<<nextStageLocation.x;

			myNextROI.setGalvoLocation(nextGalvoLocation);
			myNextROI.setStageLocation(nextStageLocation);


			moveToROIWithStage(myNextROI);
			return;
		}

		LocationSimple newLoc;
		newLoc.x = -myNextROI.getGalvoLocation().x*scanVoltageConversion;
		newLoc.y = myNextROI.getGalvoLocation().y*scanVoltageConversion;
		emit moveToNext(newLoc);
	}else{
		status("start PosMon before moving galvos");
		smartScanStatus = -1;
	}}


void S2UI::moveToROIWithStage(const TileInfo nextROI){
	if( posMonStatus){

		float xStage =  nextROI.getStageLocation().x;
		float yStage = -nextROI.getStageLocation().y;



		// First check the stage position arguments.  Is the move too big?

		float xDiff = qAbs(xStage -uiS2ParameterMap[5].getCurrentValue());
		float yDiff = qAbs(yStage +uiS2ParameterMap[6].getCurrentValue());
		qDebug()<<"xDiff = "<<xDiff;
		qDebug()<<"yDiff = "<<yDiff;
		if ((xDiff>2000.0)|| (yDiff > 2000.0)){
			qDebug()<<"stage move too large!";
			return;

		}


		LocationSimple newLoc;
		newLoc.x = -nextROI.getGalvoLocation().x*scanVoltageConversion;
		newLoc.y = nextROI.getGalvoLocation().y*scanVoltageConversion;

		emit moveToNextWithStage(newLoc, xStage, yStage);
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


void S2UI::loadLatest(QString inputString){
	if ((smartScanStatus ==1)||(gridScanStatus!=0)){
		LandmarkList seedList;
		qDebug()<<"loadlatest smartscan";
		qDebug()<<"tipList length "<<tipList.length()<<" loadScanNumber "<<loadScanNumber;
		if (!tipList.isEmpty()){

			seedList = tipList.at(loadScanNumber);
			qDebug()<<"seedList length "<<seedList.length();
		}
		LocationSimple tileLocation;
		int v = scanList[loadScanNumber].setTimeStamp(QDateTime::currentDateTime()); //3rd timestamp when imaging is done


		tileLocation.ev_pc1 = scanList.value(loadScanNumber).getGalvoLocation().ev_pc1;
		tileLocation.ev_pc2 = scanList.value(loadScanNumber).getGalvoLocation().ev_pc1;
		// outgoing landmarks are shifted to the tile upper left
		tileLocation.x = scanList.value(loadScanNumber).getGalvoLocation().x-((float)  scanList.value(loadScanNumber).getGalvoLocation().ev_pc1)/2.0;
		tileLocation.y = scanList.value(loadScanNumber).getGalvoLocation().y-((float)  scanList.value(loadScanNumber).getGalvoLocation().ev_pc2)/2.0;
		// and the stage location is added to the tile landmark
		tileLocation.x = tileLocation.x + (scanList.value(loadScanNumber).getStageLocation().x/ uiS2ParameterMap[8].getCurrentValue());
		tileLocation.y = tileLocation.y + (scanList.value(loadScanNumber).getStageLocation().y/ uiS2ParameterMap[9].getCurrentValue());
		// throw the stage location along for the ride to the  StackAnalyzer, because when it comes back, we'll need to subtract it again.
		tileLocation.mcenter.x = scanList.value(loadScanNumber).getStageLocation().x;
		tileLocation.mcenter.y = scanList.value(loadScanNumber).getStageLocation().y;
		tileLocation.ave= loadScanNumber;

		tileLocation.pixmax = scanList.value(loadScanNumber).getGalvoLocation().pixmax;
		tileLocation.pixval = scanList.value(loadScanNumber).getGalvoLocation().pixval;
		qDebug()<<"tileLocation.x = "<<tileLocation.x;
		qDebug()<<"seedList is empty? "<<seedList.isEmpty();
		bool isSoma = loadScanNumber==0;
		int tileStatus=0;
		if (gridScanStatus!=0){
			emit eventSignal("startGridLoad");

			emit  callSAGridLoad(getFileString(),   tileLocation, saveDir.absolutePath() );

		}else{
			bool isAdaptive = false;
			int methodChoice = 0;
			emit eventSignal("startAnalysis");
			QTimer::singleShot(0,this, SLOT(processingStarted()));


			if (tracingMethodComboB->currentIndex()==0){ //MOST
				methodChoice = 0;
				isAdaptive = false;

			}
			if (tracingMethodComboB->currentIndex()==1){ //APP2
				methodChoice = 2;
				isAdaptive = false;
			}
			if (tracingMethodComboB->currentIndex()==2){ //Neutube
				methodChoice = 1;

			}
			if (tracingMethodComboB->currentIndex()==3){ //adaptive MOST
				methodChoice = 0;
				isAdaptive  = true;

			}
			if (tracingMethodComboB->currentIndex()==4){ //adaptive APP2
				methodChoice = 2;
				isAdaptive = true;

			}
			if (tracingMethodComboB->currentIndex()==5){ //adaptive Neutube
				methodChoice= 1;
				isAdaptive = true;

			}
			if (tracingMethodComboB->currentIndex()==6){ //automatic
				methodChoice= -1;
				isAdaptive = true;

			}

			if (tracingMethodComboB->currentIndex()==7){ //debugging mode
				methodChoice= 3;
				isAdaptive = false;

			}
			qDebug()<<"isadaptive = "<<isAdaptive;
			qDebug()<<"methodChoice = "<<methodChoice;
			int internalThreadNumber = traceThreadNumber; // otherwise traceThreadNumber can be incremented during this sequence by other calls/threads

			if (multiThreadTracingCB->isChecked()){
				if (internalThreadNumber==0){
					emit callSATrace(inputString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
									 this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);


				}else if (internalThreadNumber==1){
					emit callSATrace0(inputString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
									  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice,  tileStatus);
				}else if (internalThreadNumber==2){
					emit callSATrace1(inputString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
									  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice,  tileStatus);
				}else if (internalThreadNumber==3){
					emit callSATrace2(inputString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
									  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice,  tileStatus);
				}




			}else{
				emit callSATrace(inputString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
								 this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
			}


			internalThreadNumber++;
			internalThreadNumber = internalThreadNumber%4;
			traceThreadNumber = internalThreadNumber;


			QDir xmlDir = QFileInfo(inputString).absoluteDir();
			QStringList newFilterList;
			newFilterList.append(QString("*.xml"));
			status("xml directory: "+xmlDir.absolutePath());
			xmlDir.setNameFilters(newFilterList);
			QStringList newFileList = xmlDir.entryList();
			if (!newFileList.isEmpty()){
				QFileInfo xmlInfo = QFileInfo(xmlDir.absoluteFilePath( newFileList.at(0)));
				QFile::copy( xmlInfo.absoluteFilePath(),saveDir.absolutePath().append(QDir::separator()).append(xmlInfo.fileName()));
				qDebug()<<"copy finished from "<<xmlInfo.absoluteFilePath()<<" to "<< saveDir.absolutePath().append(QDir::separator()).append(xmlInfo.fileName());

			}else{
				qDebug()<<"no xml file available";
			}



		}

		loadScanNumber++;
	}else{
		loadScanFromFile(inputString);
	}
	// if there's an .xml file in the filestring directory, copy it to the save directory:


}


void S2UI::loadDuplicateTile(TileInfo duplicateTile, LandmarkList seedList, int tileStatus, int correctX, int correctY){

	QString tileFileString = duplicateTile.getFileString();


	LocationSimple tileLocation;
	tileLocation.ev_pc1 = duplicateTile.getGalvoLocation().ev_pc1;
	tileLocation.ev_pc2 = duplicateTile.getGalvoLocation().ev_pc2;
	// outgoing landmarks are shifted to the tile upper left
   // tileLocation.x = duplicateTile.getGalvoLocation().x-((float)  duplicateTile.getGalvoLocation().ev_pc1)/2.0;
   // tileLocation.y = duplicateTile.getGalvoLocation().y-((float)  duplicateTile.getGalvoLocation().ev_pc2)/2.0;
	// and the stage location is added to the tile landmark
	tileLocation.x = correctX; //tileLocation.x + (duplicateTile.getStageLocation().x/ uiS2ParameterMap[8].getCurrentValue());
	tileLocation.y = correctY; // tileLocation.y + (duplicateTile.getStageLocation().y/ uiS2ParameterMap[9].getCurrentValue());
	// throw the stage location along for the ride to the  StackAnalyzer, because when it comes back, we'll need to subtract it again.
	tileLocation.mcenter.x = duplicateTile.getStageLocation().x;
	tileLocation.mcenter.y = duplicateTile.getStageLocation().y;
	tileLocation.ave= -1;
	tileLocation.pixmax = duplicateTile.getGalvoLocation().pixmax;
	tileLocation.pixval = duplicateTile.getGalvoLocation().pixval;

	bool isSoma = false;
	bool isAdaptive = false;
	int methodChoice = 0;
	if (tileStatus !=-1){
		emit eventSignal("startAnalysis");
		QTimer::singleShot(0,this, SLOT(processingStarted()));}


	if (tracingMethodComboB->currentIndex()==0){ //MOST
		methodChoice = 0;
		isAdaptive = false;

	}
	if (tracingMethodComboB->currentIndex()==1){ //APP2
		methodChoice = 2;
		isAdaptive = false;
	}
	if (tracingMethodComboB->currentIndex()==2){ //Neutube
		methodChoice = 1;

	}
	if (tracingMethodComboB->currentIndex()==3){ //adaptive MOST
		methodChoice = 0;
		isAdaptive  = true;

	}
	if (tracingMethodComboB->currentIndex()==4){ //adaptive APP2
		methodChoice = 2;
		isAdaptive = true;

	}
	if (tracingMethodComboB->currentIndex()==5){ //adaptive Neutube
		methodChoice= 1;
		isAdaptive = true;

	}
	if (tracingMethodComboB->currentIndex()==6){ //automatic
		methodChoice= -1;
		isAdaptive = true;

	}

	if (tracingMethodComboB->currentIndex()==7){ //debugging mode
		methodChoice= 3;
		isAdaptive = false;

	}
	if (multiThreadTracingCB->isChecked()){
		int internalThreadNumber = traceThreadNumber;  // otherwise traceThreadNumber could be incremented during the checks below
		if (internalThreadNumber==0){
			emit callSATrace(tileFileString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							 this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);

			internalThreadNumber++;
			internalThreadNumber = internalThreadNumber%4;
			traceThreadNumber = internalThreadNumber;
			return;
		}else if (internalThreadNumber==1){
			emit callSATrace0(tileFileString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
			internalThreadNumber++;
			internalThreadNumber = internalThreadNumber%4;
			traceThreadNumber = internalThreadNumber;
			return;
		}else if (internalThreadNumber==2){
			emit callSATrace1(tileFileString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
			internalThreadNumber++;
			internalThreadNumber = internalThreadNumber%4;
			traceThreadNumber = internalThreadNumber;
			return;
		}else if (internalThreadNumber==3){
			emit callSATrace2(tileFileString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
							  this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
			internalThreadNumber++;
			internalThreadNumber = internalThreadNumber%4;
			traceThreadNumber = internalThreadNumber;
			return;
		}






	}else{
		emit callSATrace(tileFileString,overlap,this->findChild<QSpinBox*>("bkgSpinBox")->value(),
						 this->findChild<QCheckBox*>("interruptCB")->isChecked(),seedList,tileLocation,saveDir.absolutePath(),useGSDTCB->isChecked(),isSoma,isAdaptive,methodChoice, tileStatus);
	}


}


void S2UI::loadingDone(Image4DSimple *mip){


	emit eventSignal("finishedGridLoad");

	if (gridScanStatus>0){
		emit loadMIPSignal((double) loadScanNumber, mip, "ignore");
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
	// collect overview stack at lowest mag and high spatial resolution.
	// this is called at the same time that a signal is sent to the controller to setup for an overview
	qDebug()<<"overview pixel to scan pixel "<<overViewPixelToScanPixel;
	// so start my monitor to see when it's in ready state:
	overviewCycles = 0;
	status("start overview");
	emit eventSignal("startZStack");
	LocationSimple overviewGalvoLocation;
	overviewGalvoLocation.x = 0.;
	overviewGalvoLocation.y = 0.;
	overviewGalvoLocation.ev_pc1 = 512.0*overViewPixelToScanPixel;;
	overviewGalvoLocation.ev_pc2 = 512.0*overViewPixelToScanPixel;;
	LocationSimple overviewStageLocation;
	overviewStageLocation.x =  uiS2ParameterMap[5].getCurrentValue();
	overviewStageLocation.y =  uiS2ParameterMap[6].getCurrentValue();
	LocationSimple overviewPixelLocation;
	overviewPixelLocation.x = uiS2ParameterMap[5].getCurrentValue()*overViewPixelToScanPixel / overviewMicronsPerPixel;
	overviewPixelLocation.y = uiS2ParameterMap[6].getCurrentValue()*overViewPixelToScanPixel / overviewMicronsPerPixel;
	overviewPixelLocation.ev_pc1 = 512.0*overViewPixelToScanPixel;
	overviewPixelLocation.ev_pc2 = 512.0*overViewPixelToScanPixel;
	TileInfo overviewTileInfo = TileInfo(zoomPixelsProduct/overViewPixelToScanPixel);
	overviewTileInfo.setGalvoLocation(overviewGalvoLocation);
	overviewTileInfo.setStageLocation(overviewStageLocation);
	overviewTileInfo.setPixelLocation(overviewPixelLocation);
	allOverviewStageLocations.append(overviewTileInfo);
	QTimer::singleShot(10, this, SLOT(overviewHandler()));
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
		qDebug()<<"allOverviewStageLocations.last().getPixelLocation().x,y"<<allOverviewStageLocations.last().getPixelLocation().x<<","<<allOverviewStageLocations.last().getPixelLocation().y;
		qDebug()<<"overviewMicronsperPixl = "<<overviewMicronsPerPixel<<"  overviewpixeltoscanpixel "<<overViewPixelToScanPixel;

		roiGS->addRect((overviewMicronsPerPixel/overViewPixelToScanPixel)*( allOverviewStageLocations.last().getPixelLocation().x- allOverviewStageLocations.last().getPixelLocation().ev_pc1/2.0),
					   (overviewMicronsPerPixel/overViewPixelToScanPixel)*( allOverviewStageLocations.last().getPixelLocation().y- allOverviewStageLocations.last().getPixelLocation().ev_pc2/2.0),
					   (overviewMicronsPerPixel/overViewPixelToScanPixel)*allOverviewStageLocations.last().getPixelLocation().ev_pc1,
					   (overviewMicronsPerPixel/overViewPixelToScanPixel)*allOverviewStageLocations.last().getPixelLocation().ev_pc2,
					   QPen(Qt::magenta, 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));
		//   roiGS->addRect(allOverviewStageLocations.last().getStageLocation().x-roiXWEdit->text().toFloat()/2.0,allOverviewStageLocations.last().getStageLocation().y-roiXWEdit->text().toFloat()/2.0,roiXWEdit->text().toFloat(),roiYWEdit->text().toFloat(), QPen::QPen(Qt::black, 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));

		// set up 3-plane z stack here?
		waitingForFile = 0;
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
	float leftEdge = roiXEdit->text().toFloat() - roiXWEdit->text().toFloat()/2.0 +uiS2ParameterMap[5].getCurrentValue();
	float topEdge =  roiYEdit->text().toFloat() - roiYWEdit->text().toFloat()/2.0+ uiS2ParameterMap[6].getCurrentValue();


	if (smartScanStatus==1){
		QGraphicsTextItem* sequenceNumberText;

		sequenceNumberText = new QGraphicsTextItem;
		sequenceNumberText->setPos(leftEdge+10,topEdge);
		sequenceNumberText->setPlainText(QString::number(loadScanNumber));
		sequenceNumberText->setDefaultTextColor(Qt::green);
		sequenceNumberText->setZValue(100);
		roiGS->addItem(sequenceNumberText);
	}

}

QString S2UI::fixFileString(QString inputString){
	// take the filestring from the microscope and swap out the path to the directory for the local
	// path to the data
	if (!isLocal){
		//qDebug()<<inputString;
		QString toSplit = inputString;
		QStringList splitList =    toSplit.split("\\");
		QString nameOfFile = splitList.last();// need to pick off the directory above this too!
		QString dirName= QString("");
		if (splitList.length() >1){
			dirName = splitList.at(splitList.length()-2);

		}
		inputString= QDir(localDataDirectory.absolutePath().append(QDir::separator()).append(dirName)).absoluteFilePath(nameOfFile);
		//inputString = localDataDirectory.absoluteFilePath(nameOfFile);
		//inputString.replace("\\AIBSDATA","\\data").replace("\\","/");
	}
	return inputString;


}
void S2UI::updateFileString(QString inputString){
	//this means a new file has been created.. it can be late by up to 1 full cycle of s2parametermap updating
	// but it guarantees that the acquisition is done
	// a separate poller of updated filename could be much faster.
	// final version will require much more rigorous timing- it's not clear how we'll parse out
	// the streamed image data into files...
	machineSaveDir->setText(inputString);


	fileString = fixFileString(inputString);
	fileString.append("_Cycle00001_Ch2_000001.ome.tif");
	if ((QString::compare(fileString,lastFile, Qt::CaseInsensitive)!=0)&(waitingForFile>0)){
		emit eventSignal("finishedZStack");
		waitingForFile = 0;
		emit loadLatestSig(fileString);
	}
	lastFile = fileString;
	//qDebug()<<lastFile;
}

// need to correctly pick directories. 1.  show current directory in new config window.  2. in new config window also show current data directory and allow user to easily change.

// for faster version, add ulf's code to rip their raw file into a 1d format with color as last dimension.  rip on analysis machine with 10Gb connectivity to data and put directly in 1ddata format.  leave writing the same as before.

void S2UI::clearROIPlot(){
	roiGS->clear();
	roiRect = QRectF(-400, -400, 800, 800);
	// roiGS->addRect(roiRect,QPen::QPen(Qt::gray, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin), QBrush::QBrush(Qt::gray));
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



void S2UI::getCurrentParameters(){
	emit currentParameters(uiS2ParameterMap);
}



void S2UI::closeEvent(QCloseEvent *){
	myNotes->close();
	myPosMon.close();
	myController.close();
	myTargetTable->close();

}


void S2UI::resetToOverviewPBCB(){
	status("resetting to overview mode");
	// this could also reset smartscan parameters, markers, etc.
}


void S2UI::resetToScanPBCB(){
	// so start my monitor to see when it's in ready state:
	centerGalvosPB->click();
	updateCurrentZoom(tileSizeCB->currentIndex());



}


void S2UI::scanStatusHandler(){

	zoomStateOK = (qAbs( uiS2ParameterMap[12].getCurrentValue() - currentTileInfo.getTileZoom())<(float) 1)&&
			( qAbs((int) uiS2ParameterMap[10].getCurrentValue() - currentTileInfo.getTilePixelsX())< 2) &&
			(qAbs((int) uiS2ParameterMap[11].getCurrentValue() - currentTileInfo.getTilePixelsY())< 2);


	bool scanStatusTimedOut = scanStatusWaitCycles >200;

	if (scanStatusTimedOut){
		QString sString = currentTileInfo.getTileInfoString().join("\n");

		status(sString);
		status("scan status timeout!");
		return;
	}

	if (!zoomStateOK&&!scanStatusTimedOut){
		scanStatusWaitCycles++;
		if (scanStatusWaitCycles%20 ==0 ){
			qDebug()<< "scanStatus wait = "<<QString::number((scanStatusWaitCycles*50)/1000);
		}

		QTimer::singleShot(50, this, SLOT(scanStatusHandler()));


	}else{
		if (waitingToStartStack){
			emit eventSignal("startZStack");
			waitingForFile = 1;
			QTimer::singleShot(startZStackDelaySB->value(), &myController, SLOT(startZStack()));//
			//emit startZStackSig();
			waitingToStartStack = false;
			return;
		}
	}
}


void S2UI::collectZoomStack(){
	// collect zoom stack based on the coordinates of the Local 3D View  window.

	View3DControl *my3DControl;
	my3DControl = 0;
	LocationSimple newTarget;

	// find the latest window and get the 3dcontrol from that...
	QList<V3dR_MainWindow *  > viewerList;
	viewerList= cb->getListAll3DViewers();
	V3dR_MainWindow *localWin;
	int iWant = 0;
	if (!viewerList.isEmpty()){
		qDebug()<<"viewerList not empty";
		for (int i =0; i<viewerList.length(); i++){
			QString windowName = cb->getImageName(viewerList.at(i));
			qDebug()<<windowName;
			if (windowName.contains("Local 3D View ")) {
				localWin = viewerList.at(i);
				iWant = i;
			}
		}
		qDebug()<<"got local window at i = "<<iWant;
		my3DControl = cb->getView3DControl_Any3DViewer(viewerList[iWant]);
		qDebug()<<"x start = "<<my3DControl->getLocalStartPosX();
		qDebug()<<"x end = "<<my3DControl->getLocalEndPosX();
		qDebug()<<"y start = "<<my3DControl->getLocalStartPosY();
		qDebug()<<"y end = "<<my3DControl->getLocalEndPosY();
		qDebug()<<"window name "<< cb->getImageName(localWin);
		float xCenter = ( my3DControl->getLocalEndPosX()+ my3DControl->getLocalStartPosX())/2.0;
		float yCenter = ( my3DControl->getLocalEndPosY()+ my3DControl->getLocalStartPosY())/2.0;
		float xWidth = ( my3DControl->getLocalEndPosX()- my3DControl->getLocalStartPosX());
		float yWidth = ( my3DControl->getLocalEndPosY()- my3DControl->getLocalStartPosY());

		qDebug()<<"x = "<<xCenter<<" y = "<<yCenter<<" x width ="<<xWidth<<" y width = "<<yWidth;
		newTarget.x = (xCenter-256.0)*overViewPixelToScanPixel;// the scan origin is at the center of the overview image.
		newTarget.y  = (yCenter-256.0)*overViewPixelToScanPixel;

		int tileWidth = 0;
		if (xWidth>yWidth){
			tileWidth =xWidth;
		}else{
			tileWidth = yWidth;
		}
		newTarget.ev_pc1 = tileWidth * overViewPixelToScanPixel;
		newTarget.ev_pc2 = tileWidth * overViewPixelToScanPixel;

		//currentTileInfo.setPixels((int) tileWidth);
	}

	//QString sString =currentTileInfo.getTileInfoString().join(" _ ");
	//status("currentTileInfo : "+sString);
	//qDebug()<<sString;
	waitingToStartStack = true;
	TileInfo newTargetTI = TileInfo(zoomPixelsProduct);
	newTargetTI.setGalvoLocation(newTarget);
	moveToROI(newTargetTI);
	updateZoom(); // Bigtime race here!  I need a delayed/conditional move that waits until the zoom status is settled.


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
	QList<TileInfo> targets;
	for (int i =0; i<previewTargets.length();i++){
		LocationSimple newTargetGalvo;
		LocationSimple newTargetStage;
		LocationSimple newTargetPixels;
		TileInfo newTarget = TileInfo(zoomPixelsProduct);

		newTargetGalvo.x = (previewTargets.at(i).x-255.0)*overViewPixelToScanPixel;// the scan origin is at the center of the overview image.
		newTargetGalvo.y = (previewTargets.at(i).y-255.0)*overViewPixelToScanPixel;

		newTargetGalvo.ev_pc1  = uiS2ParameterMap[10].getCurrentValue();
		newTargetGalvo.ev_pc2  = uiS2ParameterMap[11].getCurrentValue();

		newTargetStage.x = allOverviewStageLocations.last().getStageLocation().x; // this is the right idea to use the latest overview, but depends on sequential overview imaging and target selection.
		newTargetStage.y = allOverviewStageLocations.last().getStageLocation().y;

		newTargetPixels =   newTargetGalvo;
		newTargetPixels.x = newTargetGalvo.x + newTargetStage.x/uiS2ParameterMap[8].getCurrentValue();
		newTargetPixels.y = newTargetGalvo.y + newTargetStage.y/uiS2ParameterMap[9].getCurrentValue();

		startCenter.x = 0.0+newTargetGalvo.x;
		startCenter.y = 0.0+newTargetGalvo.y;
		startCenter.ev_pc1 = uiS2ParameterMap[10].getCurrentValue();
		startCenter.ev_pc2 = uiS2ParameterMap[11].getCurrentValue();

		newTarget.setGalvoLocation(newTargetGalvo);
		newTarget.setStageLocation(newTargetStage);
		newTarget.setPixelLocation(newTargetPixels);
		targets.append(newTarget);
		LandmarkList startList;
		startList.append(startCenter);
		startingROIList.append(startList);
	}
	allTargetLocations = targets;
	emit updateTable(targets,startingROIList);
}




void S2UI::loadMIP(double imageNumber, Image4DSimple* mip, QString tileSaveString){

	qDebug()<<"loadMIP";

	if (scanList.isEmpty()) return;

	QTextStream summaryTextStream;

	if (imageNumber==-1){
		summaryTextStream.setDevice(&summaryTextFile);
		summaryTextStream<<"@@@@@@@@@@@;@@@@@@@@@@@"<<"\n";
		summaryTextStream<<"tile number ;"<<imageNumber<<"\n";
		summaryTextStream<<"double-checked tile "<<tileSaveString<<" at "<<QDateTime::currentDateTime().toString("yyyy_MM_dd_ddd_hh_mm_ss_zzz")<<"\n";
		return;}




	int v = scanList[imageNumber].setTimeStamp(QDateTime::currentDateTime()); // fourth timestamp when analysis is done.
	scanList[imageNumber].setScanIndex(imageNumber);
	//    myScanData->addNewTile(scanList.at(imageNumber)); // this tileInfo is the final data for this tile, so const is ok.



	myScanMonitor->addNewTile(scanList.at(imageNumber));
	QDir s2ScanDir = QFileInfo(scanList.at(imageNumber).getFileString()).absoluteDir();
	QList<LocationSimple> pixelLocations;
	QList<LocationSimple> galvoLocations;
	QList<long> scanNumbers;
	for (int i = 0; i<myScanMonitor->allScanData.last().allTiles.length(); i++){
		pixelLocations.append(myScanMonitor->allScanData.last().allTiles.at(i).getPixelLocation());
		galvoLocations.append(myScanMonitor->allScanData.last().allTiles.at(i).getGalvoLocation());
		scanNumbers.append(myScanMonitor->allScanData.last().allTiles.at(i).getScanIndex());
	}
	QImage s2ScanImage;
	myScanMonitor->allScanData.last().boundingBoxX=0;
	myScanMonitor->allScanData.last().boundingBoxY=0;
	long sizex=0;
	long sizey=0;
	myScanMonitor->allScanData.last().imagedArea=0;
	if (myScanMonitor->allScanData.last().allTiles.length()>0) {

		float minx = 1000000.0;
		float maxx = -1000000.0;
		float miny = 1000000.0;
		float maxy = -1000000.0;

		QList<LocationSimple> imageLocations;
		myScanMonitor->allScanData.last().totalTileArea=0;
		for (int i = 0; i<myScanMonitor->allScanData.last().allTiles.length(); i++){
			LocationSimple locationi = myScanMonitor->allScanData.last().allTiles.at(i).getPixelLocation();
			locationi.ev_pc1 = myScanMonitor->allScanData.last().allTiles.at(i).getGalvoLocation().ev_pc1;
			locationi.ev_pc2 = myScanMonitor->allScanData.last().allTiles.at(i).getGalvoLocation().ev_pc2;
			if (locationi.x<minx) minx = locationi.x;
			if (locationi.x+locationi.ev_pc1>maxx) maxx = locationi.x+locationi.ev_pc1;
			if (locationi.y<miny) miny = locationi.y;
			if (locationi.y+ locationi.ev_pc2 >maxy) maxy = locationi.y+locationi.ev_pc1;

			sizex = (long)  (maxx-minx+1.);
			sizey = (long)  (maxy-miny+1.);
			myScanMonitor->allScanData.last().totalTileArea = myScanMonitor->allScanData.last().totalTileArea+ (float) locationi.ev_pc2 * (float) locationi.ev_pc1;
		}



		for (int i = 0; i<myScanMonitor->allScanData.last().allTiles.length(); i++){
			LocationSimple locationi = myScanMonitor->allScanData.last().allTiles.at(i).getPixelLocation();
			locationi.x = locationi.x-minx;
			locationi.y = locationi.y-miny;
			imageLocations.append(locationi);
		}
		s2ScanImage  = QImage(sizex,sizey,QImage::Format_RGB888);
		s2ScanImage.fill(0);

		for (int i = 0; i<imageLocations.length(); i++){
			for (long k = imageLocations.at(i).x; k< (imageLocations.at(i).x+imageLocations.at(i).ev_pc1); k++){
				for (long j = imageLocations.at(i).y; j< (imageLocations.at(i).y+imageLocations.at(i).ev_pc2); j++){
					QRgb pixelValue =  s2ScanImage.pixel(k,j);
					pixelValue = ((uint) pixelValue ) + (uint) 1;
					s2ScanImage.setPixel(k,j,pixelValue);



				}
			}
		}



		for (long imi=0; imi<sizex; imi++){
			for (long imj=0; imj<sizey; imj++){
				int imijpixel =  qBlue( s2ScanImage.pixel(imi, imj));

				if ( imijpixel > 0)  myScanMonitor->allScanData.last().imagedArea++;
			}
		}

		myScanMonitor->allScanData.last().boundingBoxX=(float) sizex;
		myScanMonitor->allScanData.last().boundingBoxY=(float) sizey;
		// all because the stupid qpainter operations involving QFont have to happen in the GUI thread...


		QPainter testPainter;
		testPainter.begin(&s2ScanImage);
		testPainter.setPen(Qt::yellow);
		for (int i = 0; i<imageLocations.length(); i++){
			testPainter.drawText( QRectF(imageLocations.at(i).x+30, imageLocations.at(i).y+10, imageLocations.at(i).ev_pc1, imageLocations.at(i).ev_pc2), QString::number(scanNumbers.at(i)));
		}
		s2ScanImage.save(s2ScanDir.absolutePath().append(QDir::separator()).append("S2Image.tif"));



	}



	// append this final tile info to our summary file

	// possibly include some running totals: sum of tile area, scanned area, boundingbox size, total time, total imaging time.
	summaryTextStream.setDevice(&summaryTextFile);
	summaryTextStream<<"@@@@@@@@@@@;@@@@@@@@@@@"<<"\n";
	summaryTextStream<<"tile number ;"<<imageNumber<<"\n";
	summaryTextStream<<"tile mode ;"<<uiS2ParameterMap[0].getCurrentString()<<"\n";
	summaryTextStream<<"tile zoom ;"<<uiS2ParameterMap[12].getCurrentValue()<<"\n";
	summaryTextStream<<"microns per pixel ;"<<uiS2ParameterMap[8].getCurrentValue()<<"\n";
	summaryTextStream<<"tracing algorithm ;"<<tracingMethodComboB->currentText()<<"\n";
	summaryTextStream<<"tile filename ;"<<tileSaveString<<"\n";
	QStringList testOutput =     scanList.value(imageNumber).getTimeStrings();

	for (int ii=0; ii<testOutput.length(); ii++){
		qDebug()<<testOutput.at(ii);
		summaryTextStream<<"tile event "<<ii<<" ;"<<testOutput.at(ii)<<"\n";

	}
	QList<float> testElapsed = scanList.value(imageNumber).getElapsedTimes();
	for (int jj =0;jj<testElapsed.length(); jj++){
		qDebug()<<"elapsed time "<<jj<<" = "<<QString::number(testElapsed.at(jj));
		summaryTextStream<<"elapsed time "<<jj<<" ;"<< QString::number(testElapsed.at(jj))<<"\n";
		if (jj==2) totalImagingTime=totalImagingTime+testElapsed.at(jj);
		if (jj==3) totalAnalysisTime=totalAnalysisTime+testElapsed.at(jj);
	}

	QList<qint64> msTileStartEvents;
	msTileStartEvents.clear();
	for (int kk=0; kk<scanList.length(); kk++){
		msTileStartEvents.append(scanList.at(kk).getTileTimes().at(1).toMSecsSinceEpoch());
	}




	summaryTextStream<<"@@  running totals ;    @@"<<"\n";
	//  some of these (total imaging time and total analysis time) are based on the tiles that have made it here, whereas others (anything from myScanMonitor) is based on
	// data submitted to that object immediately before imaging.  bottom line is that the last entry of these totals should be spot-on but other entries are just to give a sense
	// of what's happening.
	summaryTextStream<<"imaged area ;"<< myScanMonitor->allScanData.last().imagedArea<<"\n";
	summaryTextStream<<"sum of tile areas ;"<<myScanMonitor->allScanData.last().totalTileArea<<"\n"  ;
	summaryTextStream<<"boundingBoxX ;"<<myScanMonitor->allScanData.last().boundingBoxX<<"\n";
	summaryTextStream<<"boundingBoxY ;"<<myScanMonitor->allScanData.last().boundingBoxY<<"\n";
	summaryTextStream<<"boundingBox Area ;"<<myScanMonitor->allScanData.last().boundingBoxX*myScanMonitor->allScanData.last().boundingBoxY<<"\n";
	summaryTextStream<<"tiles in queue ;"  <<  allROILocations->length()<<"\n";
	summaryTextStream<<"tiles imaged ;"  <<  scanList.length()<<"\n";
	summaryTextStream<<"total imaging time ;"<<totalImagingTime<<"\n";
	summaryTextStream<<"total analysis time ;"<<totalAnalysisTime<<"\n";
	summaryTextStream<<"ms since start ;" <<  QDateTime::currentDateTime().toMSecsSinceEpoch()-scanStartTime.toMSecsSinceEpoch()<<"\n";

	// get the average difference by finding the total time delay between first and last  signals
	summaryTextStream<<"average ms between tile starts ;"<<((float) (msTileStartEvents.last()-msTileStartEvents.first()) )/((float) scanList.length()-1)<<"\n";


	if ((!mip==0)||(mip->getTotalBytes()==0)){
		scaleintensity(mip,0,0,8000,double(0),double(255));
		scale_img_and_convert28bit(mip, 0, 255) ;
		QImage myMIP;
		int x = mip->getXDim();
		int y = mip->getYDim();
		V3DLONG total  =0;
		Image4DProxy<Image4DSimple> mipProx(mip);
		myMIP = QImage(x, y, QImage::Format_RGB888);
		for (V3DLONG i=0; i<x; i++){
			for (V3DLONG j=0; j<y;j++){
				myMIP.setPixel(i,j,mipProx.value8bit_at(i,j,0,0)+mipProx.value8bit_at(i,j,0,0)*256);
				total++;
			}
		}


		QGraphicsPixmapItem* mipPixmap = new QGraphicsPixmapItem(QPixmap::fromImage(myMIP));
		float xPixMicrons = scanList.value(imageNumber).getPixelLocation().x*uiS2ParameterMap[8].getCurrentValue();// scanList.value(imageNumber).getGalvoLocation().x*uiS2ParameterMap[8].getCurrentValue()+scanList.value(imageNumber).getStageLocation().x;//
		float yPixMicrons  = scanList.value(imageNumber).getPixelLocation().y*uiS2ParameterMap[9].getCurrentValue();// scanList.value(imageNumber).getGalvoLocation().y*uiS2ParameterMap[9].getCurrentValue()+scanList.value(imageNumber).getStageLocation().y;//
		mipPixmap->setScale(uiS2ParameterMap[8].getCurrentValue());
		mipPixmap->setPos(xPixMicrons,
						  yPixMicrons);
		mipPixmap->setOffset(-x/2.0,-y/2.0 );

		//    mipPixmap->setPos((xPix-((float) x )/2.0)*uiS2ParameterMap[8].getCurrentValue(),
		//          (yPix-((float) x )/2.0)*uiS2ParameterMap[9].getCurrentValue());
		//mipPixmap->acceptHoverEvents();
		//mipPixmap->setToolTip(scanList.value(imageNumber).getFileString());



		roiGS->addItem(mipPixmap);
		QGraphicsTextItem* sequenceNumberText;

		sequenceNumberText = new QGraphicsTextItem;
		sequenceNumberText->setPos(xPixMicrons-uiS2ParameterMap[8].getCurrentValue()*(x/2.0),yPixMicrons-uiS2ParameterMap[8].getCurrentValue()*(y/2.0) );
		sequenceNumberText->setPlainText(QString::number(imageNumber));
		sequenceNumberText->setTextWidth(100);
		sequenceNumberText->setDefaultTextColor(makeQColorFromIndex(10,colorIndex));
		sequenceNumberText->setZValue(1000);
		sequenceNumberText->setScale(0.8);
		roiGS->addItem(sequenceNumberText);


	}else{
		qDebug()<<"not displaying MIP of area already scanned";
	}
}


void S2UI::processingStarted(){
	numProcessing++;
	analysisRunning->setText(QString::number(numProcessing));
}

void S2UI::processingFinished(){
	numProcessing--;
	analysisRunning->setText(QString::number(numProcessing));
}




void S2UI::finalizeZoom(){
	qDebug()<<"setting up stack in finalizeZoom...";
	emit stackSetupSig(zStepSize ,currentTileInfo.getTileZoom(), currentTileInfo.getTilePixelsX(), currentTileInfo.getTilePixelsY() );
	qDebug()<<"emitted signal to set up stack in finalizeZoom...";

	zoomStateOK = false;
	scanStatusWaitCycles = 0;
	qDebug()<<"firing singleshot...";

	QTimer::singleShot(50,this,SLOT(scanStatusHandler()));
	// status("tileInfo resonantOK: "+currentTileInfo.getTileInfoString().at(5));
	//  qDebug()<<currentTileInfo.getTileInfoString().at(3);
}






// =================================  UPDATERS
// this is both GUI updaters and content/parameter updaters that interact with various handlers



void S2UI::updateOverlap(int value){
	overlap = 0.01* ((float) value);

}

void S2UI::updateZoom(){

	if (!posMonStatus){
		status("zoom update failed- posMon inactive");
		return;
	}
	// update current mode only if necessary
	status("resonantOK: "+QString::number(currentTileInfo.resOK));
	status("current mode? "+uiS2ParameterMap[0].getCurrentString());
	// disable mode changes for now- requires more overhead (turning on pmts, pockels cell, etc)
	//    if (uiS2ParameterMap[0].getCurrentString().contains("esonant") == !currentTileInfo.resOK){
	//        status("changing active mode");
	//        if (currentTileInfo.resOK){ myController.cleanAndSend("-sts activeMode ResonantGalvo");
	//        }else{
	//            myController.cleanAndSend("-sts activeMode Galvo");
	//        }
	//    }
	activeModeChecks = 0;
	finalizeZoom();

}
void S2UI::updateZoomPixelsProduct(int ignore){

	zoomPixelsProduct = zoomSpinBox->value()*pixelsSpinBox->value();
	zoomPixelsProductLabel->setText(QString("zoom*pixels = ").append(QString::number(zoomPixelsProduct)).append("   (default for 16x objective: 3328, for 25x: 2048)"));
	QTimer::singleShot(10, this, SLOT(initializeROISizes()));
	overViewPixelToScanPixel = ((float) zoomSpinBox->value())*((float) pixelsSpinBox->value()/512.0);

}
void S2UI::updateCurrentZoom(int currentIndex){

	currentTileInfo = tileSizeChoices->at(currentIndex);
	updateZoom() ;
}
void S2UI::updateZoomHandler(){

}

void S2UI::updateLipoFactor(int ignore){
	emit updateLipoFactorInSA(((float) lipoFactorSlider->value() )*2.0/100.0);
	lipoFactorSliderLabel->setText(QString("alpha = ").append(QString::number(((float) lipoFactorSlider->value() )*2.0/100.0)));
}


void S2UI::updateRedThreshold(int ignore){
	emit updateRedThreshInSA((int) redThresholdSlider->value()*80);
	redThresholdSliderLabel->setText(QString("red threshold = ").append(QString::number(( redThresholdSlider->value() )*8000/100)));

}





void S2UI::updateMinMaxBlock(int ignore){
	int maxBlock = maxBlockSizeSB->value();
	int minBlock = minBlockSizeSB->value();
	if (maxBlock < minBlock){ maxBlock = minBlock;}
	emit updateMinMaxBlockSizes(minBlock,maxBlock );
	minBlockSizeSBLabel->setText(QString("min block= ").append(QString::number(minBlock)).append(" pixels"));
	maxBlockSizeSBLabel->setText(QString("max block= ").append(QString::number(maxBlock)).append(" pixels"));
}


void S2UI::updateSearchRadiusCallback(int ignore){
	emit updateSearchRadius((double) searchPixelRadiusSB->value());
}



void S2UI::updateZStepSize(int ignore){
	zStepSize = ((float) stackZStepSizeSlider->value())/10.0;
	stackZStepSizeLabel->setText(QString("z step size = ").append(QString::number(zStepSize)).append(" um"));

}






// =================================
// +++++++++++++++++++++++++++++++






QColor S2UI::makeQColorFromIndex(int maxIndex, int index){
	return    QColor((index%maxIndex)*255/maxIndex,qAbs((256-(index%maxIndex)*255/maxIndex))%256,qAbs((index%(maxIndex/2))*512/maxIndex)%256);
}










// new or move to config parameters:
// min and maximum adaptive tile size
// pixel-zoom product stuff
//
//  configure multi-tile scan of bounding box...?

// configure single-tile scan of bounding box for galvo scans.

// start by centering scan
