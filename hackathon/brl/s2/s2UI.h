

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
    /*! \brief call StackAnalyzer::startTracing() method in a particular thread [ , 0, 1, 2]
     *
     * currently dedicated and hardcoded QThreads for each StackAnalyzer to get this off the ground. future version should
     * use the threadpool but there is a lot of setup/update info that would be needed in the constructor
     * to allow for a simple run() method or similar. Instead the current version updates each StackAnalyzer continuously from the GUI
     * as needed and sends these signals (with lots of critical info) when it's time to analyze a new tile.
     */
    void callSATrace0(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int, int);
    /*! \brief call StackAnalyzer::startTracing() method in a particular thread [ , 0, 1, 2]
     *
     * currently dedicated and hardcoded QThreads for each StackAnalyzer to get this off the ground. future version should
     * use the threadpool but there is a lot of setup/update info that would be needed in the constructor
     * to allow for a simple run() method or similar. Instead the current version updates each StackAnalyzer continuously from the GUI
     * as needed and sends these signals (with lots of critical info) when it's time to analyze a new tile.
     */
    void callSATrace1(QString,float,int,bool,LandmarkList,LocationSimple,QString,bool,bool,bool,int, int);
    /*! \brief call StackAnalyzer::startTracing() method in a particular thread [ , 0, 1, 2]
     *
     * currently dedicated and hardcoded QThreads for each StackAnalyzer to get this off the ground. future version should
     * use the threadpool but there is a lot of setup/update info that would be needed in the constructor
     * to allow for a simple run() method or similar. Instead the current version updates each StackAnalyzer continuously from the GUI
     * as needed and sends these signals (with lots of critical info) when it's time to analyze a new tile.
     */
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
    /*! \brief update the current Field of View on the ROI plot
     */
    void updateROIPlot(QString ignore);
    /*! \brief update the zoom on the ROI plot using the slider
     */
    void updateGVZoom(int sliderValue);
    /*! \brief monitors allROILocations for continuous imaging in S2 scan
     *
     *  critcal for s2 scan!  This checks allROILocations every 10 ms and if
     * there are tiles in allROILocations, and S2UI is not already waiting for a tile, the
     * first tile location (and associated tips) are removed from allROILocations (and allTipsList),
     * and S2UI::moveToROI() is called.  The tile and tip data is appended to the scanList and  tipList, and
     * the actual stack collection is triggered after a sequence of signals starting with S2UI::updateZoom():
     * S2UI::updateZoom()
     * S2UI::finalizeZoom()
     * S2UI::scanStatusHandler()
     * --> user-defined delay from S2UI::startZStackDelaySB
     * S2Controller::startZStack()
     *
     *  most of this sequence is a legacy of early versions getting this off the ground when I was initially building
     * in the capability to switch between galvo and resonant modes.  Tile overhead (e.g. from the user-defined delay above)
     * can seriously slow down S2 scans, so a faster version with event-driven signals could help overall S2 speed.  The idea
     * would be to figure out a fast signal from PV indicating that it's ready to go and allow that to trigger the S2Controller::startZStack() method.
     */
    void s2ROIMonitor();
    /*! \brief internal signal to move to next ROI
     *
     */
    void moveToROI(const TileInfo nextROI);
    /*! \brief internal signal to move to next ROI for stage-based scanning
     *
     */
    void moveToROIWithStage(const TileInfo nextROI);
    /*! \brief clears everything from the ROI plot, including overview and all tile MIPs
     *
     */
    void clearROIPlot();
    /*! \brief critical component for smartScan flow control
     *
     * is called frequently during smartscans, essentially every time a MIP is plotted in the ROI plot.
     * handles updated scan modes, sends messages to qDebug and Status indicating the number of target ROIs
     * also an offramp here to non-continuous scanning, but this should be deprecated and removed.
     */
    void smartScanHandler();
    QString getFileString();
    void loadScanFromFile(QString fileString);
    void toLoad();
    /*! \brief sends string to the noteTaker for logging software actions.
     *
     *  currently the noteTaker log is too busy to really be useful, so alternative  schemes
     * could be implemented here by directing this method to multiple logging/status streams.
     * if done nicely, all the qDebug() statements could be eliminated from S2.
     */
	void status(QString statString);

	void updateLocalRemote(bool state);
    void runSAStuffClicked();
    void updateOverlap(int value);
    void overviewHandler();
    void scanStatusHandler();
    void loadForSA();
    void resetToOverviewPBCB();
    void resetToScanPBCB();
    /*! \brief pick smartscan starting targets from markers in overview image
     *
     *  should work regardless of how the markers were generated.  this method does include
     * some coordinate transformations that could be cleaned up.
     */
    void pickTargets();

    /*! \brief initiates s2 scan of all targets selected in S2UI::pickTargets()
     */
    void startAllTargets();
    void handleAllTargets();
    /*! \brief seemingly innocuous method to plot the MIP image of a stack on the ROI plot
     *
     *  actually critical code here sets final tileInfo for each tile, updates the S2Monitor and
     *  saves the image of the scan footprint S2Image.tif.  All of this logging was originally in S2Monitor, but qpainter
     * operations involving QFont have to happen within the GUI thread.  An alternative scheme for making
     * the S2Image.tif should be done in S2Monitor, perhaps using another library like openCV for writing the number onto the image.
     */
    void loadMIP(double imageNumber, Image4DSimple* mip, QString tileSaveString);
    void loadingDone(Image4DSimple* mip);
    void processingStarted();
    void processingFinished();
    void updateZoom();

    void updateCurrentZoom(int currentIndex);
    void finalizeZoom();
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
    /*! \brief checks input TileInfo against scanned tiles and queued tiles based on pixelLocation info
     *
     *  looks for duplicate tiles (xy within 5 pixels) and also checks to see if all 4 corners are within any tile.
     *  this method (and the associated tile location decisions) currently distribute the decisionmaking across S2UI and StackAnalyzer
     */
    bool isDuplicateROI(TileInfo inputTileInfo);
    void closeEvent(QCloseEvent *event);

    QString fixFileString(QString inputString);

    /*! \brief  make a QColor from argument index given a range of maxIndex colors.
     *
     *  currently clunky.  modify to turn this into a normal colormap!
     */
    QColor makeQColorFromIndex(int maxIndex, int index);



    QList<TileInfo> allOverviewStageLocations;

    QList<TileInfo> allTargetLocations;
    /*! \brief list of tile locations that will be scanned
     *
     *  this is monitored by S2UI::s2ROIMonitor() and also checked for duplicate tiles
     */
    QList<TileInfo> *allROILocations;

    /*! \brief all the tiles for each s2 scan
     *
     *  this is a QList containing LandmarkLists for each S2 scan within a session.
     * tiles are added to the LandmarkList by S2UI::s2ROIMonitor() when a tile is sent to the microscope.
     */
    QList<LandmarkList> allScanLocations;



    QList<LandmarkList> *allTipsList;
    /*! \brief list of all scans in an S2 scan
     *
     * similar to S2UI::allScanLocations , but this is for a single S2 scan
     * scanList is used to keep track of tile locations that leave for StackAnalyzer and come back so we dont have multiple copies of this list
     * spread across S2UI and all the StackAnalyzer instances.
     */
    QList<TileInfo> scanList;

    TileInfo currentTileInfo;

    /*! \brief for each s2scan, this QList contains the list of tips to trace in each tile.
     *
     *   the entries are is passed along to StackAnalyzer as the seedlist for tracing
     */
    QList<LandmarkList> tipList;

    /*! \brief continuously updated Map of microscope parameters
     *
     *  S2Monitor collects this data from the microscope but this S2UI version is
     *  only updated when a value changes.
     */
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
