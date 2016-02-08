

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
public slots:
    void pmStatusHandler(bool pmStatus);
    void handleNewLocation(QList<LandmarkList> newTipsList, LandmarkList newlandmarks);
    void loadLatest();
    void collectOverview();
    void getCurrentParameters();
    void combinedSmartScan(QString);
signals:
    void startPM();
    void stopPM();
    void callSALoad(QString,float,int,bool,LandmarkList, LocationSimple, QString);
    void newImageData(Image4DSimple);
    void moveToNext(LocationSimple);
	void noteStatus(QString);
    void processSmartScanSig(QString);
    void currentParameters(QMap<int, S2Parameter> currentParameterMap);
    void updateTable(LandmarkList allTargetLocations,QList<LandmarkList> allScanLocations);
private slots:
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
    void s2ROIMonitor();
    void moveToROI(LocationSimple);
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
private:
    V3DPluginCallback2 * cb;

   QThread *workerThread;

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

    QGroupBox *createROIControls();
    QGroupBox *createS2Monitors();
    QGroupBox *createTracingParameters();



    void createButtonBox1();
	

    void createTargetList();

	QCheckBox *localRemoteCB;
    QPushButton *runAllTargetsPB;

    QLabel *s2Label;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QPushButton  *startZStackPushButton;
    QPushButton * resetToOverviewPB;
    QPushButton * resetToScanPB;
    QPushButton * pickTargetsPB;

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

// Target and ROI table widget

    TargetList myTargetTable;
    int targetIndex;


// External S2 controllers

    S2Controller myController;
    S2Controller myPosMon;


// gui hookup

    void hookUpSignalsAndSlots();
    QDir getSaveDirectory();

//  internal methods and important parameters
    void checkParameters(QMap<int, S2Parameter> currentParameterMap);
    bool isDuplicateROI(LocationSimple inputLocation);
    void closeEvent(QCloseEvent *event);

    LandmarkList allTargetLocations;
    QList<LandmarkList> allScanLocations;
    LandmarkList *allROILocations;
    QList<LandmarkList> *allTipsList;
    QMap<int, S2Parameter> uiS2ParameterMap;

    QString fileString ;
    QString lastFile;
    bool posMonStatus;
    int waitingForFile;//  0 => not waiting.  1 => waiting for image.
    bool waitingForOverview;
	bool isLocal;
    int smartScanStatus;
    double scanNumber;
    int resultNumber;
    int allTargetStatus;
    float overViewPixelToScanPixel;

    LandmarkList scanList;
    QList<LandmarkList> tipList;

    Image4DSimple*  total4DImage;

    NoteTaker *myNotes;

    QDir saveDir;
    QDir sessionDir;
    QDir currentDirectory;
    QFile saveTextFile;
    QTextStream outputStream;
    QString scanDataFileString;
    float overlap;
    int overviewCycles;
    int scanStatusWaitCycles;
    bool havePreview;

//   vaa3d variables
    v3dhandle previewWindow;

};
//! [0]

#endif
