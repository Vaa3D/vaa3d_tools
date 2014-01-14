/**@file mainwindow.h
 * @brief Main window
 * @author Ting Zhao
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

#include "tz_image_lib_defs.h"
#include "frameinfodialog.h"
#include "zstackpresenter.h"
#include "zmessagereporter.h"
#include "moviedialog.h"
#include "autosaveswclistdialog.h"
#include "zactionactivator.h"

class ZStackFrame;
class QMdiArea;
class QActionGroup;
class ZStackDoc;
class QProgressDialog;
class BcAdjustDialog;
class QUndoGroup;
class QUndoView;
class QMdiSubWindow;
class Z3DCanvas;
class ZStack;
class ZFlyEmDataFrame;
class HelpDialog;
class QProgressBar;
class DiagnosisDialog;
class PenWidthDialog;
class ZDvidClient;
class DvidObjectDialog;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

public: /* frame operation */
  ZStackFrame* activeStackFrame();
  ZStackFrame* currentStackFrame();
  int frameNumber();
  inline QUndoGroup* undoGroup() const { return m_undoGroup; }
  void initOpenglContext();
  void config();

public slots:
  void addStackFrame(ZStackFrame *frame, bool isReady = true);
  void presentStackFrame(ZStackFrame *frame);
  void openFile(const QString &fileName);

  void updateAction();
  void updateMenu();
  void updateStatusBar();

private:
  Ui::MainWindow *m_ui;

  QProgressDialog* getProgressDialog();
  QProgressBar* getProgressBar();

  void setActionActivity();

protected:
  //a virtual function from QWidget. It is called when the window is closed.
  void changeEvent(QEvent *e);
  virtual void closeEvent(QCloseEvent *event);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);

  QString getOpenFileName(const QString &caption,
                          const QString &filter = QString());
  QStringList getOpenFileNames(const QString &caption,
                               const QString &filter = QString());
  QString getSaveFileName(const QString &caption,
                          const QString &filter = QString());
  void createActionMap();

private slots:
  // slots for 'File' menu
  //void on_actionProjectToggle_triggered();
  void connectedThreshold(int x, int y, int z);
  void on_actionConnected_Threshold_triggered();
  void on_actionCanny_Edge_triggered();
  void on_actionWatershed_triggered();
  void on_actionSave_Stack_triggered();
  void on_actionExtract_Channel_triggered();
  void on_actionAutoMerge_triggered();
  void on_actionLoad_from_a_file_triggered();
  void on_actionSave_As_triggered();
  void on_actionFrom_SWC_triggered();
  void on_actionAdd_Reference_triggered();
  void on_actionLoad_triggered();
  void on_actionSave_triggered();
  void on_actionManual_triggered();
  void on_actionAbout_iTube_triggered();
  void on_actionBrightnessContrast_triggered();
  void on_actionProject_triggered();
  void on_actionRefine_Ends_triggered();
  void on_actionRemove_Small_triggered();
  void on_actionUpdate_triggered();
  void on_actionAutomatic_triggered();
  void on_actionAutomatic_Axon_triggered();
  void on_actionDisable_triggered();
  void on_actionEnhance_Line_triggered();
  void on_actionSolidify_triggered();
  void on_actionBinarize_triggered();
  void on_actionOpen_triggered();
  void on_actionEdit_Swc_triggered();
  void on_actionRescale_Swc_triggered();
  void on_action3DView_triggered();
  void save();
  void saveAs();

  //void openTrace();
  void openRecentFile();
  void expandCurrentFrame();

  // for 'File->Export'
  void exportSwc();
  void exportVrml();
  void exportBinary();
  void exportNeuronStructureAsMultipleSwc();
  void exportNeuronStructure();
  void exportChainFileList();
  void exportTubeConnection();
  void exportTubeConnectionFeat();
  void exportSvg();
  void exportTraceProject();
  void exportPuncta();

  // for 'File->Import'
  void openTraceProject();
  void importBinary();
  void importSwc();
  void importGoodTube();
  void importBadTube();
  void importTubeConnection();
  void importPuncta();
  void importImageSequence();

  // slots for 'Edit' menu
//  void undo();
//  void redo();

  // slots for 'View' menu
  void updateViewMode(QAction *action);
  void viewObject(QAction *action);
  void showFrameInfo();
  void checkViewAction(QAction *action);
  void takeScreenshot();

  // slots for 'Tools' menu
  void activateInteractiveTrace(QAction *action);
  void activateInteractiveMarkPuncta(QAction *action);
  void buildConn();
  void manageObjs();
  void binarize();
  void bwsolid();
  void enhanceLine();

  // slots for 'Options'
  void setOption();

  // slots for 'Help'
  void about();
  void test();

  // slots for frame
  void updateFrameInfoDlg();
  void updateActiveUndoStack();
  void removeStackFrame(ZStackFrame *frame);
  void addStackFrame(Stack *stack, bool isOwner = true);
  void addStackFrame(ZStack *stack);
  void updateViewMenu(ZInteractiveContext::ViewMode viewMode);
  void stretchStackFrame(ZStackFrame *frame);

  void updateBcDlg(const ZStackFrame *frame);
  void updateBcDlg();
  void bcAdjust();
  void autoBcAdjust();
  void on_actionMedian_Filter_triggered();

  void on_actionDistance_Map_triggered();

  void on_actionShortest_Path_Flow_triggered();

  void on_actionExpand_Region_triggered();

  void on_actionDilate_triggered();

  void on_actionExtract_Neuron_triggered();

  void on_actionSkeletonization_triggered();

  void on_actionPixel_triggered();

  //Addictional actions of an frame when it's activated
  void evokeStackFrame(QMdiSubWindow *frame);

  void on_actionImport_Network_triggered();

  void on_actionAddSWC_triggered();

  void on_actionImage_Sequence_triggered();

  void on_actionAddFlyEmNeuron_Network_triggered();

  void on_actionSynapse_Annotation_triggered();

  void on_actionPosition_triggered();

  void on_actionImportMask_triggered();

  void on_actionFlyEmSelect_triggered();

  void on_actionImportSegmentation_triggered();

  void on_actionFlyEmClone_triggered();

  void on_actionClear_Decoration_triggered();

  void on_actionFlyEmGrow_triggered();

  void on_actionFlyEmSelect_connection_triggered();

  void on_actionAxon_Export_triggered();

  void on_actionExtract_body_triggered();

  void on_actionPredict_errors_triggered();

  void on_actionCompute_Features_triggered();

  void on_actionMexican_Hat_triggered();

  void on_actionInvert_triggered();

  void on_actionFlyEmQATrain_triggered();

  void on_actionUpdate_Configuration_triggered();

  void on_actionErrorClassifcationTrain_triggered();

  void on_actionErrorClassifcationPredict_triggered();

  void on_actionErrorClassifcationEvaluate_triggered();

  void on_actionErrorClassifcationComputeFeatures_triggered();

  void on_actionTem_Paper_Volume_Rendering_triggered();

  void on_actionTem_Paper_Neuron_Type_Figure_triggered();

  void on_actionBinary_SWC_triggered();

  void on_actionImportFlyEmDatabase_triggered();

  void on_actionMake_Movie_triggered();

  void on_actionOpen_3D_View_Without_Volume_triggered();

  void on_actionLoop_Analysis_triggered();

  void on_actionMorphological_Thinning_triggered();

  void on_actionAddMask_triggered();

  void on_actionMask_triggered();

  void on_actionShortcut_triggered();

  void on_actionMake_Projection_triggered();

  void on_actionMask_SWC_triggered();

  void on_actionAutosaved_Files_triggered();

  void on_actionDiagnosis_triggered();

  void on_actionSave_SWC_triggered();

  void on_actionSimilarity_Matrix_triggered();

  void on_actionSparse_objects_triggered();

  void on_actionDendrogram_triggered();

  void on_actionPen_Width_for_SWC_Display_triggered();

  void on_actionDVID_Object_triggered();

private:
  void createActions();
  void customizeActions();
  void createMenus();
  void createContextMenu();
  void createToolBars();
  void createStatusBar();
  void readSettings();
  void writeSettings();
  bool okToContinue();
  void saveFile(const QString &fileName);
  void openTraceProject(QString fileName);
  void setCurrentFile(const QString &fileName);
  void createAutoSaveDir();
  void updateRecentFileActions();
  QString strippedName(const QString &fullFileName);

  void enableStackActions(bool b);
  void createUndoView();
  //bool loadTraceFile(const QString &fileName);

  //Record <path> as the path (could be a file or directory) opened last time.
  void recordLastOpenPath(const QString &path);

  //Get the path opened last time.
  QString getLastOpenPath() const;

  //Report the problem when a file cannot be opened correctly.
  void reportFileOpenProblem(const QString &filePath,
                             const QString &reason = "");

  //Add a flyem data frame. Nothing happens if <frame> is NULL.
  void addFlyEmDataFrame(ZFlyEmDataFrame *frame);

  //Error handling
  void report(const std::string &title, const std::string &msg,
              ZMessageReporter::EMessageType msgType);

private:
  QMdiArea *mdiArea;

  QStringList recentFiles;
  QString curFile;
  QString m_lastOpenedFilePath;

  enum { MaxRecentFiles = 10 };
  QAction *recentFileActions[MaxRecentFiles];
  QAction *separatorAction;

  QMenu *exportMenu;
  QMenu *importMenu;
  QMenu *objectViewMenu;
  QMenu *traceMenu;
//  QMenu *processMenu;

  // toolbars
  QToolBar *fileToolBar;
  QToolBar *editToolBar;

  // 'File' menu: 'New', 'Open', 'Import', 'Export', 'Save', 'Save As',
  //              'Close', 'Exit'
  QActionGroup *m_readActionGroup;
  QActionGroup *m_writeActionGroup;
  QActionGroup *m_viewActionGroup;
  QAction *newAction;
  QAction *openAction;
  QAction *expandAction;
  QAction *swcExportAction;
  QAction *svgExportAction;
  QAction *vrmlExportAction;
  QAction *bnExportAction;
  QAction *nsExportAction;
  QAction *nsMultipleSwcExportAction;
  QAction *connExportAction;
  QAction *connFeatExportAction;
  QAction *chainSourceExportAction;
  //QAction *projectExportAction;
  //QAction *projectImportAction;
  QAction *bnImportAction;
  QAction *swcImportAction;
  QAction *gtImportAction;
  QAction *btImportAction;
  QAction *connImportAction;
  QAction *closeAction;
  QAction *exitAction;
  QAction *punctaImportAction;
  QAction *punctaExportAction;
  QAction *imageDirImportAction;

  // 'Edit' menu: 'Cut', 'Copy', 'Paste', 'Delete', 'Undo'
  QAction *undoAction;
  QAction *redoAction;

  // 'View' menu: 'Mode' ('Normal', 'Project', 'Zoom'),
  //   'Objects' ('Hide', 'Normal', 'Solid', 'Surface', 'Skeleton')
  //   'Document'
  QActionGroup *viewMode;
  //QAction *normalAction;
  //QAction *projAction;
  QAction *zoomAction;
  QActionGroup *objectView;
  //QAction *objectViewHideAction;
  QAction *objectViewNormalAction;
  QAction *objectViewSolidAction;
  QAction *objectViewSurfaceAction;
  QAction *objectViewSkeletonAction;
  QAction *infoViewAction;
  QAction *screenshotAction;

  // 'Tools' menu:
  //   'Trace' ('Disable', 'Fit segment', 'Trace tube', 'Automatic')
  //   'Build Conn'
  //   'Process' ('Binarize', 'Solidify', ...)
  QActionGroup *interactiveTrace;
  QAction *noTraceAction;
  QAction *fitsegAction;
  QAction *traceTubeAction;
  QAction *autoTraceAction;

  QActionGroup *interactiveMarkPuncta;
  QAction *noMarkPunctaAction;
  QAction *markPunctaAction;

  QAction *buildConnAction;
  QAction *processBinarizeAction;
  QAction *processBwsolidAction;

  QAction *manageObjsAction;

  // 'Options' menu: 'Settings'
  QAction *settingAction;

  // 'Help' menu: 'About'
  //QAction *aboutAction;
  //QAction *aboutQtAction;
  QAction *testAction;

  QAction *openTraceAction;

  ZStackActionActivator m_stackActionActivator;

  FrameInfoDialog m_frameInfoDlg;
  QProgressDialog *m_progress;
  BcAdjustDialog *m_bcDlg;
  HelpDialog *m_helpDlg;
  DiagnosisDialog *m_DiagnosisDlg;

  // undo redo
  QUndoGroup *m_undoGroup;
  QUndoView *m_undoView;

  Z3DCanvas *m_sharedContext;

  ZMessageReporter *m_reporter;

  int m_frameCount;


  MovieDialog m_movieDlg;
  AutosaveSwcListDialog m_autosaveSwcDialog;

  PenWidthDialog *m_penWidthDialog;

  QMap<QString, QAction*> m_actionMap;

  ZDvidClient *m_dvidClient;
  DvidObjectDialog *m_dvidObjectDlg;
};

#endif // MAINWINDOW_H
