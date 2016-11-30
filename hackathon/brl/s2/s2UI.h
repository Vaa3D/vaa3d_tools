

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
#include "stackAnalyzer.h"
#include "noteTaker.h"
#include "targetList.h"
#include "eventLogger.h"
#include "tileInfo.h"
#include "s2monitor.h"
#include <QMutex>
QT_BEGIN_NAMESPACE
class QWidget;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

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
    TileInfoMonitor* myTileInfoMonitor;
public slots:
    /*! \brief update posMon status
     *
     * handles if posMon monitor is running
     */
    void pmStatusHandler(bool pmStatus);
    /*! \brief handle new tile locations and tips coming from StackAnalyzer
     *
     * asynchronously processes output of StackAnalyzer, including MIP image data, tip locations, etc.
     * QMutex inside here is intended to prevent conflicts from multiple threads converging
     * also the site of critical flow control for un-imaged (or already traced) tiles that are needed
     * for linking of current structures with future (or past) tiles.
     */
    void handleNewLocation(QList<LandmarkList> newTipsList, LandmarkList newlandmarks, Image4DSimple *mip, double scanIndex,QString tileSaveString, int tileStatus);
    /*! \brief load image stack data from series of .tif files at inputString
     *
     * this is a rig-specific command to read in the .tif image series output of PrairieView.
     * future versions will
     */
    void loadLatest(QString inputString);
    void collectOverview();
    void getCurrentParameters();
    /*! \brief renders final .swc file from stackAnalyzer
     *
     * not always particularly helpful... currently only coming from stackAnalyzer, which is not really
     * where the 'final' status is determined.
     */
    void combinedSmartScan(QString);

signals:
    void startPM();
    void stopPM();
    void callSALoad(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool);
    void callSALoadSubtractive(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool,int);
    void callSALoadAda(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool);
    void callSALoadAdaSubtractive(QString,float,int,bool,LandmarkList, LocationSimple, QString, bool,bool,int);
    void callSAGridLoad(QString, LocationSimple, QString);

    /*! \brief external signal to S2Controller to move to new galvo location
     */
    void moveToNext(LocationSimple);
    /*! \brief external signal to S2Controller to move to new stage location
     */
    void moveToNextWithStage(LocationSimple, float, float);

    void noteStatus(QString);
    /*! \brief send scanData.txt filename as string to StackAnalyzer
     */
    void processSmartScanSig(QString);
    void currentParameters(QMap<int, S2Parameter> currentParameterMap);
    void updateTable(QList<TileInfo> allTargetLocations,QList<LandmarkList> allScanLocations);
    void eventSignal(QString);

    void updateLipoFactorInSA(float);
    void updateRedThreshInSA(int);
    void updateLipoModeInSA(int);
    void updateMinMaxBlockSizes(int, int);
    void updateSearchRadius(double);


    /*! \brief  signal to S2Controller to set up acquisition
     *
     * critical, rig-dependent signal here would need to be routed to
     * a new controller for an alternative hardware implementation
     */
    void stackSetupSig(float, float, int, int);

    /*! \brief trigger to S2Controller to start stack
     *
     * currently delayed by user-defined period to avoid conflicts with commands to PV.
     * needs optimization or more event-driven approach.
     */
    void startZStackSig();


    /*! \brief call StackAnalyzer::startTracing() method in a particular thread [ , 0, 1, 2]
     *
     * currently dedicated and hardcoded QThreads for each StackAnalyzer to get this off the ground. future version should
     * use the threadpool but there is a lot of setup/update info that would be needed in the constructor
     * to allow for a simple run() method or similar. Instead the current version updates each StackAnalyzer continuously from the GUI
     * as needed and sends these signals (with lots of critical info) when it's time to analyze a new tile.
     */
    void callSATrace(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int, int);
    void callSATrace0(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int, int);
    void callSATrace1(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int, int);
    void callSATrace2(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int, int);

    void loadMIPSignal(double imageNumber, Image4DSimple* mip, QString tileSaveString);

    /*! \brief send tile and tip info to TileInfoMonitor to wait until tile tracing is finished
     *
     * critical component of re-tracing and pre-tracing flow control.
     */
    void waitForDuplicate(TileInfo, LandmarkList, int , int, int, QString);
    void loadLatestSig(QString);
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
    void loadMIP(double imageNumber, Image4DSimple* mip, QString tileSaveString);
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

/*! \brief internal signal that subsequently sends tile info to StackAnalyzer if a tile has already been imaged
 *
 * this is similar to S2UI::loadLatest(QString inputString) but designed to handle tiles that have already been imaged
 * so the .swc file for the tile should already exist. also passes the correct X and Y coordinates (i.e. the coordinates that match the filename
 * and the scanData.txt file)
 */
    void loadDuplicateTile(TileInfo duplicateTile, LandmarkList seedList, int tileStatus, int correctX, int correctY);


private:
    V3DPluginCallback2 * cb;
    QThread *workerThread;
    QThread *workerThread0;
    QThread *workerThread1;
    QThread *workerThread2;

    QThread *swcWatchThread;

    QString versionString;


    S2ScanData * myScanData;
    S2Monitor * myScanMonitor;

    QMutex myMutex;
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
    QLabel * tileNotes;

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



    QSpinBox * startZStackDelaySB;
    QLabel * startZStackDelayLabel;




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

    QString fixFileString(QString inputString);

    QColor makeQColorFromIndex(int maxIndex, int index);



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
    bool haventRunBoundingBox;

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

    Image4DSimple*  total4DImage;

    NoteTaker *myNotes;

    QDir saveDir;
    QDir sessionDir;
    QDir currentDirectory;
    QFile saveTextFile;
    QFile summaryTextFile;
    QTextStream outputStream;
    QString scanDataFileString;
    QString eventLogString;
    QDateTime scanStartTime;
    float totalImagingTime;
    float totalAnalysisTime;
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
};
//! [0]

#endif
