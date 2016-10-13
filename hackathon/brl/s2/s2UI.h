

#ifndef S2UI_H
#define S2UI_H

#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTcpSocket>
#include <QDir>
#include <QCheckBox>
#include <QTabWidget>
#include <QDir>
#include <v3d_interface.h>
#include "s2Controller.h"
#include "s2plot.h"
#include "stackAnalyzer.h"
#include "noteTaker.h"
#include "targetList.h"
#include "eventLogger.h"
#include "tileInfo.h"

QT_BEGIN_NAMESPACE
class QWidget;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

//! [0]
class S2UI : public QDialog
{
    Q_OBJECT

public:
    S2UI(V3DPluginCallback2 &callback, QWidget *parent = 0 );
    QLineEdit *s2LineEdit;

    StackAnalyzer *myStackAnalyzer;
    StackAnalyzer *myStackAnalyzer0;
    StackAnalyzer *myStackAnalyzer1;
    StackAnalyzer *myStackAnalyzer2;

    EventLogger* myEventLogger;
public slots:
    void pmStatusHandler(bool pmStatus);
    void handleNewLocation(QList<LandmarkList> newTipsList, LandmarkList newlandmarks, Image4DSimple *mip, double scanIndex);
    void loadLatest();
    void collectOverview();
    void getCurrentParameters();
    void combinedSmartScan(QString);
    void monitorLiveFile();
    void updateLiveFile();
    void startLiveFile();
    void tTrace();
signals:
    void startPM();
    void stopPM();
    void callSALoad(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool);
    void callSALoadSubtractive(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool,int);
    void callSALoadAda(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool);
    void callSALoadAdaSubtractive(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool,int);
    void callSAGridLoad(QString, LocationSimple, QString);
    void newImageData(Image4DSimple);
    void moveToNext(LocationSimple);
    void moveToNextWithStage(LocationSimple, float, float);
	void noteStatus(QString);
    void processSmartScanSig(QString);
    void currentParameters(QMap<int, S2Parameter> currentParameterMap);
    void updateTable(QList<TileInfo> allTargetLocations,QList<LandmarkList> allScanLocations);
    void eventSignal(QString);

    void updateLipoFactorInSA(float);
    void updateRedThreshInSA(int);
    void updateLipoModeInSA(int);

    void updateMinMaxBlockSizes(int, int);
    void updateSearchRadius(double);
    void stackSetupSig(float, float, int, int);
    void startZStackSig();
    void callSATrace(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int);
    void callSATrace0(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int);
    void callSATrace1(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int);
    void callSATrace2(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int);


private slots:
    void runBoundingBox();
    void startS2();
    void startScan();
    void loadScan();
    void displayScan();
    void posMonButtonClicked();
    void updateS2Data(  QMap<int, S2Parameter> currentParameterMap);
    void updateString(QString broadcastedString);
    void updateFileString(QString inputString);
    void startingSmartScan();
    void startingZStack();
    void updateROIPlot(QString ignore);
    void updateGVZoom(int sliderValue);
    void s2ROIMonitor();
    void moveToROI(const TileInfo nextROI);
    void moveToROIWithStage(const TileInfo nextROI);
    void clearROIPlot();
    void smartScanHandler();
    QString getFileString();
    void loadScanFromFile(QString fileString);
    void toLoad();
	void status(QString statString);
	void updateLocalRemote(bool state);
    void runSAStuffClicked();
    void updateOverlap(int value);
    void overviewHandler();
    void scanStatusHandler();
    void loadForSA();
    void resetToOverviewPBCB();
    void resetToScanPBCB();
    void pickTargets();
    void startAllTargets();
    void handleAllTargets();
    void loadMIP(double imageNumber, Image4DSimple* mip);
    void loadingDone(Image4DSimple* mip);
    void processingStarted();
    void processingFinished();
    void updateZoom();
    void updateCurrentZoom(int currentIndex);
    void finalizeZoom();
    void activeModeChecker();
    void updateZoomHandler();
    void resetDirectory();
    void resetDataDir();
    void collectZoomStack();
    void updateZoomPixelsProduct(int ignore);
    void initializeROISizes();

    void updateLipoFactor(int ignore);
    void updateRedThreshold(int ignore);
    void updateMinMaxBlock(int ignore);
    void updateZStepSize(int ignore);
    void updateSearchRadiusCallback(int ignore);

    void tryXYMove();


private:
    V3DPluginCallback2 * cb;
    QThread *workerThread;
    QThread *workerThread0;
    QThread *workerThread1;
    QThread *workerThread2;

    QString versionString;


 // Layout and buttons

    QTabWidget * lhTabs;
    QTabWidget * rhTabs;


    QGridLayout *mainLayout;
    QGridLayout * gl;

    QDialogButtonBox *buttonBox1;

    QGroupBox *parameterBox;
    QVBoxLayout *vbox;
    QGroupBox *roiGroupBox;
    QGroupBox *createROIMonitor();
    QRectF roiRect;
    QGraphicsScene * roiGS;
    QGraphicsView * roiGV;
    QGraphicsRectItem *newRect;
    QSlider *zoomSlider;
    QTransform originalTransform;


    QGroupBox *createROIControls();
    QGroupBox *createS2Monitors();
    QGroupBox *createTracingParameters();
    QGroupBox *createConfigPanel();



    QSpinBox * minBlockSizeSB;
    QLabel * minBlockSizeSBLabel;
    QSpinBox * maxBlockSizeSB;
    QLabel * maxBlockSizeSBLabel;

    QLabel * analysisRunning;

    QSlider * redThresholdSlider;
    QLabel * redThresholdSliderLabel;

    QSlider * lipoFactorSlider;
    QLabel * lipoFactorSliderLabel;

    QComboBox * chooseLipoMethod;
    QLabel * chooseLipoMethodLabel;


    QSlider * stackZStepSizeSlider;
    QLabel * stackZStepSizeLabel;


    QCheckBox * stageOnlyCB;
    QLabel * stageOnlyCBLabel;

    QCheckBox * multiThreadTracingCB;
    QLabel * multiThreadTracingCBLabel;

    QCheckBox * sendThemAllCB;
    QLabel * sendThemAllCBLabel;

    QSpinBox * searchPixelRadiusSB;
    QLabel * searchPixelRadiusSBLabel;

    void createButtonBox1();
	
    void createTargetList();

    QPushButton *resetDirPB;
    QPushButton *setLocalPathToData;
    QCheckBox *localRemoteCB;
    QPushButton *runAllTargetsPB;
    QCheckBox *useGSDTCB;
    QCheckBox *runContinuousCB;
    QCheckBox *gridScanCB;
    QSpinBox *gridSizeSB;

    QComboBox *tileSizeCB;

    QList<TileInfo> *tileSizeChoices;

    QComboBox *tracingMethodComboB;
    QComboBox *channelChoiceComboB;
    QCheckBox *addBoundingBoxScan;
    QLabel *addBoundingBoxScanLabel;

    QLabel *s2Label;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QPushButton  *startZStackPushButton;
    QPushButton * resetToOverviewPB;
    QPushButton * resetToScanPB;
    QPushButton * pickTargetsPB;

    QPushButton * collectZoomStackPB;

    QPushButton *collectOverviewPushButton;
    QPushButton *loadScanPushButton;
    QPushButton *startPosMonButton;
    QPushButton *startSmartScanPB;
    QPushButton *centerGalvosPB ;
    QPushButton * startStackAnalyzerPB;
    QPushButton *roiClearPB;
    QPushButton *runSAStuff;
    QLineEdit *roiXEdit;
    QLineEdit *roiYEdit;
    QLineEdit *roiZEdit;
    QLineEdit *roiXWEdit ;
    QLineEdit *roiYWEdit ;
    QLineEdit *roiZWEdit;
    QSpinBox *overlapSpinBox;
    QLabel *overlapSBLabel;
    QSpinBox *zoomSpinBox;
    QSpinBox *pixelsSpinBox;
    QLabel *zoomSpinBoxLabel;
    QLabel *pixelsSpinBoxLabel;
    QLabel * zoomPixelsProductLabel;

    QLabel *machineSaveDir;
    QLabel *localDataDir;
    QLabel *machineSaveDirLabel;
    QLabel *localDataDirLabel;
    QDir localDataDirectory;

    QLabel * liveFileStringLabel;
    QLabel * liveFileString;
    QPushButton * setLiveFilePath;

    QSpinBox * startZStackDelaySB;
    QLabel * startZStackDelayLabel;
    QFileInfo *liveFile;
    QFileInfo *liveFileStatus;

    QPushButton * tTracePB;


    QPushButton *tryStageMove;
    QLineEdit *tryStageXEdit;
    QLineEdit *tryStageYEdit;




// Target and ROI table widget

    TargetList* myTargetTable;
    int targetIndex;
    int colorIndex;


// External S2 controllers

    S2Controller myController;
    S2Controller myPosMon;


// gui hookup

    void hookUpSignalsAndSlots();
    QDir getSaveDirectory();

//  internal methods and important parameters
    void checkParameters(QMap<int, S2Parameter> currentParameterMap);
    bool isDuplicateROI(TileInfo inputTileInfo);
    void closeEvent(QCloseEvent *event);

    QList<TileInfo> allOverviewStageLocations;

    QList<TileInfo> allTargetLocations;
    QList<TileInfo> *allROILocations;


    QList<LandmarkList> allScanLocations;



    QList<LandmarkList> *allTipsList;
    QList<TileInfo> scanList;

    TileInfo currentTileInfo;

    QList<LandmarkList> tipList;


    QMap<int, S2Parameter> uiS2ParameterMap;

    QString fileString ;
    QString lastFile;
    bool posMonStatus;
    int waitingForFile;//  0 => not waiting.  1 => waiting for image.
    bool waitingForLast;
    bool waitingForOverview;
	bool isLocal;
    bool liveFileRunning;
    bool haventRunBoundingBox;
    QDateTime liveFileModified;

    int smartScanStatus;
    int gridScanStatus;
    int methodChoice;
    double scanNumber;
    double loadScanNumber;
    int resultNumber;
    int allTargetStatus;
    float overViewPixelToScanPixel;
    float overviewMicronsPerPixel;
    float scanVoltageConversion;
    float zoomPixelsProduct;
    float zStepSize;


    int traceThreadNumber;

    QString fixFileString(QString inputString);


    Image4DSimple*  total4DImage;

    NoteTaker *myNotes;

    QDir saveDir;
    QDir sessionDir;
    QDir currentDirectory;
    QFile saveTextFile;
    QTextStream outputStream;
    QString scanDataFileString;
    QString eventLogString;
    float overlap;
    int overviewCycles;
    int scanStatusWaitCycles;
    int activeModeChecks;
    bool havePreview;
    bool resetDir;
    bool zoomStateOK;
    bool waitingToStartStack;

    int numProcessing;


//   vaa3d variables
    v3dhandle previewWindow;
    v3dhandle liveFileWindow;
};
//! [0]

#endif
