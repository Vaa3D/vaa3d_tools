#include "mainwindow.h"

#include "z3dgl.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
//#include <QtSvg>
#include <QDir>

#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif

#include "ui_mainwindow.h"

#include "tz_darray.h"

#include "zstackframe.h"
#include "zstackdoc.h"
#include "settingdialog.h"
#include "zstackview.h"
#include "zimagewidget.h"
#include "zinteractivecontext.h"
#include "traceoutputdialog.h"
#include "bcadjustdialog.h"
#include "channeldialog.h"
#include "tz_math.h"
//itkimagedefs.h has to be included before tz_error.h for unknown reason.
#include "zstackprocessor.h"
#include "tz_error.h"
#include "zeditswcdialog.h"
#include "cannyedgedialog.h"
#include "medianfilterdialog.h"
#include "diffusiondialog.h"
#include "connectedthresholddialog.h"
#include "zstackpresenter.h"
#include "zstack.hxx"
#include "zrescaleswcdialog.h"
#include "tz_image_io.h"
#include "distancemapdialog.h"
#include "regionexpanddialog.h"
#include "neuroniddialog.h"
#include "zcircle.h"

#include "tz_sp_grow.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_stat.h"
#include "tz_stack_attribute.h"
#include "zspgrowparser.h"
#include "zvoxelarray.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_threshold.h"
#include "zsuperpixelmaparray.h"
#include "zsegmentmaparray.h"
#include "tz_xml_utils.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_graph_defs.h"
#include "tz_graph_utils.h"
#include "tz_workspace.h"
#include "tz_graph.h"
#include "flyemskeletonizationdialog.h"
#include "zstackaccessor.h"
#include "zmatrix.h"
#include "zswcbranch.h"
#include "zswctreematcher.h"
#include "parameterdialog.h"
#include "zstring.h"
#include "zrandomgenerator.h"
#include "zjsonobject.h"
#include "zpoint.h"
#include "tz_geo3d_utils.h"
#include "zsvggenerator.h"
#include "zdendrogram.h"
#include "zcuboid.h"
#include "QsLog/QsLog.h"
#include "flyem/zneuronnetwork.h"
#include "zswcnetwork.h"
#include "zstackfile.h"
#include "flyem/zflyemstackframe.h"
#include "flyem/zsegmentationanalyzer.h"
#include "zfiletype.h"
#include "mexicanhatdialog.h"
#include "neutubeconfig.h"
#include "zfilelist.h"
#include "z3dwindow.h"
#include "z3dvolumesource.h"
#include "z3dcompositor.h"
#include "zstackskeletonizer.h"
#include "flyem/zflyemdataframe.h"
#include "zmoviescript.h"
#include "zmoviemaker.h"
#include "z3dvolumeraycaster.h"
#include "zstackdoccommand.h"
#include "zqtmessagereporter.h"
#include "helpdialog.h"
#include "zstackstatistics.h"
#include "zqtbarprogressreporter.h"
#include "projectiondialog.h"
#include "biocytin/swcprocessor.h"
#include "startsettingdialog.h"
#include "zstackreadthread.h"
#include "zswcpositionadjuster.h"
#include "diagnosisdialog.h"
#include "swc/zswcresampler.h"
#include "biocytin/zbiocytinfilenameparser.h"

#if !defined(__MINGW32__)
#include "ztest.h"
#endif

#include "z3dcanvas.h"
#include "z3dapplication.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_frameInfoDlg(this),
    m_autosaveSwcDialog(this)
{
  std::cout << "Creating mainwindow ..." << std::endl;

#ifdef _DEBUG_2
  std::cout << NeutubeConfig::getInstance().getPath(NeutubeConfig::AUTO_SAVE)
               << std::endl;
#endif

  m_reporter = new ZQtMessageReporter();

  m_lastOpenedFilePath = ".";
  m_ui->setupUi(this);
  this->setWindowIcon(QIcon(":/images/app.png"));
  mdiArea = new QMdiArea;
  mdiArea->setActivationOrder(QMdiArea::StackingOrder);

  setCentralWidget(mdiArea);
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          this, SLOT(updateMenu()));
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          this, SLOT(updateStatusBar()));
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          this, SLOT(updateFrameInfoDlg()));

  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          this, SLOT(updateActiveUndoStack()));

  connect(&m_frameInfoDlg, SIGNAL(newCurveSelected(int)),
          this, SLOT(updateFrameInfoDlg()));
  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          this, SLOT(updateBcDlg()));

  connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
          this, SLOT(evokeStackFrame(QMdiSubWindow*)));

  m_undoGroup = new QUndoGroup(this);
  createUndoView();

  //create the rest of the window
  std::cout << "Creating menus ..." << std::endl;
  createActions();
  std::cout << "Creating menus ......" << std::endl;
  createMenus();
  std::cout << "Creating menus ........." << std::endl;
  createContextMenu();

  std::cout << "Creating toolbars ..." << std::endl;
  createToolBars();
  createStatusBar();

  readSettings();
  setCurrentFile("");
  createAutoSaveDir();

  m_progress = new QProgressDialog(this);
  m_progress->setCancelButton(0);

  m_bcDlg = new BcAdjustDialog(this);
  connect(m_bcDlg, SIGNAL(valueChanged()), this, SLOT(bcAdjust()));
  connect(m_bcDlg, SIGNAL(autoAdjustTriggered()), this, SLOT(autoBcAdjust()));

  m_helpDlg = new HelpDialog(this);
  m_DiagnosisDlg = new DiagnosisDialog(this);

  setAcceptDrops(true);

  // init openGL context
  QGLFormat format = QGLFormat();
  format.setAlpha(true);
  format.setDepth(true);
  format.setDoubleBuffer(true);
  format.setRgba(true);
  format.setSampleBuffers(true);
  format.setStereo(true);
  m_sharedContext = new Z3DCanvas("Init Canvas", 32, 32, format, this);

  m_frameCount = 0;

  createActionMap();
}

MainWindow::~MainWindow()
{
  delete m_ui;
  delete m_reporter;
}

void MainWindow::createActionMap()
{
}

void MainWindow::config()
{
  if (!NeutubeConfig::getInstance().getApplication().empty()) {
    setWindowTitle(windowTitle() + " - " +
                   NeutubeConfig::getInstance().getApplication().c_str());
  }

  customizeActions();
}

void MainWindow::changeEvent(QEvent *e)
{
  QMainWindow::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void MainWindow::createUndoView()
{
//  m_undoView = new QUndoView(m_undoGroup, this);
//  m_undoView->setWindowTitle(tr("Command History"));
//  m_undoView->show();
//  m_undoView->setAttribute(Qt::WA_QuitOnClose, false);
}

void MainWindow::createActions()
{
  openAction = m_ui->actionOpen;

  expandAction = new QAction(tr("&Expand current"), this);
  expandAction->setStatusTip(tr("Expand current document with SWCs or masks"));
  expandAction->setIcon(QIcon(":/images/expand.png"));
  connect(expandAction, SIGNAL(triggered()), this, SLOT(expandCurrentFrame()));

  m_readActionGroup = new QActionGroup(this);
  bnImportAction = new QAction(tr("Import &Tracing result"), m_readActionGroup);
  bnImportAction->setStatusTip(tr("Import binary tracing results"));
  connect(bnImportAction, SIGNAL(triggered()), this, SLOT(importBinary()));

  swcImportAction = new QAction(tr("&SWC file"), m_readActionGroup);
  swcImportAction->setStatusTip(tr("Import SWC file"));
  connect(swcImportAction, SIGNAL(triggered()), this, SLOT(importSwc()));

  gtImportAction = new QAction(tr("&Good tubes"), m_readActionGroup);
  connect(gtImportAction, SIGNAL(triggered()), this, SLOT(importGoodTube()));

  btImportAction = new QAction(tr("&Bad tubes"), m_readActionGroup);
  connect(btImportAction, SIGNAL(triggered()), this, SLOT(importBadTube()));

  connImportAction = new QAction(tr("&Tube connection"), m_readActionGroup);
  connect(connImportAction, SIGNAL(triggered()),
    this, SLOT(importTubeConnection()));


  punctaImportAction = new QAction(tr("&puncta file"), m_readActionGroup);
  punctaImportAction->setStatusTip(tr("Import puncta file"));
  connect(punctaImportAction, SIGNAL(triggered()), this, SLOT(importPuncta()));

  for (int i = 0; i < MaxRecentFiles; ++i) {
    recentFileActions[i] = new QAction(m_readActionGroup);
    recentFileActions[i]->setVisible(false);
    connect(recentFileActions[i], SIGNAL(triggered()),
      this, SLOT(openRecentFile()));
  }

  m_ui->actionMake_Projection->setIcon(QIcon(":/images/project.png"));

  imageDirImportAction = new QAction(tr("&Image sequence"), m_readActionGroup);
  imageDirImportAction->setStatusTip(tr("Import image sequence"));
  connect(imageDirImportAction, SIGNAL(triggered()),
          this, SLOT(importImageSequence()));

  m_writeActionGroup = new QActionGroup(this);

  m_writeActionGroup->addAction(m_ui->actionSave);
  m_writeActionGroup->addAction(m_ui->actionSave_As);
  m_writeActionGroup->addAction(m_ui->actionSave_Stack);

  swcExportAction = new QAction(tr("&SWC file"), m_writeActionGroup);
  swcExportAction->setStatusTip(tr("Export tracing results as a SWC file"));
  connect(swcExportAction, SIGNAL(triggered()), this, SLOT(exportSwc()));

  svgExportAction = new QAction(tr("&SVG file"), m_writeActionGroup);
  svgExportAction->setStatusTip(tr("Export a SWC file to a SVG file"));
  connect(svgExportAction, SIGNAL(triggered()), this, SLOT(exportSvg()));

  vrmlExportAction = new QAction(tr("&VRML file"), m_writeActionGroup);
  vrmlExportAction->setStatusTip(tr("Export tracing results as a VRML file"));
  connect(vrmlExportAction, SIGNAL(triggered()), this, SLOT(exportVrml()));

  bnExportAction = new QAction(tr("&Tracing result"), m_writeActionGroup);
  bnExportAction->setStatusTip(tr("Export tracing that can be loaded later"));
  connect(bnExportAction, SIGNAL(triggered()), this, SLOT(exportBinary()));

  nsExportAction = new QAction(tr("&Neuron structure"), m_writeActionGroup);
  nsExportAction->setStatusTip(tr("Export neuron structure as a SWC file"));
  connect(nsExportAction, SIGNAL(triggered()),
    this, SLOT(exportNeuronStructure()));

  nsMultipleSwcExportAction =
      new QAction(tr("&Neuron structure as multiple SWC"), m_writeActionGroup);
  nsMultipleSwcExportAction->setStatusTip(tr("Export neuron structure as multiple SWC files"));
  connect(nsMultipleSwcExportAction, SIGNAL(triggered()),
    this, SLOT(exportNeuronStructureAsMultipleSwc()));

  connExportAction = new QAction(tr("&Tube connection"), m_writeActionGroup);
  connect(connExportAction, SIGNAL(triggered()),
    this, SLOT(exportTubeConnection()));

  connFeatExportAction = new QAction(tr("&Connection feature"),
                                     m_writeActionGroup);
  connect(connFeatExportAction, SIGNAL(triggered()),
          this, SLOT(exportTubeConnectionFeat()));

  chainSourceExportAction = new QAction(tr("Chain file list"),
                                        m_writeActionGroup);
  connect(chainSourceExportAction, SIGNAL(triggered()),
      this, SLOT(exportChainFileList()));


  punctaExportAction = new QAction(tr("&Puncta file"), m_writeActionGroup);
  punctaExportAction->setStatusTip(tr("Export puncta as a SWC file"));
  connect(punctaExportAction, SIGNAL(triggered()), this, SLOT(exportPuncta()));



  exitAction = new QAction(tr("E&xit"), this);
  //no starndarized key sequence for exitAction
  exitAction->setShortcut(tr("Ctrl+Q"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

  undoAction = m_undoGroup->createUndoAction(this, tr("&Undo"));
  undoAction->setIcon(QIcon(":/images/undo.png"));
  undoAction->setShortcuts(QKeySequence::Undo);

  redoAction = m_undoGroup->createRedoAction(this, tr("&Redo"));
  redoAction->setIcon(QIcon(":/images/redo.png"));
  redoAction->setShortcuts(QKeySequence::Redo);

  m_viewActionGroup = new QActionGroup(this);
  m_viewActionGroup->addAction(m_ui->action3DView);

  viewMode = new QActionGroup(m_viewActionGroup);

  viewMode->addAction(m_ui->actionNormal);
  viewMode->addAction(m_ui->actionProject);

  connect(viewMode, SIGNAL(triggered(QAction*)),
    this, SLOT(updateViewMode(QAction*)));

  objectView = new QActionGroup(m_viewActionGroup);
  objectView->setExclusive(false);

  //objectViewNormalAction = new QAction(tr("&Normal"), objectView);
  objectViewNormalAction = new QAction(tr("&Normal"), this);
  objectViewNormalAction->setIcon(QIcon(":/images/normalobj.png"));
  objectViewNormalAction->setCheckable(true);
  objectViewNormalAction->setStatusTip(tr("Show all objects"));

  objectViewSolidAction = new QAction(tr("&Dense"), objectView);
  objectViewSolidAction->setIcon(QIcon(":/images/normalobj.png"));
  objectViewSolidAction->setCheckable(true);
  objectViewSolidAction->setStatusTip(tr("Show objects in a dense form"));

  objectViewSurfaceAction = new QAction(tr("&Sparse"), objectView);
  objectViewSurfaceAction->setIcon(QIcon(":/images/surfobj.png"));
  objectViewSurfaceAction->setCheckable(true);
  objectViewSurfaceAction->setStatusTip(tr("Show objects in a sparse form"));

  objectViewSkeletonAction = new QAction(tr("&Skeleton"), objectView);
  objectViewSkeletonAction->setIcon(QIcon(":/images/skelobj.png"));
  objectViewSkeletonAction->setCheckable(true);
  objectViewSkeletonAction->setStatusTip(tr("Show objects in the skeleton form"));

  objectViewNormalAction->setChecked(true);
  connect(objectView, SIGNAL(triggered(QAction*)),
    this, SLOT(viewObject(QAction*)));

  infoViewAction = new QAction(tr("&Information"), this);
  infoViewAction->setStatusTip("View data information of the active window");
  infoViewAction->setIcon(QIcon(":/images/document.png"));
  connect(infoViewAction, SIGNAL(triggered()), this, SLOT(showFrameInfo()));

  screenshotAction = new QAction(tr("&Take Screenshot"), this);
  screenshotAction->setStatusTip("Take screenshot");
  screenshotAction->setIcon(QIcon(":/images/screenshot_toolbar.png"));
  connect(screenshotAction, SIGNAL(triggered()), this, SLOT(takeScreenshot()));

  interactiveTrace = new QActionGroup(this);
  interactiveTrace->addAction(m_ui->actionDisable);
  interactiveTrace->addAction(m_ui->actionFit_Segment);
  interactiveTrace->addAction(m_ui->actionTrace_Tube);
  interactiveTrace->addAction(m_ui->actionTree_Preview);

  connect(interactiveTrace, SIGNAL(triggered(QAction*)),
    this, SLOT(activateInteractiveTrace(QAction*)));
  //traceTubeAction->setChecked(true);

  interactiveMarkPuncta = new QActionGroup(this);
  interactiveMarkPuncta->addAction(m_ui->actionDisable_Mark_Puncta);
  interactiveMarkPuncta->addAction(m_ui->actionMark_Puncta);
  connect(interactiveMarkPuncta, SIGNAL(triggered(QAction*)), this,
          SLOT(activateInteractiveMarkPuncta(QAction*)));

  buildConnAction = new QAction(tr("Build Connection"), this);
  connect(buildConnAction, SIGNAL(triggered()), this, SLOT(buildConn()));

  manageObjsAction = new QAction(tr("&Manage Objects..."), this);
  manageObjsAction->setStatusTip(tr("Manage objects (swcs, puncta) of current frame"));
  connect(manageObjsAction, SIGNAL(triggered()), this, SLOT(manageObjs()));

  settingAction = new QAction(tr(" &Settings ..."), this);
  settingAction->setStatusTip(tr("Environment setup"));
  settingAction->setIcon(QIcon(":/images/setting.png"));
  connect(settingAction, SIGNAL(triggered()), this, SLOT(setOption()));

  m_ui->actionOpen_3D_View_Without_Volume->setIcon(QIcon(":/images/3dview.png"));

  m_ui->actionShortcut->setIcon(QIcon(":/images/help2.png"));

  m_ui->actionMask_SWC->setIcon(QIcon(":/images/masktoswc.png"));

#ifdef _DEBUG_
  testAction = new QAction(tr("&Test"), this);
  testAction->setShortcut(tr("Ctrl+T"));
  testAction->setStatusTip(tr("Test"));
  testAction->setIcon(QIcon(":/images/test.png"));
  connect(testAction, SIGNAL(triggered()), this, SLOT(test()));
#endif

  //customizeActions();
}

void MainWindow::customizeActions()
{
  const NeutubeConfig& config = NeutubeConfig::getInstance();

  bool isTracingOn = config.getMainWindowConfig().isTracingOn();
  m_ui->actionTrace_Tube->setVisible(isTracingOn);
  m_ui->actionDisable->setChecked(!isTracingOn);
  m_ui->actionDisable->setVisible(isTracingOn);
  m_ui->actionTree_Preview->setVisible(isTracingOn);
  m_ui->actionTracing_result->setVisible(isTracingOn);
  m_ui->actionFit_Segment->setVisible(isTracingOn);
  m_ui->actionAutomatic->setVisible(isTracingOn);
  m_ui->actionAutomatic_Axon->setVisible(isTracingOn);
  m_ui->actionFrom_SWC->setVisible(isTracingOn);
  m_ui->menuTube->menuAction()->setVisible(isTracingOn);
  m_ui->menuTrace_Project->menuAction()->setVisible(isTracingOn);
  m_ui->actionSave_SWC->setVisible(isTracingOn);
  this->buildConnAction->setVisible(isTracingOn);

  bool isSwcEditOn = config.getMainWindowConfig().isSwcEditOn();
  m_ui->menuSwc->menuAction()->setVisible(isSwcEditOn);

  bool isMarkPuncatOn = config.getMainWindowConfig().isMarkPunctaOn();
  m_ui->menuPuncta->menuAction()->setVisible(isMarkPuncatOn);

  m_ui->actionAddNeuron_Network->setVisible(
        config.getMainWindowConfig().isExpandNeuronNetworkOn());
  m_ui->actionV3D_Apo->setVisible(
        config.getMainWindowConfig().isExpandV3dApoOn());
  m_ui->actionV3D_Marker->setVisible(
        config.getMainWindowConfig().isExpandV3dMarkerOn());

  m_ui->actionBinarize->setVisible(
        config.getMainWindowConfig().isProcessBinarizeOn());
  m_ui->actionBinary_SWC->setVisible(
        config.getMainWindowConfig().isBinaryToSwcOn());
  m_ui->actionMask_SWC->setVisible(
        config.getMainWindowConfig().isMaskToSwcOn());

  bool hasApplication = false;

  if (!config.getApplication().empty()) {
    if (config.getApplication() != "FlyEM") {
      m_ui->menuFLy_EM->menuAction()->setVisible(false);
    } else {
      m_ui->menuFLy_EM->menuAction()->setVisible(true);
      hasApplication = true;
    }
  }

  if (!config.isSettingOn()) {
    settingAction->setVisible(false);
  }

  m_ui->menuApplications->menuAction()->setVisible(hasApplication);
  m_ui->actionMake_Projection->setVisible(false);
  m_ui->actionTree_Preview->setVisible(false);

  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_ui->actionMake_Projection->setVisible(true);
    m_ui->actionUpdate->setVisible(false);
    m_ui->menuFilter->menuAction()->setVisible(false);
    m_ui->menuBinary_Morphology->menuAction()->setVisible(false);
    m_ui->menuSegmentation->menuAction()->setVisible(false);
    m_ui->menuEdge_Detection->menuAction()->setVisible(false);
    m_ui->menuQuery->menuAction()->setVisible(false);
    m_ui->menuOptions->menuAction()->setVisible(false);
    punctaExportAction->setVisible(false);
  } else {
    expandAction->setVisible(false);
    m_ui->actionAddMask->setVisible(false);
    m_ui->actionMask->setVisible(false);
    m_ui->actionMake_Projection->setVisible(false);
    m_ui->actionMask_SWC->setVisible(false);
    //m_ui->menuHelp->menuAction()->setVisible(false);
  }

#ifdef _DEBUG_
  testAction->setVisible(
        NeutubeConfig::getInstance().getApplication() != "Biocytin");
#endif
}

void MainWindow::createMenus()
{
  //the menu bar is created the first time menuBar is called
  exportMenu = m_ui->menuFile->addMenu(tr("&Export"));
  exportMenu->addAction(bnExportAction);
  exportMenu->addAction(nsExportAction);
  exportMenu->addAction(nsMultipleSwcExportAction);
  exportMenu->addAction(svgExportAction);

#ifdef _ADVANCED_
  exportMenu->addAction(chainSourceExportAction);
  exportMenu->addAction(connExportAction);
  exportMenu->addAction(connFeatExportAction);
#endif
  //exportMenu->addAction(punctaExportAction);
  m_ui->menuExport->addAction(punctaExportAction);

  exportMenu->menuAction()->setVisible(false);

  m_ui->menuTrace_Project->addAction(bnImportAction);

  separatorAction = m_ui->menuFile->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i) {
    m_ui->menuFile->addAction(recentFileActions[i]);
  }
  m_ui->menuFile->addSeparator();
  m_ui->menuFile->addAction(exitAction);

  m_ui->menuEdit->addAction(undoAction);
  m_ui->menuEdit->addAction(redoAction);

  objectViewMenu = m_ui->menuView->addMenu(tr("&Object"));
  objectViewMenu->addActions(objectView->actions());
  m_ui->menuView->addAction(screenshotAction);
  m_ui->menuView->addSeparator();
  m_ui->menuView->addAction(infoViewAction);

  //traceMenu = m_ui->menuTools->addMenu(tr("&Trace"));
  //traceMenu->addActions(interactiveTrace->actions());
  noTraceAction = m_ui->actionDisable;
  fitsegAction = m_ui->actionFit_Segment;
  traceTubeAction = m_ui->actionTrace_Tube;
  autoTraceAction = m_ui->actionAutomatic;

  noMarkPunctaAction = m_ui->actionDisable_Mark_Puncta;
  markPunctaAction = m_ui->actionMark_Puncta;

  //m_ui->menuTools->addAction(editSwcAction);
  m_ui->menuTools->addAction(buildConnAction);
  m_ui->menuTools->addAction(manageObjsAction);

#ifndef _ADVANCED_
  m_ui->menuProcess->setEnabled(false);
#endif

  m_ui->menuOptions->addAction(settingAction);
#ifdef _DEBUG_
  m_ui->menuHelp->addAction(testAction);
#endif

#if defined(_FLYEM_)
  m_ui->menuFLy_EM->setEnabled(true);
#else
  m_ui->menuFLy_EM->setEnabled(false);
#endif

  updateMenu();

  //ZStackFrame *frame = new ZStackFrame(this);
  //addStackFrame(frame);
}

void MainWindow::createContextMenu()
{
}

void MainWindow::createToolBars()
{
//  fileToolBar = addToolBar(tr("&File"));
  //fileToolBar->addAction(newAction);
  m_ui->toolBar->addAction(openAction);

  if (NeutubeConfig::getInstance().getApplication() == "FlyEM") {
    m_ui->toolBar->addAction(m_ui->actionImportFlyEmDatabase);
  }

  m_ui->toolBar->addAction(expandAction);
  m_ui->toolBar->addAction(m_ui->actionMask);
  //fileToolBar->addAction(saveAction);

  m_ui->toolBar->addSeparator();

  m_ui->toolBar->addActions(interactiveTrace->actions());

  //m_ui->toolBar->addSeparator();
  //m_ui->toolBar->addAction(m_ui->actionAutomatic);

  m_ui->toolBar->addAction(m_ui->actionMake_Projection);
  m_ui->toolBar->addAction(m_ui->actionMask_SWC);

  m_ui->toolBar->addSeparator();
  //m_ui->toolBar->addAction(objectViewHideAction);
  m_ui->toolBar->addAction(objectViewSolidAction);
  m_ui->toolBar->addAction(objectViewSurfaceAction);
  m_ui->toolBar->addAction(objectViewSkeletonAction);

  m_ui->toolBar->addSeparator();

  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_ui->toolBar->addAction(m_ui->actionOpen_3D_View_Without_Volume);
  }

  m_ui->toolBar->addAction(m_ui->actionBrightnessContrast);

#ifdef _ADVANCED_
  //m_ui->toolBar->addAction(infoViewAction);
#endif

  m_ui->toolBar->addAction(settingAction);
  m_ui->toolBar->addSeparator();
  m_ui->toolBar->addAction(screenshotAction);
#ifdef _DEBUG_
  m_ui->toolBar->addAction(testAction);
#endif
  m_ui->toolBar->addAction(m_ui->actionShortcut);
}

void MainWindow::createStatusBar()
{
  updateStatusBar();
}

void MainWindow::updateStatusBar()
{
  if (frameNumber() == 0) {
    statusBar()->showMessage(tr("Load a stack to start"));
  } else {
    ZStackFrame *frame = activeStackFrame();
    if (frame != NULL) {
      statusBar()->showMessage(frame->briefInfo());
    }
  }
}

void MainWindow::updateAction()
{
  ZStackFrame *frame = currentStackFrame();

  m_writeActionGroup->setDisabled(frame == NULL);
  m_viewActionGroup->setDisabled(frame == NULL);
  viewMode->setDisabled(frame == NULL);
  manageObjsAction->setDisabled(activeStackFrame() == NULL);


  if (frame != NULL) {
    if (frame->presenter() != NULL) {
      undoAction = frame->document()->getUndoAction();
      redoAction = frame->document()->getRedoAction();
    }
  }
}

void MainWindow::updateMenu()
{
  updateAction();
  if (frameNumber() == 0) {
    enableStackActions(false);
  } else {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      if (frame->presenter() != NULL) { /* not a closing frame */
        m_ui->menuEdit->clear();
        m_ui->menuEdit->addAction(undoAction);
        m_ui->menuEdit->addAction(redoAction);

        enableStackActions(true);

        updateViewMenu(frame->presenter()->interactiveContext().viewMode());

        if (NeutubeConfig::getInstance().getMainWindowConfig().isTracingOn()) {
          switch (frame->presenter()->interactiveContext().traceMode()) {
          //case ZStackPresenter::INT_NONE:
          case ZInteractiveContext::TRACE_OFF:
            m_ui->actionDisable->setChecked(true);
            break;
            //case ZStackPresenter::INT_FIT_LOCSEG:
          case ZInteractiveContext::TRACE_SINGLE:
            m_ui->actionFit_Segment->setChecked(true);
            break;
          case ZInteractiveContext::TRACE_TUBE:
            m_ui->actionTrace_Tube->setChecked(true);
            break;
          case ZInteractiveContext::TRACE_PREIVEW_RECONSTRUCTION:
            m_ui->actionTree_Preview->setChecked(true);
            break;
          default:
            break;
          }
        }

        switch (frame->presenter()->interactiveContext().editPunctaMode()) {
        case ZInteractiveContext::MARK_PUNCTA_OFF:
          m_ui->actionDisable_Mark_Puncta->setChecked(true);
          break;
        case ZInteractiveContext::MARK_PUNCTA:
          m_ui->actionMark_Puncta->setChecked(true);
          break;
        default:
          break;
        }

        if (frame->presenter()->isObjectVisible() == false) {
          checkViewAction(NULL);
        } else {
          switch (frame->presenter()->objectStyle()) {
          case ZStackDrawable::NORMAL:
            checkViewAction(objectViewSolidAction);
            break;
          case ZStackDrawable::SOLID:
            checkViewAction(objectViewSolidAction);
            break;
          case ZStackDrawable::BOUNDARY:
            checkViewAction(objectViewSurfaceAction);
            break;
          case ZStackDrawable::SKELETON:
            checkViewAction(objectViewSkeletonAction);
            break;
          }
        }
      } else {
        enableStackActions(false);
      }
    } else {
      enableStackActions(false);
    }
  }
}

void MainWindow::enableStackActions(bool b)
{
  noTraceAction->setEnabled(b);
  fitsegAction->setEnabled(b);
  traceTubeAction->setEnabled(b);
  expandAction->setEnabled(b);

  m_ui->actionMake_Projection->setEnabled(b);

  m_ui->actionAddSWC->setEnabled(b);

  m_ui->actionTree_Preview->setEnabled(b);

  m_ui->actionAutomatic->setEnabled(b);
  m_ui->actionAutomatic_Axon->setEnabled(b);

  m_ui->menuLoad_into->setEnabled(b);

  noMarkPunctaAction->setEnabled(b);
  markPunctaAction->setEnabled(b);
  //autoDetectPunctaAction->setEnabled(b);

  bnImportAction->setEnabled(b);
  swcImportAction->setEnabled(b);
  gtImportAction->setEnabled(b);
  btImportAction->setEnabled(b);
  connImportAction->setEnabled(b);
  punctaImportAction->setEnabled(b);

  swcExportAction->setEnabled(b);
  svgExportAction->setEnabled(b);
  vrmlExportAction->setEnabled(b);
  bnExportAction->setEnabled(b);
  nsExportAction->setEnabled(b);
  nsMultipleSwcExportAction->setEnabled(b);
  connExportAction->setEnabled(b);
  connFeatExportAction->setEnabled(b);
  chainSourceExportAction->setEnabled(b);
  punctaExportAction->setEnabled(b);

  //objectViewHideAction->setEnabled(b);
  objectViewNormalAction->setEnabled(b);
  objectViewSolidAction->setEnabled(b);
  objectViewSurfaceAction->setEnabled(b);
  objectViewSkeletonAction->setEnabled(b);

  settingAction->setEnabled(b);

  m_ui->actionMask_SWC->setEnabled(b);
  m_ui->actionOpen_3D_View_Without_Volume->setEnabled(b);
  m_ui->actionMask->setEnabled(b);

  m_ui->actionBinarize->setEnabled(b);
  m_ui->actionInvert->setEnabled(b);
  m_ui->actionExtract_Channel->setEnabled(b);
  m_ui->actionBinary_SWC->setEnabled(b);
}

void MainWindow::report(const std::string &title, const std::string &msg,
                        ZMessageReporter::EMessageType msgType)
{
  m_reporter->report(title, msg, msgType);
}

void MainWindow::initOpenglContext()
{
  // initGL requires a valid OpenGL context
  if (m_sharedContext != NULL) {
    // initialize OpenGL
    if (!Z3DApplication::app()->initializeGL()) {
      QString msg = Z3DApplication::app()->getErrorMessage();
      msg += ". 3D functions will be disabled.";
      report("OpenGL Initialization", msg.toStdString(),
             ZMessageReporter::Error);
    }

    if (NeutubeConfig::getInstance().isStereoEnabled()) {
      Z3DApplication::app()->setStereoSupported(m_sharedContext->format().stereo());
    } else {
      Z3DApplication::app()->setStereoSupported(false);
    }

    m_sharedContext->hide();
  }
}

bool MainWindow::okToContinue()
{
  if (isWindowModified()) {
    int r = QMessageBox::warning(this, tr("Stack"),
         tr("The stack has been modified.\n"
            "Do you want to save your changes?"),
         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (r == QMessageBox::Yes) {
      save();
    } else if (r == QMessageBox::Cancel) {
      return false;
    }
  }

  return true;
}

void MainWindow::checkViewAction(QAction *action)
{
  if (action != NULL) {
    action->setChecked(true);
  }

  QList<QAction*> viewActions = objectView->actions();
  for (QList<QAction*>::iterator iter = viewActions.begin();
       iter != viewActions.end(); ++iter) {
    if (*iter != action) {
      (*iter)->setChecked(false);
    }
  }
}

void MainWindow::takeScreenshot()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Screenshot"), m_lastOpenedFilePath,
                                     tr("Tiff stack files (*.tif) "), NULL/*,
                                     QFileDialog::DontUseNativeDialog*/);
    if (!fileName.isEmpty()) {
      m_lastOpenedFilePath = fileName;
      frame->takeScreenshot(fileName);
    }
  } else {
    report("No Active Frame", "Select one image first.",
           ZMessageReporter::Information);
    //QMessageBox::information(this, "No Active Frame", "Select one image first!");
  }
}

void MainWindow::updateViewMode(QAction *action)
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    if (action == m_ui->actionNormal) {
      frame->setViewMode(ZInteractiveContext::VIEW_NORMAL);
      //frame->presenter()->setMode(ZStackPresenter::NORMAL_MODE);
      /*
      frame->presenter()->interactiveContext().
          setViewMode(ZInteractiveContext::VIEW_NORMAL);
          */
      frame->updateView();
    } else if (action == m_ui->actionProject) {
      qDebug() << action->isChecked();
      frame->setViewMode(ZInteractiveContext::VIEW_PROJECT);
      frame->updateView();
    }
  }
}

void MainWindow::updateViewMenu(ZInteractiveContext::ViewMode viewMode)
{
  switch (viewMode) {
  case ZInteractiveContext::VIEW_NORMAL:
    m_ui->actionNormal->setChecked(true);
    break;
  case ZInteractiveContext::VIEW_PROJECT:
    m_ui->actionProject->setChecked(true);
    break;
  }
}

void MainWindow::viewObject(QAction *action)
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    if (action->isChecked()) {
      frame->showObject();
      checkViewAction(action);
      if (action == objectViewNormalAction) {
        frame->setObjectDisplayStyle(ZStackDrawable::NORMAL);
      } else if (action == objectViewSolidAction) {
        frame->setObjectDisplayStyle(ZStackDrawable::SOLID);
      } else if (action == objectViewSurfaceAction) {
        frame->setObjectDisplayStyle(ZStackDrawable::BOUNDARY);
      } else if (action == objectViewSkeletonAction) {
        frame->setObjectDisplayStyle(ZStackDrawable::SKELETON);
      }
    } else {
      frame->hideObject();
    }
  }
}

void MainWindow::stretchStackFrame(ZStackFrame *frame)
{
  if (frame != NULL) {
    frame->setSizeHintOption(NeuTube::SIZE_HINT_TAKING_SPACE);
    frame->resize(frame->sizeHint());
    frame->setSizeHintOption(NeuTube::SIZE_HINT_CURRENT_BEST);
  }
}

void MainWindow::presentStackFrame(ZStackFrame *frame)
{
  stretchStackFrame(frame);
  frame->show();
  updateMenu();
  //mdiArea->setActiveSubWindow(frame);
  getProgressDialog()->reset();
  frame->setViewInfo();

  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    frame->autoBcAdjust();

    if (!frame->document()->stackSourcePath().isEmpty()) {
      ZString sourcePath = frame->document()->stackSourcePath().toStdString();

      ZString suffix =
          ZBiocytinFileNameParser::getSuffix(ZBiocytinFileNameParser::ROI);

      sourcePath = sourcePath.dirPath() + ZString::FileSeparator +
          ZBiocytinFileNameParser::getCoreName(sourcePath);

      QFileInfo fileInfo((sourcePath + suffix + ".tif").c_str());
      if (!fileInfo.exists()) {
        fileInfo.setFile(
              (sourcePath + static_cast<const ZString&>(suffix).toLower() + ".tif").c_str());
      } else if (!fileInfo.exists()) {
        fileInfo.setFile(
              (sourcePath + static_cast<const ZString&>(suffix).toUpper() + ".tif").c_str());
      }

      if (fileInfo.exists()) {
        frame->loadRoi(fileInfo.absoluteFilePath());
      }
    }
  }
}

void MainWindow::openFile(const QString &fileName)
{
  ZFileType::EFileType fileType = ZFileType::fileType(fileName.toStdString());

  if (ZFileType::isNeutubeOpenable(fileType)) {
    m_progress->setRange(0, 3);

    int currentProgress = 0;
    m_progress->open();
    m_progress->setLabelText(QString("Loading " + fileName + " ..."));

   m_progress->setValue(++currentProgress);

    ZStackFrame *frame = NULL;
    if (ZFileType::isImageFile(fileType)) {
      frame = new ZStackFrame(mdiArea);
      connect(frame, SIGNAL(ready(ZStackFrame*)),
              this, SLOT(presentStackFrame(ZStackFrame*)));
      frame->createDocument();
      frame->show();
      frame->setViewInfo("Loading data ...");
    } else {
      frame = new ZStackFrame;
    }


    m_progress->setValue(++currentProgress);
    //qApp->processEvents();
    m_progress->show();
    //QApplication::processEvents();

    if (fileType == ZFileType::JSON_FILE) {
      frame->document()->setAdditionalSource(fileName.toStdString());
      m_progress->reset();
      frame->open3DWindow(this);
      delete frame;
    } else if (frame->readStack(fileName.toStdString().c_str()) == SUCCESS) {
      setCurrentFile(fileName);
      if (ZFileType::isImageFile(fileType)) {
        addStackFrame(frame, false);
      } else {
        m_progress->reset();
        frame->open3DWindow(this);
        delete frame;
      }
    } else {
      delete frame;
      m_progress->reset();
      reportFileOpenProblem(fileName);
    }

    //m_progress->reset();
  } else {
    reportFileOpenProblem(fileName, " unrecognized file name extension");
  }
}

void MainWindow::openTraceProject(QString fileName)
{
  if (!fileName.isEmpty()) {
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setLabelText(QString("Loading " + fileName + " ..."));
    m_progress->show();

    m_progress->setValue(25);
    ZStackFrame *frame = new ZStackFrame(NULL);
    fileName = QDir::cleanPath(fileName);
    QString projFilePath = fileName;
    if (projFilePath.endsWith(".trace")) {
      projFilePath += "/" + ZStackFrame::defaultTraceProjectFile();
    }

    if (frame->loadTraceProject(projFilePath.toLocal8Bit().constData(),
                                m_progress->findChild<QProgressBar*>())
      == 0) {
      setCurrentFile(fileName);
      addStackFrame(frame);
      m_progress->reset();
    } else {
      delete frame;
      m_progress->reset();
      reportFileOpenProblem(fileName);
    }
  }
}

void MainWindow::openTraceProject()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Project"),
                                   m_lastOpenedFilePath,
                                   tr("XML files (*.xml)"),
                                   NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    openTraceProject(fileName);
  }
}

void MainWindow::addStackFrame(Stack *stack, bool isOwner)
{
  if (stack != NULL) {
    ZStackFrame *frame = new ZStackFrame(mdiArea);
    frame->loadStack(stack, isOwner);
    addStackFrame(frame);
  }
}

void MainWindow::addStackFrame(ZStack *stack)
{
  if (stack != NULL) {
    ZStackFrame *frame = new ZStackFrame(mdiArea);
    frame->loadStack(stack);
    frame->setWindowTitle(stack->sourcePath());
    addStackFrame(frame);
  }
}

void MainWindow::addStackFrame(ZStackFrame *frame, bool /*isReady*/)
{
  QApplication::processEvents();

  if (!mdiArea->findChildren<ZStackFrame*>().contains(frame)) {
    mdiArea->addSubWindow(frame);
  }
  connect(frame, SIGNAL(infoChanged()), this, SLOT(updateStatusBar()));
  connect(frame, SIGNAL(closed(ZStackFrame*)), this, SLOT(updateMenu()));
  connect(frame, SIGNAL(closed(ZStackFrame*)),
    this, SLOT(updateFrameInfoDlg()));
  connect(frame, SIGNAL(closed(ZStackFrame*)),
    this, SLOT(removeStackFrame(ZStackFrame*)));
  connect(frame, SIGNAL(infoChanged()), this, SLOT(updateFrameInfoDlg()));
  connect(frame->document().get(), SIGNAL(stackDelivered(Stack*, bool)),
      this, SLOT(addStackFrame(Stack*, bool)));
  connect(frame->document().get(), SIGNAL(frameDelivered(ZStackFrame*)),
      this, SLOT(addStackFrame(ZStackFrame*)));
  connect(frame->document().get(), SIGNAL(stackModified()),
          this, SLOT(updateBcDlg()));
  connect(frame->presenter(), SIGNAL(viewModeChanged()),
          this, SLOT(updateMenu()));

  m_undoGroup->addStack(frame->document()->undoStack());

  int margin = 30;
  if (margin * m_frameCount * 2 > mdiArea->width() ||
      margin * m_frameCount * 2 > mdiArea->height()) {
    m_frameCount = 0;
  }

  if (mdiArea->subWindowList().size() == 1) {
    m_frameCount = 0;
  }

  QRect rect = frame->geometry();
  rect.moveTopLeft(QPoint(m_frameCount * margin, m_frameCount * margin));
  frame->setGeometry(rect);
  ++m_frameCount;

  //connect(frame, SIGNAL(presenterChanged()), this, SLOT(updateMenu()));
  //windowMenu->addAction(frame->windowMenuAction());
  //windowActionGroup->addAction(frame->windowMenuAction());

  //frame->show();
  //frame->raise();
  //mdiArea->setActiveSubWindow(frame);
}

void MainWindow::removeStackFrame(ZStackFrame *frame)
{
  if (frame != NULL) {
    //frame will be deleted automatically upon close event
    mdiArea->removeSubWindow(frame);
    m_undoGroup->removeStack(frame->document()->undoStack());
  }
}

void MainWindow::save()
{
  if (curFile.isEmpty()) {
    saveAs();
  } else {
    saveFile(curFile);
  }
}

void MainWindow::saveFile(const QString &fileName)
{
  setCurrentFile(fileName);
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->saveStack(fileName);
    frame->setWindowTitle(fileName);
  }
}

void MainWindow::saveAs()
{
  ZStackFrame *frame = currentStackFrame();

  if (frame != NULL) {
    QString fileName = QFileDialog::getSaveFileName(
          this, tr("Save stack"), frame->document()->stack()->sourcePath(),
          tr("Tiff stack files (*.tif) "), NULL/*,
          QFileDialog::DontUseNativeDialog*/);

    if (!fileName.isEmpty()) {
      m_lastOpenedFilePath = fileName;
      saveFile(fileName);
    }
  }
}

void MainWindow::exportSwc()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save SWC"), m_lastOpenedFilePath,
         tr("SWC files (*.swc) "));

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      frame->exportSwc(fileName.toLocal8Bit().constData());
    }
  }
}

void MainWindow::exportPuncta()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo) "));

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      frame->exportPuncta(fileName);
    }
  }
}

void MainWindow::exportNeuronStructure()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save neuron structure"),
                                 "./untitled.swc",
                                 tr("SWC files (*.swc) "));
  if (!fileName.isEmpty()) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      frame->exportSwcReconstruct(fileName);
    } else {
      report("Export Failed", "The frame is empty.", ZMessageReporter::Warning);
      /*
      QMessageBox::warning(this, tr("Export Failed"),
                           tr("Empty frame."), QMessageBox::Ok);
                           */

    }
  }
}

void MainWindow::exportVrml()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Export VRML"), "./untitled.vrml",
         tr("Vrml files (*.wrl) "));

  if (!fileName.isEmpty()) {
    ZStackFrame *frame = activeStackFrame();
    if (frame != NULL) {
      frame->exportVrml(fileName);
    }
  }
}

void MainWindow::exportBinary()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save tracing results"),
                                 m_lastOpenedFilePath,
                                 //tr("Binary files (*.tb)"), 0,
                                 QString(), 0,
                                 QFileDialog::DontConfirmOverwrite);

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = activeStackFrame();
    if (frame != NULL) {
      frame->exportTube(fileName);
    }
  }
}

void MainWindow::exportNeuronStructureAsMultipleSwc()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save neuron structure as multiple swcs"),
                                 "./untitled.swc",
                                 tr("SWC files (*.swc) "));
  if (!fileName.isEmpty()) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      int total = frame->exportSwcReconstruct(fileName, true);
      //int total = frame->document()->exportMultipleSwcTree(fileName);
      QMessageBox::information(this, tr("result"),
                               QString("%1 trees saved").arg(total));
    } else {
      QMessageBox::warning(this, tr("Export Failed"),
                           tr("Empty frame."), QMessageBox::Ok);

    }
  }
}

void MainWindow::exportChainFileList()
{
  QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save tracing results"), m_lastOpenedFilePath,
         tr("Binary files (*.tb)"), 0,
         QFileDialog::DontConfirmOverwrite);

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = activeStackFrame();
    if (frame != NULL) {
      frame->exportChainFileList(fileName);
    }
  }
}

void MainWindow::exportTubeConnection()
{
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save connection"), m_lastOpenedFilePath,
           tr("XML files (*.xml) "));

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = activeStackFrame();
    if (frame != NULL) {
      frame->exportChainConnection(fileName);
    }
  }
}

void MainWindow::exportTubeConnectionFeat()
{
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save connection feature"),
                                   m_lastOpenedFilePath,
                                   tr("Feature files (*.txt) "));

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = activeStackFrame();
    if (frame != NULL) {
      frame->exportChainConnectionFeat(fileName);
    }
  }
}

void MainWindow::exportSvg()
{
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save svg file"), "./untitled.svg",
           tr("Svg file (*.svg) "));

  if (!fileName.isEmpty()) {
    activeStackFrame()->document()->exportSvg(
          fileName.toLocal8Bit().constData());
  }
}

void MainWindow::exportTraceProject()
{
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save trace project"),
                                   "./untitled.xml",
                                   tr("XML file (*.xml) "));

  if (!fileName.isEmpty()) {
    TraceOutputDialog dlg;
    dlg.exec();
    activeStackFrame()->saveTraceProject(fileName, dlg.dir(), dlg.prefix());
  }
}

void MainWindow::importBinary()
{
  QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Import tracing results"), m_lastOpenedFilePath,
        tr("Binary files (*.tb)"),
        NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!files.isEmpty()) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      m_lastOpenedFilePath = files[0];
      frame->document()->importLocsegChain(files);
      frame->updateView();
    }
  }
}

void MainWindow::importSwc()
{
  QStringList files =
      QFileDialog::getOpenFileNames(this, tr("Import SWC files"), m_lastOpenedFilePath,
                                    tr("SWC files (*.swc)"),
                                    NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!files.isEmpty()) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      m_lastOpenedFilePath = files[0];
      frame->document()->importSwc(files);
      frame->updateView();
    } else {
      QMessageBox::warning(this, "Import failed.", "Null frame.");
    }
  }
}

void MainWindow::importPuncta()
{
  QStringList files =
      QFileDialog::getOpenFileNames(this, tr("Import Puncta files"), m_lastOpenedFilePath,
                                    tr("Puncta files (*.apo)"),
                                    NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!files.isEmpty()) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      m_lastOpenedFilePath = files[0];
      frame->document()->importPuncta(files);
      frame->updateView();
    } else {
      QMessageBox::warning(this, "Import failed.", "Null frame.");
    }
  }
}

void MainWindow::importGoodTube()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Good Tube"),
    ".", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    if (currentStackFrame() != NULL) {
      currentStackFrame()->document()->importGoodTube(dirpath.toLocal8Bit().constData());
      currentStackFrame()->updateView();
    }
  }
}

void MainWindow::importBadTube()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Bad Tube"),
    ".", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    if (currentStackFrame() != NULL) {
      currentStackFrame()->document()->importBadTube(dirpath.toLocal8Bit().constData());
      currentStackFrame()->updateView();
    }
  }
}

void MainWindow::importTubeConnection()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Import Connection"), m_lastOpenedFilePath,
        tr("XML files (*.xml)"), NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    if (currentStackFrame() != NULL) {
      m_lastOpenedFilePath = fileName;
      currentStackFrame()->document()->
          importLocsegChainConn(fileName.toLocal8Bit().constData());
      currentStackFrame()->updateView();
    }
  }
}

//void MainWindow::undo()
//{
//  ZStackFrame *frame = activeStackFrame();
//  if (activeStackFrame() != NULL) {
//    frame->undo();
//  }
//}

//void MainWindow::redo()
//{
//  ZStackFrame *frame = activeStackFrame();
//  if (activeStackFrame() != NULL) {
//    frame->redo();
//  }
//}

QString MainWindow::getOpenFileName(const QString &caption, const QString &filter)
{
  QString fileName =
      QFileDialog::getOpenFileName(this, caption, m_lastOpenedFilePath,
                                   filter,
                                   NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    QFileInfo fInfo(fileName);
    recordLastOpenPath(fInfo.absoluteDir().absolutePath());
  }

  return fileName;
}

QStringList MainWindow::getOpenFileNames(
    const QString &caption, const QString &filter)
{
  QStringList fileNameList =
      QFileDialog::getOpenFileNames(this, caption, m_lastOpenedFilePath,
                                   filter,
                                   NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileNameList.isEmpty()) {
    QFileInfo fInfo(fileNameList[0]);
    recordLastOpenPath(fInfo.absoluteDir().absolutePath());
  }

  return fileNameList;
}

QString MainWindow::getSaveFileName(const QString &caption, const QString &filter)
{
  QString fileName = QFileDialog::getSaveFileName(
        this, caption, m_lastOpenedFilePath, filter, NULL,
        /*QFileDialog::DontUseNativeDialog |*/ QFileDialog::DontConfirmOverwrite);

  if (!fileName.isEmpty()) {
    QFileInfo fInfo(fileName);
    recordLastOpenPath(fInfo.absoluteDir().absolutePath());
  }

  return fileName;
}

void MainWindow::importImageSequence()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image Sequence"),
                                   m_lastOpenedFilePath,
                                   tr("Image files (*.tif *.png)"),
                                   NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    ZStackFrame *frame = new ZStackFrame(mdiArea);

    //ZStackFrame *frame = new ZStackFrame;

    m_progress->open();
    m_progress->setRange(0, 2);
    m_progress->setLabelText(
          QString("Loading image sequence: " + fileName + " ..."));
    m_progress->show();
    int currentProgress = 0;
    m_progress->setValue(++currentProgress);

    qApp->processEvents();


    if (frame->importImageSequence(fileName.toStdString().c_str()) == SUCCESS) {
      addStackFrame(frame);
      presentStackFrame(frame);
    } else{
      delete frame;
    }

    m_progress->setValue(++currentProgress);
    m_progress->reset();
  }
  QApplication::processEvents();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (okToContinue()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

ZStackFrame* MainWindow::activeStackFrame()
{
  return dynamic_cast<ZStackFrame*>(mdiArea->activeSubWindow());
}

ZStackFrame* MainWindow::currentStackFrame()
{
  return dynamic_cast<ZStackFrame*>(mdiArea->currentSubWindow());
}

void MainWindow::setCurrentFile(const QString &fileName)
{
  curFile = fileName;
  setWindowModified(false);
  QString shownName = tr("Untitled");
  if (!curFile.isEmpty()) {
    shownName = strippedName(curFile);
    recentFiles.removeAll(curFile);
    recentFiles.prepend(curFile);
    updateRecentFileActions();
  }

  //setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Tiff")));
}

void MainWindow::createAutoSaveDir()
{
  if (NeutubeConfig::getInstance().isAutoSaveEnabled()) {
    std::cout << "Create autsave dir" << std::endl;

    QString autoSaveDir =
        NeutubeConfig::getInstance().getPath(NeutubeConfig::AUTO_SAVE).c_str();

    std::cout << autoSaveDir.toStdString() << std::endl;

    QDir dir(autoSaveDir);
    if (!dir.exists()) {
      StartSettingDialog dlg;
      dlg.setAutoSaveDir(autoSaveDir);
      if (dlg.exec()) {
        QDir dir(dlg.getAutosaveDir());
        NeutubeConfig::getInstance().setAutoSaveDir(
              dir.absolutePath().toStdString());
        if (!dir.exists()) {
          if (!dir.mkpath(dir.absolutePath())) {
            report("Faile to Create Autosave Directory",
                   "Cannot create " + dlg.getAutosaveDir().toStdString() +
                   "Autosave will be disabled.",
                   ZMessageReporter::Information);
          }
        }

      }
    }
  }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::updateRecentFileActions()
{
  QMutableStringListIterator i(recentFiles);
  while (i.hasNext()) {
    if (!QFile::exists(i.next())) {
      i.remove();
    }
  }

  for (int j = 0; j < MaxRecentFiles; ++j) {
    if (j < recentFiles.count()) {
      QString text = tr("&%1 %2").arg(j+1).arg(recentFiles[j]);
      recentFileActions[j]->setText(text);
      recentFileActions[j]->setData(recentFiles[j]);
      recentFileActions[j]->setVisible(true);
    } else {
      recentFileActions[j]->setVisible(false);
    }
  }

  separatorAction->setVisible(!recentFiles.isEmpty());
}

void MainWindow::openRecentFile()
{
  //dynamically cast sender() to a QAction
  QAction *action = qobject_cast<QAction *>(sender());
  if (action) {
    QString filePath = action->data().toString();
    if (filePath.endsWith(".xml") || filePath.endsWith(".trace")
      || filePath.endsWith(".trace/")) {
      openTraceProject(filePath);
    } else {
      m_lastOpenedFilePath = filePath;
      openFile(filePath);
    }
  }
}

void MainWindow::about()
{
  QString title = "<h2>neuTube</h2>";
  if (!NeutubeConfig::getInstance().getApplication().empty()) {
    title += QString("<p>") +
        NeutubeConfig::getInstance().getApplication().c_str() + " Edition" +
        "</p>";
  }
  QMessageBox::about(this, tr("About neuTube"),
                     title +
                     "<p>neuTube is a software application "
                     "for neuron tracing and visualization. "
                     "It was originally developed by Ting Zhao "
                     "in Myers Lab "
                     "at Howard Hughes Medical Institute, Janelia Farm Research Campus.</p>"
                     "<p>Current developers: </p>"
                     "<ul>"
                     "<li>Ting Zhao</li>"
                     "<p>Howard Hughes Medical Institute, Janelia Farm Research Campus, "
                     "Ashburn, VA 20147</p>"
                     "<li>Linqing Feng</li>"
                     "<p>Jinny Kim's Lab, Center for Functional Connectomics, KIST, Korea</p>"
                     "</ul>"
                     "<p>The Software is provided \"as is\" without warranty of any kind, "
                     "either express or implied, including without limitation any implied "
                     "warranties of condition, uniterrupted use, merchantability, fitness "
                     "for a particular purpose, or non-infringement.</p>"
                     "<p>For any regarded question or feedback, please mail to "
                     "<a href=mailto:tingzhao@gmail.com>tingzhao@gmail.com</a></p>"
                     );
}

void MainWindow::writeSettings()
{
  QSettings settings("Janelia Farm", "neuTube");

  settings.setValue("lastPath", m_lastOpenedFilePath);
  settings.setValue("geometry", saveGeometry());
  settings.setValue("recentFiles", recentFiles);
  settings.setValue("autoSaveDir", QString(NeutubeConfig::getInstance().
                    getPath(NeutubeConfig::AUTO_SAVE).c_str()));
}

void MainWindow::readSettings()
{
  std::cout << "Read settings ..." << std::endl;
  QSettings settings("Janelia Farm", "neuTube");
  restoreGeometry(settings.value("geometry").toByteArray());
  recentFiles = settings.value("recentFiles").toStringList();
  m_lastOpenedFilePath = settings.value("lastPath").toString();
  updateRecentFileActions();

  if (settings.contains("autoSaveDir")) {
    NeutubeConfig::getInstance().setAutoSaveDir(
          settings.value("autoSaveDir").toString().toStdString());
  }
}

void MainWindow::activateInteractiveTrace(QAction *action)
{
  ZStackFrame *frame = currentStackFrame();

  if (frame != NULL) {
    if (action == m_ui->actionTree_Preview) {
      /*
      frame->presenter()->interactiveContext().
          setTraceMode(ZInteractiveContext::TRACE_PREIVEW_RECONSTRUCTION);
      frame->document()->updatePreviewSwc();
      frame->updateView();
      */
    } else {
      bool switchedFromPreview = false;
      if (frame->presenter()->interactiveContext().isReconPreview()) {
        switchedFromPreview = true;
      }
      if (action == noTraceAction) {
        frame->presenter()->interactiveContext().
            setTraceMode(ZInteractiveContext::TRACE_OFF);
        frame->presenter()->updateLeftMenu();
      } else if (action == fitsegAction) {
        frame->presenter()->interactiveContext().
            setTraceMode(ZInteractiveContext::TRACE_SINGLE);
        frame->presenter()->updateLeftMenu();
      } else if (action == traceTubeAction) {
        frame->presenter()->interactiveContext().
            setTraceMode(ZInteractiveContext::TRACE_TUBE);
        frame->presenter()->updateLeftMenu();
      }
      if (switchedFromPreview) {
        frame->updateView();
      }
    }
  }
}

void MainWindow::activateInteractiveMarkPuncta(QAction *action)
{
  ZStackFrame *frame = currentStackFrame();

  if (frame != NULL) {
    if (action == noMarkPunctaAction) {
      frame->presenter()->interactiveContext().
          setMarkPunctaMode(ZInteractiveContext::MARK_PUNCTA_OFF);
      frame->presenter()->updateLeftMenu();
    } else if (action == markPunctaAction) {
      frame->presenter()->interactiveContext().
          setMarkPunctaMode(ZInteractiveContext::MARK_PUNCTA);
      frame->presenter()->updateLeftMenu();
    }
  }
}

void MainWindow::buildConn()
{
  if (currentStackFrame() != NULL) {
    currentStackFrame()->document()->buildLocsegChainConn();
    currentStackFrame()->updateView();
  }
}

void MainWindow::manageObjs()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    frame->showManageObjsDialog();
  }
}

void MainWindow::binarize()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->pushBinarizeCommand();
  }
}

void MainWindow::bwsolid()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->pushBwsolidCommand();
    //currentStackFrame()->presenter()->solidifyStack();
    //currentStackFrame()->updateView();
    /*
    QUndoCommand *cmd = new ZStackDocBwSolidCommand(
          currentStackFrame()->document().get());
    currentStackFrame()->pushUndoCommand(cmd);
    */
    //currentStackFrame()->undoStack()->push(cmd);
  }
}

void MainWindow::enhanceLine()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    m_progress->setRange(0, 0);
    m_progress->setLabelText(tr("Enhance line structure ..."));
    m_progress->show();

    frame->pushEnhanceLineCommand();

    /*
    QUndoCommand *cmd = new ZStackDocEnhanceLineCommand(
          currentStackFrame()->document().get());
    currentStackFrame()->pushUndoCommand(cmd);
    */
    //currentStackFrame()->document()->enhanceLine();
    m_progress->reset();
    //currentStackFrame()->updateView();
  }
}

void MainWindow::setOption()
{
  if (activeStackFrame() != NULL) {
    activeStackFrame()->showSetting();
  }
}

int MainWindow::frameNumber()
{
  if (mdiArea == NULL) {
    return 0;
  } else {
    return mdiArea->subWindowList().size();
  }
}

void MainWindow::showFrameInfo()
{
  if (m_frameInfoDlg.isVisible() == true) {
    m_frameInfoDlg.raise();
  } else {
    m_frameInfoDlg.show();
    updateFrameInfoDlg();
  }
}

void MainWindow::updateFrameInfoDlg()
{
  if (m_frameInfoDlg.isVisible() == true) {
    if (currentStackFrame() != NULL) {
      if (!currentStackFrame()->isClosing()) {
        QStringList infoList = currentStackFrame()->toStringList();
        QString frameInfo;
        for (QStringList::const_iterator line = infoList.begin();
        line < infoList.end(); ++line) {
          frameInfo += "<p>" + *line + "</p>";
        }
        m_frameInfoDlg.setText(frameInfo);
        m_frameInfoDlg.setCurve(
            currentStackFrame()->curveToPlot(m_frameInfoDlg.plotSettings(),
                m_frameInfoDlg.curveOption()));
        m_frameInfoDlg.updatePlotSettings();
      }
    } else {
      if (frameNumber() == 0) {
        m_frameInfoDlg.setText(tr("Nothing exists. "
                                  "This dialog is to show you information "
                                  "about your document."));
      } else {
        m_frameInfoDlg.setText(QString("%1 ghost(s)").arg(frameNumber()));
      }
    }
  }
}

void MainWindow::updateActiveUndoStack()
{
  if (activeStackFrame() != NULL) {
    activeStackFrame()->document()->undoStack()->setActive(true);
  } else if (currentStackFrame() != NULL) {
    currentStackFrame()->document()->undoStack()->setActive(false);
  }
}



void MainWindow::on_action3DView_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->open3DWindow(frame);
  }
}

void MainWindow::on_actionBinarize_triggered()
{
  binarize();
}

void MainWindow::on_actionSolidify_triggered()
{
  bwsolid();
}

void MainWindow::bcAdjust()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    for (int i = 0; i < m_bcDlg->getMaxNumOfChannel(); i++) {
      frame->setBc(m_bcDlg->greyScale(i), m_bcDlg->greyOffset(i), i);
    }
    frame->updateView();
  }
}

void MainWindow::autoBcAdjust()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->autoBcAdjust();
    /*
    frame->document()->startProgress();
    for (int i = 0; i < frame->document()->stackChannelNumber(); i++) {
      frame->document()->advanceProgress(0.1);
      double lower, upper;
      ZStackStatistics::getGreyMapHint(*frame->document()->stack(), i,
                                       &lower, &upper);
      frame->document()->advanceProgress(0.2);
      m_bcDlg->setValue(lower, upper, i);
      frame->setBc(m_bcDlg->greyScale(i), m_bcDlg->greyOffset(i), i);
    }
    frame->document()->endProgress();

    frame->updateView();
    */
  }
}

void MainWindow::updateBcDlg(const ZStackFrame *frame)
{
  if (frame != NULL) {
    int nChannel = frame->presenter()->buddyDocument()->stack()->channelNumber();
    m_bcDlg->setNumOfChannel(nChannel);
    for (int i=0; i<std::min(nChannel, m_bcDlg->getMaxNumOfChannel()); i++) {
      m_bcDlg->setRange(frame->document()->stack()->min(i),
                        frame->document()->stack()->max(i), i);
      qDebug() << frame->document()->stack()->min(i) <<
                  ' ' << frame->document()->stack()->max(i) << "\n";

      m_bcDlg->setValue(iround(frame->displayGreyMin(i)),
                        iround(frame->displayGreyMax(i)), i);

      qDebug() << frame->displayGreyMin(i) << ' ' <<
                  iround(frame->displayGreyMax(i)) << "\n";
    }
  }
}

void MainWindow::on_actionOpen_triggered()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open stack"),
        m_lastOpenedFilePath,
        tr("Stack files (*.tif *.lsm *.raw *.png *.swc *.nnt *.apo *.marker *.json)"),
        NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = QFileInfo(fileName).absoluteDir().path();
    openFile(fileName);
  }
}

void MainWindow::on_actionEdit_Swc_triggered()
{
  if (currentStackFrame() != NULL && currentStackFrame()->document()->hasSwcList()) {
    ZEditSwcDialog editswcDialog(this, currentStackFrame()->document()->swcList());
    editswcDialog.exec();
  } else {
    ZEditSwcDialog editswcDialog(this);
    editswcDialog.exec();
  }
}

void MainWindow::on_actionRescale_Swc_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ZRescaleSwcDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      ZRescaleSwcSetting setting = dlg.getRescaleSetting();
      frame->executeSwcRescaleCommand(setting);
    }
  }

    /*else {
    QMessageBox::warning(this, tr("Current document don't have swc tree"),
                         tr("Current document don't have swc tree."), QMessageBox::Ok);
  }*/
}

void MainWindow::on_actionEnhance_Line_triggered()
{
  enhanceLine();
}

void MainWindow::on_actionDisable_triggered()
{

}

void MainWindow::on_actionAutomatic_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->executeAutoTraceCommand();
    //currentStackFrame()->view()->progressBar()->setValue(0);

    /*
    ZStackDocAutoTraceCommand *atcommand = new ZStackDocAutoTraceCommand(currentStackFrame()->document().get(),
                                                                         currentStackFrame()->view()->progressBar());
*/
//currentStackFrame()->pushUndoCommand(atcommand);
    //currentStackFrame()->presenter()->autoTrace();
    //currentStackFrame()->updateView();
  }
}

void MainWindow::on_actionAutomatic_Axon_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->executeAutoTraceAxonCommand();
  }
#if 0
  if (currentStackFrame() != NULL) {
    currentStackFrame()->view()->progressBar()->setValue(0);
    ZStackDocAutoTraceAxonCommand *atcommand = new ZStackDocAutoTraceAxonCommand(currentStackFrame()->document().get(),
                                                                         currentStackFrame()->view()->progressBar());
    currentStackFrame()->pushUndoCommand(atcommand);
    //currentStackFrame()->presenter()->autoTrace();
    //currentStackFrame()->updateView();
  }
#endif

}

void MainWindow::on_actionUpdate_triggered()
{
  if (currentStackFrame() != NULL) {
    currentStackFrame()->document()->updateStackFromSource();
    qDebug() << "Updating slider\n";
    currentStackFrame()->view()->updateSlider();
    currentStackFrame()->updateView();
  }
}

void MainWindow::on_actionRefine_Ends_triggered()
{
  if (currentStackFrame() != NULL) {
    currentStackFrame()->document()->refineLocsegChainEnd();
    currentStackFrame()->updateView();
  }
}

void MainWindow::on_actionRemove_Small_triggered()
{
  if (currentStackFrame() != NULL) {
    bool ok;
    double thre = QInputDialog::getDouble(this, tr("Length threshold for small tube"),
                                          tr("Length (in pixel) threshold:"), 100, 0, 100000, 2, &ok);
    if (ok) {
      QUndoCommand *removesmallcommand =
          new ZStackDocCommand::TubeEdit::RemoveSmall(
            currentStackFrame()->document().get(), thre);
      currentStackFrame()->pushUndoCommand(removesmallcommand);
    }
  }
}

void MainWindow::on_actionProject_triggered()
{
}

void MainWindow::on_actionBrightnessContrast_triggered()
{
  m_bcDlg->show();
  m_bcDlg->raise();

  updateBcDlg(currentStackFrame());
}

void MainWindow::updateBcDlg()
{
  if (m_bcDlg->isVisible()) {
    updateBcDlg(currentStackFrame());
  }
}

void MainWindow::on_actionAbout_iTube_triggered()
{
  about();
}

void MainWindow::on_actionManual_triggered()
{
  QStringList args;
  args.append(QApplication::applicationDirPath() + "/../../itube_manual.pdf");
  qDebug() << args << '\n';
  qDebug() << QProcess::execute("/usr/bin/open", args)<< '\n';
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void MainWindow::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();

  QString file = urls.first().toLocalFile();

  if (file.endsWith(".xml") || file.endsWith(".trace")
    || file.endsWith(".trace/")) {
    openTraceProject(file);
  } else {
    m_lastOpenedFilePath = QFileInfo(file).absoluteDir().path();
    openFile(file);
  }
}

void MainWindow::on_actionSave_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    if (frame->isReadyToSave()) {
      frame->saveProject();
    } else {
      QString dataFile = frame->document()->stack()->sourcePath();
      if (dataFile.endsWith("/")) {
        dataFile.truncate(dataFile.length() - 1);
      }
      dataFile.truncate(dataFile.lastIndexOf('/'));

      qDebug() << dataFile;

      QString fileName =
          QFileDialog::getSaveFileName(this, tr("Save tracing project"),
                                       dataFile + "/untitled.trace",
                                       tr("Tracing project"), 0);

      qDebug() << fileName;

      if (!fileName.isEmpty()) {
        frame->saveProjectAs(fileName);
      }
    }
  }
}

void MainWindow::on_actionLoad_triggered()
{
  QString dirpath =/*
      QFileDialog::getExistingDirectory(this, tr("Tracing Project"),
                                        ".", QFileDialog::ShowDirsOnly);*/
#if defined __APPLE__
      QFileDialog::getOpenFileName(this, tr("Tracing Project"), m_lastOpenedFilePath,
                                   "Tracing project (*.trace)");
#else
      QFileDialog::getOpenFileName(this, tr("Tracing Project"), m_lastOpenedFilePath,
                               "Tracing project (*.xml)");
#endif

  if (!dirpath.isEmpty()) {
    m_lastOpenedFilePath = dirpath;
    QString projectFile = dirpath;
#if defined __APPLE__
    projectFile += "/" + ZStackFrame::defaultTraceProjectFile();
#endif
    if (QFile(projectFile).exists()) {
      m_progress->setRange(0, 100);
      m_progress->setLabelText(QString("Loading " + dirpath + " ..."));
      m_progress->show();

      m_progress->setValue(25);
      ZStackFrame *frame = new ZStackFrame;

      qDebug() << projectFile;

      if (frame->loadTraceProject(projectFile.toLocal8Bit().constData(),
                                  m_progress->findChild<QProgressBar*>())
        == 0) {
        setCurrentFile(dirpath);
        addStackFrame(frame);
        m_progress->reset();
      } else {
        report("Open Failed", "The file cannot be open.",
               ZMessageReporter::Warning);
        /*
        QMessageBox::warning(this, tr("Open Failed"),
                             tr("The file cannot be open."), QMessageBox::Ok);
*/
        delete frame;
        m_progress->reset();
      }
    }
  }
}


void MainWindow::on_actionAdd_Reference_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    QStringList pathList =
        QFileDialog::getOpenFileNames(this, tr("Add SWC Reference"), m_lastOpenedFilePath,
                                     "Swc files (*.swc)",
                                      NULL/*, QFileDialog::DontUseNativeDialog*/);
    if (!pathList.isEmpty()) {
      m_lastOpenedFilePath = pathList[0];
      frame->importSwcAsReference(pathList);
      frame->updateView();
    }
  }
}

void MainWindow::on_actionFrom_SWC_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->document()->traceFromSwc();
    frame->updateView();
  }
}

void MainWindow::on_actionSave_As_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    QString dataFile = frame->document()->stack()->sourcePath();
    if (dataFile.endsWith("/")) {
      dataFile.truncate(dataFile.length() - 1);
    }
    dataFile.truncate(dataFile.lastIndexOf('/'));

    qDebug() << dataFile;

    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save tracing project"),
                                     dataFile + "/untitled.trace",
                                     tr("Tracing project"), 0);

    if (!fileName.isEmpty()) {
      frame->saveProjectAs(fileName);
    }
  }
}

void MainWindow::on_actionLoad_from_a_file_triggered()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Project"), m_lastOpenedFilePath,
                                   tr("XML files (*.xml)"));
  m_lastOpenedFilePath = fileName;
  openTraceProject(fileName);
}

void MainWindow::on_actionAutoMerge_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->document()->mergeAllChain();
    frame->updateView();
  }
}

void MainWindow::on_actionExtract_Channel_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ChannelDialog dlg(NULL, frame->document()->stack()->channelNumber());
    if (dlg.exec() == QDialog::Accepted) {
      m_progress->setRange(0, 100);
      m_progress->setLabelText(QString("Extracing channel ..."));
      m_progress->show();

      m_progress->setValue(25);

      int channel = dlg.channel();
      Stack *stack = frame->document()->stack()->copyChannel(channel);
      if (stack != NULL) {
        m_progress->setRange(0, 100);
        m_progress->setLabelText(QString("Extracting Channel %1 ...").arg(channel));
        m_progress->show();
        ZStackFrame *nframe = new ZStackFrame;
        nframe->loadStack(stack, true);
        nframe->document()->stack()->setSource(
              frame->document()->stack()->sourcePath(), channel);
        QString src(frame->document()->stack()->sourcePath());
        src += QString("_channel_%1").arg(channel+1);
        nframe->setWindowTitle(src);
        addStackFrame(nframe);
        presentStackFrame(nframe);
      }

      m_progress->reset();
    }
  }
}

void MainWindow::on_actionSave_Stack_triggered()
{
  saveAs();
}

void MainWindow::on_actionWatershed_triggered()
{
  ZStackFrame *frame = currentStackFrame();

  if (frame != NULL) {
    frame->executeWatershedCommand();
  }
#if 0
  if (currentStackFrame() != NULL) {
    //currentStackFrame()->document()->watershed();
    //currentStackFrame()->updateView();
    QUndoCommand *cmd = new ZStackDocWatershedCommand(currentStackFrame()->document().get());
    currentStackFrame()->pushUndoCommand(cmd);
  }
#endif
}

void MainWindow::on_actionCanny_Edge_triggered()
{
#if defined(_USE_ITK_)
  if (currentStackFrame() != NULL) {
    ZStackProcessor proc;
    CannyEdgeDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      proc.cannyEdge(currentStackFrame()->document()->stack(),
                     dlg.variance(), dlg.lowerThreshold(),
                     dlg.upperThreshold());
      currentStackFrame()->updateView();
    }
  }
#else
  this->m_ui->actionCanny_Edge->setDisabled(true);
#endif
}

void MainWindow::connectedThreshold(int x, int y, int z)
{
#if defined(_USE_ITK_)
  ZStackProcessor proc;
  ConnectedThresholdDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {

    proc.connectedThreshold(currentStackFrame()->document()->stack(),
                            x, y, z,
                            dlg.lowerThreshold(), dlg.upperThreshold());
    currentStackFrame()->updateView();
  }
  disconnect(currentStackFrame()->presenter(),
             SIGNAL(mousePositionCaptured(int,int,int)),
             this, SLOT(connectedThreshold(int, int, int)));
#else
  UNUSED_PARAMETER(x);
  UNUSED_PARAMETER(y);
  UNUSED_PARAMETER(z);
#endif
}

void MainWindow::on_actionConnected_Threshold_triggered()
{
#if defined(_USE_ITK_)
  if (currentStackFrame() != NULL) {
    connect(currentStackFrame()->presenter(),
            SIGNAL(mousePositionCaptured(int,int,int)),
            this, SLOT(connectedThreshold(int, int, int)), Qt::DirectConnection);
    currentStackFrame()->presenter()->enterMouseCapturingMode();
  }
#else
  this->m_ui->actionConnected_Threshold->setDisabled(true);
#endif
}

void MainWindow::on_actionMedian_Filter_triggered()
{
#if defined(_USE_ITK_)
  if (currentStackFrame() != NULL) {
    ZStackProcessor proc;

    MedianFilterDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      proc.medianFilter(currentStackFrame()->document()->stack(), dlg.radius());
      currentStackFrame()->updateView();
    }
  }
#else
  this->m_ui->actionMedian_Filter->setDisabled(true);
#endif
}

void MainWindow::on_actionDistance_Map_triggered()
{
  if (currentStackFrame() != NULL) {
    ZStackProcessor proc;

    DistanceMapDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      if (currentStackFrame()->document()->stack()->isBinary()) {
        proc.distanceTransform(currentStackFrame()->document()->stack(),
                               dlg.isSquared(), dlg.isSliceWise());
        currentStackFrame()->document()->notifyStackModified();
        currentStackFrame()->updateView();
      }
    }
  }
}

void MainWindow::on_actionShortest_Path_Flow_triggered()
{
  if (currentStackFrame() != NULL) {
    ZStackProcessor proc;

    if (currentStackFrame()->document()->stack()->isBinary()) {
      proc.shortestPathFlow(currentStackFrame()->document()->stack());
      currentStackFrame()->document()->notifyStackModified();
      currentStackFrame()->updateView();
    }
  }
}

void MainWindow::on_actionExpand_Region_triggered()
{
  if (currentStackFrame() != NULL) {
    ZStackProcessor proc;

    RegionExpandDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      if (currentStackFrame()->document()->stack()->isBinary()) {
        proc.expandRegion(currentStackFrame()->document()->stack(),
                          dlg.getRadius());
        currentStackFrame()->document()->notifyStackModified();
        currentStackFrame()->updateView();
      }
    }
  }
}

void MainWindow::on_actionDilate_triggered()
{

}

void MainWindow::on_actionExtract_Neuron_triggered()
{
  NeuronIdDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {
    ZSegmentMapArray segmentMapArray;
    segmentMapArray.load("/home/zhaot/Work/neutube/neurolabi/data/"
                         "ting_example_stack/segment_to_body_map.txt");
    //segmentMapArray.print();
    int bodyId = dlg.getNeuronId();
    std::vector<int> segments =
        segmentMapArray.bodyToSegment(bodyId);

    for (size_t i = 0; i < segments.size(); i++) {
      std::cout << segments[i] << std::endl;
    }

    if (segments.size() > 1) {
      std::ostringstream outDirStream;
      outDirStream << "/home/zhaot/Work/neutube/neurolabi/data/flyem/"
                   << bodyId;

      if (!QDir().exists(QString(outDirStream.str().c_str()))) {
        QDir().mkpath(outDirStream.str().c_str());
      }

      /* Generate XML file*/
      std::ofstream xmlStream((outDirStream.str() + ".xml").c_str());
      xmlStream << "<xml>" << std::endl <<
                   "<data>" << std::endl <<
                   "<image type=\"dir\">" << std::endl <<
                   "<url>" << outDirStream.str() << "</url>" << std::endl <<
                   "<ext>tif</ext>" << std::endl <<
                   "</image>" << std::endl <<
                   "</data>" << std::endl <<
                   "</xml>" <<
                   std::endl;
      xmlStream.close();

      for (int planeId = 611; planeId <= 710; planeId++) {
        std::cout << "Plane ID: " << planeId << std::endl;

        ZSuperpixelMapArray superpixelMapArray;
        superpixelMapArray.load("/home/zhaot/Work/neutube/neurolabi/data/"
                                "ting_example_stack/superpixel_to_segment_map.txt",
                                planeId);

        //superpixelMapArray.print();

        std::vector<int> superpixel =
            superpixelMapArray.segmentToSuperpixel(segments);

        std::cout << "Superpixels:" << std::endl;
        for (size_t i = 0; i < superpixel.size(); i++) {
          std::cout << superpixel[i] << std::endl;
        }

        if (superpixel.size() > 0) {
          std::ostringstream filePathStream;
          filePathStream << "/home/zhaot/Work/neutube/neurolabi/data/"
                            "ting_example_stack/superpixel_maps/sp_map.00"
                         << planeId << ".png";

          std::string filePath = filePathStream.str();

          std::cout << filePath << std::endl;

          Stack *stack = Read_Stack_U(filePath.c_str());

          Stack *mask = Make_Stack(GREY, stack->width, stack->height,
                                   stack->depth);
          Zero_Stack(mask);

          for (size_t i = 0; i < superpixel.size(); i++) {
            uint16_t *array16 = (uint16_t*) stack->array;
            size_t nvoxel = Stack_Voxel_Number(stack);
            for (size_t k = 0; k < nvoxel; k++) {
              if (array16[k] == superpixel[i]) {
                mask->array[k] = 1;
              }
            }
          }

          Kill_Stack(stack);

          std::ostringstream filePathStream2;
          filePathStream2 << outDirStream.str() <<
                             "/neuron.00" << planeId << ".tif";

          std::cout << filePathStream2.str() << std::endl;

          Write_Stack((char*) filePathStream2.str().c_str(), mask);
          Kill_Stack(mask);
        }
      }

      Stack_Document *doc =
          Xml_Read_Stack_Document((outDirStream.str() + ".xml").c_str());
      File_List *list = (File_List*) doc->ci;
      Print_File_List(list);

      Stack *stack = Read_Image_List_Bounded(list);
      Stack *out = Stack_Region_Expand(stack, 8, 1, NULL);
      Kill_Stack(stack);
      stack = Downsample_Stack(out, dlg.getDownsampleRate() - 1,
                             dlg.getDownsampleRate() - 1, 0);
      Write_Stack((char*) (outDirStream.str() + "_ds.tif").c_str(), stack);
    } //if (segments.size() > 1)
  }
}

void MainWindow::on_actionSkeletonization_triggered()
{
  FlyEmSkeletonizationDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      ZStack *stack = frame->document()->stack();
      //stack->binarize();

      Stack *stackData = stack->c_stack();

      ZStackSkeletonizer skeletonizer;
      skeletonizer.setProgressReporter(frame->document()->getProgressReporter());
      skeletonizer.setRebase(true);
      if (dlg.isExcludingSmallObj()) {
        skeletonizer.setMinObjSize(dlg.sizeThreshold());
      } else {
        skeletonizer.setMinObjSize(0);
      }

      double distThre = -1.0;
      if (!dlg.isConnectingAll()) {
        distThre = dlg.distanceThreshold();
      }
      skeletonizer.setDistanceThreshold(distThre);

      skeletonizer.setLengthThreshold(dlg.lengthThreshold());
      skeletonizer.setKeepingSingleObject(dlg.isKeepingShortObject());

      if (dlg.isLevelChecked()) {
        skeletonizer.setLevel(dlg.level());
      }

      ZSwcTree *wholeTree = skeletonizer.makeSkeleton(stackData);

      if (wholeTree != NULL) {
        frame->executeAddObjectCommand(wholeTree, NeuTube::Documentable_SWC);
        frame->open3DWindow(this, Z3DWindow::EXCLUDE_VOLUME);
      } else {
        report("Skeletonization failed", "No SWC tree generated.",
               ZMessageReporter::Error);
      }
    }
  }
}

void MainWindow::on_actionPixel_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ParameterDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      ZString str(dlg.parameter());
      std::vector<int> pixelValue = str.toIntegerArray();

      if (pixelValue.size() == 0) {
        return;
      }

      ZStack *stack = frame->document()->stack();

      int x, y, z;
      bool found = false;
      for (z = 0; z < stack->depth(); z++) {
        for (y = 0; y < stack->height(); y++) {
          for (x = 0; x < stack->width(); x++) {
            for (int c = 0; c < stack->channelNumber(); c++) {
              if (c < (int) pixelValue.size()) {
                if (stack->value(x, y, z, c) == pixelValue[c]) {
                  found = true;
                } else {
                  found = false;
                  break;
                }
              }
            }

            if (found) {
              break;
            }
          }
          if (found) {
            break;
          }
        }
        if (found) {
          break;
        }
      }

      if (found) {
        //ZCircle *circle = new ZCircle(x, y, z, 10);
        //frame->presenter()->addDecoration(circle);
        frame->setViewPortCenter(x, y, z);
        //currentStackFrame()->updateView();
      } else {
        std::cout << "Cannot find any voxel with the specified value."
                  << std::endl;
      }
    }
  }
}

void MainWindow::test()
{
#if !defined(__MINGW32__)
  ZTest::test(this);
#endif
  statusBar()->showMessage(tr("Test done."));
}

void MainWindow::evokeStackFrame(QMdiSubWindow *frame)
{
#ifdef _DEBUG_
  std::cout << "frame evoked: " << frame << std::endl;
#endif

#ifdef __APPLE__
  if (frame != NULL) {
    frame->hide();
    QApplication::processEvents();
    qDebug() << "process 1";
    frame->show();
    QApplication::processEvents();
    qDebug() << "process 2";
    mdiArea->setActiveSubWindow(frame);
  }
#endif

  ZStackFrame *targetFrame = dynamic_cast<ZStackFrame*>(frame);

  QList<QMdiSubWindow *> frameList = mdiArea->subWindowList();
  for (QList<QMdiSubWindow *>::iterator iter = frameList.begin();
       iter != frameList.end(); ++iter) {
    ZStackFrame *frameIter = dynamic_cast<ZStackFrame*>(*iter);
    if (frameIter != NULL) {
      if (frameIter != targetFrame) {
        frameIter->displayActiveDecoration(false);
      }
    }
  }

  if (targetFrame != NULL) {
    targetFrame->displayActiveDecoration(true);
  }
}

void MainWindow::on_actionImport_Network_triggered()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Network"),
                                   m_lastOpenedFilePath,
                                   tr("FlyEM network files (*.txt)"));

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;
    FlyEm::ZNeuronNetwork flyemNetwork;
    flyemNetwork.import(fileName.toStdString());
    flyemNetwork.layoutSwc();
    ZSwcNetwork *network = flyemNetwork.toSwcNetwork();

    ZStackFrame *frame = new ZStackFrame;
    frame->document()->appendSwcNetwork(*network);
    delete network;

    //addStackFrame(frame);
    frame->open3DWindow(this);
    delete frame;

    QApplication::processEvents(); //force file dialog to close.
                                   //might be a bug in qt
  }
}

void MainWindow::on_actionAddSWC_triggered()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    QStringList fileList =
        QFileDialog::getOpenFileNames(this, tr("Load SWC files"),
                                      m_lastOpenedFilePath,
                                      tr("SWC files (*.swc)"),
                                      NULL/*, QFileDialog::DontUseNativeDialog*/);
    if (!fileList.isEmpty()) {
      frame->load(fileList);
      if (NeutubeConfig::getInstance().getMainWindowConfig().
          isExpandSwcWith3DWindow()) {
        frame->open3DWindow(this, Z3DWindow::EXCLUDE_VOLUME);
      }
    }
  }
}

void MainWindow::on_actionImage_Sequence_triggered()
{
  importImageSequence();
}

void MainWindow::on_actionAddFlyEmNeuron_Network_triggered()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Network"),
                                   m_lastOpenedFilePath,
                                   tr("FlyEM network files (*.fnt)"));

  if (!fileName.isEmpty()) {
    m_lastOpenedFilePath = fileName;

    m_progress->setRange(0, 2);
    m_progress->setLabelText(QString("Loading neuron network: " + fileName + " ..."));
    int currentProgress = 0;
    m_progress->setValue(++currentProgress);
    qApp->processEvents();

    ZStackFrame *frame = new ZStackFrame;
    frame->load(fileName);

    m_progress->setValue(++currentProgress);
    m_progress->reset();

    //addStackFrame(frame);
    frame->open3DWindow(this);
    delete frame;

    QApplication::processEvents(); //force file dialog to close.
                                   //might be a bug in qt
  }
}

void MainWindow::on_actionSynapse_Annotation_triggered()
{
  QStringList fileList =
      QFileDialog::getOpenFileNames(this, tr("Load Synapse Annotations"),
                                   m_lastOpenedFilePath,
                                   tr("JSON files (*.json)"),
                                    NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileList.isEmpty()) {
    ZStackFrame *frame = new ZStackFrame;
    frame->load(fileList);

    frame->open3DWindow(this);
    delete frame;

    QApplication::processEvents(); //force file dialog to close.
                                   //might be a bug in qt
  }
}

void MainWindow::on_actionPosition_triggered()
{
  ParameterDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {
    ZString str(dlg.parameter());
    std::vector<int> posValue = str.toIntegerArray();

    if (posValue.size() == 0) {
      return;
    }

    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      //frame->setViewPortCenter(posValue[0], posValue[1], posValue[2]);
      frame->viewRoi(posValue[0], posValue[1], posValue[2], 100);
    }
  }
}

void MainWindow::on_actionImportMask_triggered()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Import mask"),
                                   m_lastOpenedFilePath,
                                   tr("Image file (*.tif *.xml *.json)"));

  if (!fileName.isEmpty()) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      /*
      ZStackFile file;
      file.import(fileName.toStdString());
      frame->setStackMask(file.readStack());
      */
      //frame->importStackMask(fileName.toStdString());
    }
  }
}

void MainWindow::on_actionFlyEmSelect_triggered()
{
  /*
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ParameterDialog dlg;
    dlg.setWindowTitle(tr("Select Bodies"));
    if (dlg.exec() == QDialog::Accepted) {
      ZString str(dlg.parameter());
      std::vector<int> bodyColor;
      if (str.startsWith("b")) {
        std::vector<int> bodyId = str.toIntegerArray();
        for (size_t i = 0; i < bodyId.size(); ++i) {
          std::vector<uint8_t> code =
              FlyEm::ZSegmentationAnalyzer::idToChannelCode(bodyId[i], 3);
          bodyColor.push_back(code[0]);
          bodyColor.push_back(code[1]);
          bodyColor.push_back(code[2]);
        }
      } else {
        bodyColor = str.toIntegerArray();
      }

      ZStackFrame *newFrame = NULL;

      if (frame->name() == "flyem") {
        ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
        newFrame = completeFrame->spinoffSegmentationSelection(bodyColor);
      } else {
        newFrame = frame->spinoffStackSelection(bodyColor);
      }

      if (newFrame != NULL) {
        addStackFrame(newFrame);
      }
    }
  }*/

  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
    ParameterDialog dlg;
    dlg.setWindowTitle(tr("Select Bodies"));
    dlg.setParamterToolTip(tr("{\"file\": <string>, \n \"id\": [<int>] | <int>, \n \"connection\": [<int>]}"));
    if (dlg.exec() == QDialog::Accepted) {
      ZString str(dlg.parameter());
      ZJsonObject jValue;
      jValue.decodeString(str.c_str());

      json_t *idObject = jValue["file"];

      if (idObject != NULL) {
        ZGraph graph(ZGraph::DIRECTED_WITHOUT_WEIGHT);
        graph.importTxtFile(ZJsonParser::stringValue(idObject));

        for (size_t i = 0; i < graph.size(); ++i) {
          completeFrame->selectSegmentationPair(
                graph.edgeStart(i), graph.edgeEnd(i), false);
        }
      }

      idObject = jValue["id"];
      if (idObject != NULL) {
        std::vector<int> bodyId;
        if (ZJsonParser::isInteger(idObject)) {
          bodyId.push_back(ZJsonParser::integerValue(idObject));
        } else {
          ZJsonArray jArray;
          jArray.set(idObject, false);

          bodyId.resize(jArray.size());

          for (size_t i = 0; i < jArray.size(); ++i) {
            bodyId[i] = ZJsonParser::integerValue(jArray.at(i));
          }
        }
        completeFrame->selectSegmentaion(bodyId, false);
      }

      idObject = jValue["connection"];
      if (idObject != NULL) {
        ZJsonArray jArray;
        jArray.set(idObject, false);

        int bodyId[2];
        for (size_t i = 0; i < jArray.size(); ++i) {
          bodyId[i%2] = ZJsonParser::integerValue(jArray.at(i));
          if (i % 2 == 1) {
            completeFrame->selectSegmentationPair(bodyId[0], bodyId[1], false);
          }
        }
      }
      frame->updateView();
    }

  }
}

void MainWindow::on_actionImportSegmentation_triggered()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Import FlyEM segmentation"),
                                   m_lastOpenedFilePath,
                                   tr("Segmentation file (*.json)"));

  if (!fileName.isEmpty()) {
    ZFlyEmStackFrame *frame = new ZFlyEmStackFrame;
    if (frame->importSegmentationBundle(fileName.toStdString())) {
      addStackFrame(frame);
    } else {
      delete frame;
    }
  }
}

void MainWindow::on_actionFlyEmClone_triggered()
{
  ZStackFrame *frame = activeStackFrame();

  if (frame != NULL) {
    ZFlyEmStackFrame *flyemFrame = new ZFlyEmStackFrame;
    flyemFrame->copyDocument(frame);
    addStackFrame(flyemFrame);
  }
}

void MainWindow::on_actionClear_Decoration_triggered()
{
  ZStackFrame *frame = activeStackFrame();

  if (frame != NULL) {
    frame->clearDecoration();
  }
}

void MainWindow::on_actionFlyEmGrow_triggered()
{
  ZStackFrame *frame = activeStackFrame();

  if (frame != NULL) {
    ParameterDialog dlg;
    dlg.setWindowTitle(tr("Select Bodies"));
    if (dlg.exec() == QDialog::Accepted) {
      ZString str(dlg.parameter());
      std::vector<int> bodyColor;

      bodyColor = str.toIntegerArray();
      ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;

      completeFrame->selectSegmentaion(bodyColor);
      completeFrame->selectNeighborSegmentation(bodyColor);
    }
  }
}

void MainWindow::on_actionFlyEmSelect_connection_triggered()
{
  ZStackFrame *frame = activeStackFrame();

  if (frame != NULL) {
    ParameterDialog dlg;
    dlg.setWindowTitle(tr("Select Connection"));
    if (dlg.exec() == QDialog::Accepted) {
      ZString str(dlg.parameter());
      std::vector<int> bodyId;

      bodyId = str.toIntegerArray();
      ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;

      completeFrame->selectSegmentationPair(bodyId[0], bodyId[1]);
    }
  }
}

void MainWindow::on_actionAxon_Export_triggered()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Import axon"),
                                   m_lastOpenedFilePath,
                                   tr("Axon export file (*.txt)"));

  if (!fileName.isEmpty()) {
    ZFlyEmStackFrame *frame = new ZFlyEmStackFrame;
    frame->createDocument();
    if (frame->importAxonExport(fileName.toStdString())) {
      frame->open3DWindow(this);
      delete frame;
    } else {
      delete frame;
      QMessageBox::critical(this, tr("Error"),
                            QString("<font size=4 face=Times>I cannot open ") +
                            "<i>" + fileName + "</i>" + "</font>",
                            QMessageBox::Ok);
    }
  }
}

void MainWindow::on_actionExtract_body_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ParameterDialog dlg;
    dlg.setWindowTitle(tr("Select Bodies"));
    if (dlg.exec() == QDialog::Accepted) {
      ZString str(dlg.parameter());
      std::vector<int> bodyColor;
      if (str.startsWith("b")) {
        std::vector<int> bodyId = str.toIntegerArray();
        for (size_t i = 0; i < bodyId.size(); ++i) {
          std::vector<uint8_t> code =
              FlyEm::ZSegmentationAnalyzer::idToChannelCode(bodyId[i],
                                                            frame->document()->stackMask()->channelNumber());
          for (size_t k = 0; k < code.size(); ++k) {
            bodyColor.push_back(code[k]);
          }
        }
      } else {
        bodyColor = str.toIntegerArray();
      }

      ZStackFrame *newFrame = NULL;

      if (frame->name() == "flyem") {
        ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
        newFrame = completeFrame->spinoffSegmentationSelection(bodyColor);
      } else {
        newFrame = frame->spinoffStackSelection(bodyColor);
      }

      if (newFrame != NULL) {
        addStackFrame(newFrame);
      }
    }
  }
}

void MainWindow::on_actionPredict_errors_triggered()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    if (frame->name() == "flyem") {
      ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
      completeFrame->predictSegmentationError();
    }
  }
}

void MainWindow::on_actionCompute_Features_triggered()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    if (frame->name() == "flyem") {
      ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
      completeFrame->computeBodyConnFeature();
    }
  }
}

void MainWindow::on_actionMexican_Hat_triggered()
{
  if (currentStackFrame() != NULL) {
    ZStackProcessor proc;
    MexicanHatDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
      proc.mexihatFilter(currentStackFrame()->document()->stack(),
                     dlg.sigma());
      currentStackFrame()->updateView();
    }
  }
}

void MainWindow::on_actionInvert_triggered()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    frame->invertStack();
  }
}

void MainWindow::on_actionFlyEmQATrain_triggered()
{
  ZFlyEmStackFrame::trainBodyConnection();
}

void MainWindow::on_actionUpdate_Configuration_triggered()
{
  NeutubeConfig &config = NeutubeConfig::getInstance();
  if (config.load(config.getConfigPath()) == false) {
    std::cout << "Unable to load configuration: "
              << config.getConfigPath() << std::endl;
  } else {
    customizeActions();
  }
#ifdef _DEBUG_
  config.print();
#endif
}

void MainWindow::on_actionErrorClassifcationTrain_triggered()
{
  ZFlyEmStackFrame::trainBodyConnection();
}

void MainWindow::on_actionErrorClassifcationPredict_triggered()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    if (frame->name() == "flyem") {
      ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
      completeFrame->predictSegmentationError();
    }
  }
}

void MainWindow::on_actionErrorClassifcationEvaluate_triggered()
{
  std::vector<double> threshold(1);
  NeutubeConfig &config = NeutubeConfig::getInstance();
  threshold[0] = config.getSegmentationClassifThreshold();

  ZFlyEmStackFrame::evaluateBodyConnectionClassifier(std::vector<double>(0));
}

void MainWindow::on_actionErrorClassifcationComputeFeatures_triggered()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    if (frame->name() == "flyem") {
      ZFlyEmStackFrame *completeFrame = (ZFlyEmStackFrame*) frame;
      completeFrame->computeBodyConnFeature();
    }
  }
}

void MainWindow::on_actionTem_Paper_Volume_Rendering_triggered()
{
  const NeutubeConfig& config = NeutubeConfig::getInstance();
  std::string dataPath = config.getPath(NeutubeConfig::DATA);
  std::string dataDir = "flyem/skeletonization/session3/smoothed";
  //std::string dataDir = "benchmark/binary/3d/block";
  ZFileList fileList;
  fileList.load(dataPath + "/" + dataDir, "tif");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);
  //Filter_3d *filter = Gaussian_Filter_3d(0.5, 0.5, 0.5);
  //input.resize(1);
  //input[0] = dataPath + "/" + dataDir + "/" + "Pm2-8_171795.tif";

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    std::string output;
    ZString inputPath(*inputIter);
    std::vector<std::string> parts = inputPath.fileParts();
    output = dataPath + "/" + dataDir + "/snapshots/" + parts[1] + ".tif";

    if (!fexist(output.c_str())) {

      std::string offsetFile = *inputIter + ".offset.txt";
      FILE *fp = fopen(offsetFile.c_str(), "r");
      ZString offsetStr;
      offsetStr.readLine(fp);
      std::vector<int> offset =offsetStr.toIntegerArray();
      fclose(fp);

      std::tr1::shared_ptr<ZStackDoc> academy =
          std::tr1::shared_ptr<ZStackDoc>(new ZStackDoc);

      academy->loadFile((*inputIter).c_str());

      double zScale = 1.125;
      Z3DWindow *stage = new Z3DWindow(academy, Z3DWindow::NORMAL_INIT,
                                       false, NULL);
      stage->getVolumeSource()->setZScale(zScale);
      stage->getVolumeRaycaster()->hideBoundBox();

      //const std::vector<double> &boundBox = stage->getBoundBox();

      Z3DCameraParameter* camera = stage->getCamera();
      camera->setProjectionType(Z3DCamera::Orthographic);
      //stage->resetCamera();

      //camera->setUpVector(glm::vec3(0.0, 0.0, -1.0));
      /*
  stage->getInteractionHandler()->getTrackball()->rotate(
        glm::vec3(1.0, 0.0, 0.0), TZ_PI_2);
        */

      glm::vec3 referenceCenter = camera->getCenter();

      double eyeDistance = 3000;//boundBox[3] - referenceCenter[1] + 2500;
      //double eyeDistance = 2000 - referenceCenter[1];
      glm::vec3 viewVector(0, -1, 0);

      viewVector *= eyeDistance;
      glm::vec3 eyePosition = referenceCenter - viewVector;

      referenceCenter[2] = (650 - offset[2]) * zScale;
      camera->setCenter(referenceCenter);
      eyePosition[2] = (650 - offset[2]) * zScale;
      camera->setEye(eyePosition);
      camera->setUpVector(glm::vec3(0, 0, -1));
      stage->resetCameraClippingRange();

      stage->getCompositor()->setBackgroundFirstColor(0, 0, 0, 1);
      stage->getCompositor()->setBackgroundSecondColor(0, 0, 0, 1);

      //std::cout << "scales: " << stage->getVolumeRaycaster()->getRenderer()->getCoordScales() << std::endl;

      camera->setNearDist(2000.0);

      stage->show();

      std::cout << output << std::endl;
      stage->takeScreenShot(output.c_str(), 4000, 4000, MonoView);
      stage->close();
      delete stage;
    }
  }
}

void MainWindow::on_actionTem_Paper_Neuron_Type_Figure_triggered()
{
  const NeutubeConfig& config = NeutubeConfig::getInstance();
  std::string dataPath = config.getPath(NeutubeConfig::DATA);
  std::string sessionDir = "flyem/skeletonization/session3";
  std::string dataDir = sessionDir + "/smoothed/snapshots/contrast/selected";
  ZFileList fileList;
  fileList.load(dataPath + "/" + dataDir, "tif", ZFileList::SORT_ALPHABETICALLY);

  FILE *fp = fopen((dataPath + "/" + sessionDir + "/" + "neuron_type.txt").c_str(), "r");
  if (fp == NULL) {
    std::cout << "Cannot open " << sessionDir + "/" + "neuron_type.txt" << std::endl;
  }
  ZString neuronTypeLine;
  std::vector<std::string> neuronTypeArray;
  neuronTypeArray.push_back("scale_bar");
  while (neuronTypeLine.readLine(fp)) {
    neuronTypeLine.trim();
    if ((neuronTypeLine[0] >= 'A' && neuronTypeLine[0] <= 'Z') ||
        (neuronTypeLine[0] >= 'a' && neuronTypeLine[0] <= 'z')) {
      neuronTypeArray.push_back(neuronTypeLine);
    }
  }

  fclose(fp);
  std::cout << neuronTypeArray.size() << " neuron types" << std::endl;

  std::vector<Stack*> textPatchArray;
  Stack *textImage = Read_Stack_U((dataPath + "/benchmark/mlayer_label.tif").c_str());
  for (int i =0; i < 10; ++i) {
    Stack textPatch2 = *textImage;
    textPatch2.depth = 1;
    textPatch2.array = textImage->array +
        i * C_Stack::width(textImage) * C_Stack::height(textImage);
    Stack *textPatch = C_Stack::boundCrop(&textPatch2);
    textPatchArray.push_back(textPatch);
  }
  C_Stack::kill(textImage);

  std::cout << neuronTypeArray.size() << " cell types" << std::endl;
  //ParameterDialog dlg;

  int totalCellNumber = 0;

  //if (dlg.exec()) {
  for (size_t neuronTypeIndex = 0; neuronTypeIndex < neuronTypeArray.size();
       ++neuronTypeIndex) {
    //std::string neuronType = dlg.parameter().toStdString();
    std::string neuronType = neuronTypeArray[neuronTypeIndex];

    std::vector<std::string> input;
    for (int i = 0; i < fileList.size(); ++i) {
      ZString path(fileList.getFilePath(i));
      std::vector<std::string> parts = path.fileParts();
      ZString fileName = parts[1];
      if (fileName.startsWith(neuronType)) {
        bool isTarget = true;
        if (isdigit(neuronType[neuronType.length() - 1])) {
          if (isdigit(fileName[neuronType.length()])) {
            isTarget = false;
          }
        }
        if (isTarget) {
          input.push_back(path.c_str());
        }
      } else {
        if (neuronType == "T4") {
          if (fileName.startsWith("nT4")) {
            input.push_back(path.c_str());
          }
        }
      }
    }

    /*
    if (neuronType == "Tm3") {
      input.resize(11);
    }
*/
    std::cout << neuronType << ": " << input.size() << " cells" << std::endl;

    if (neuronType != "scale_bar") {
      totalCellNumber += input.size();
    }

    int mLayerStart = 317;
    //int mLayerStart = 692;
    //int mLayerEnd = 3538;
    int mLayerEnd = 3534;

    double layerPercent[] = {0, 9.4886, 18.5061, 29.6097, 33.3782, 40.3096,
                             46.9717, 57.8735, 72.1400, 89.0982, 100.0000};
    int layerArray[11];
    for (int i = 0; i < 11; ++i) {
      layerArray[i] = mLayerStart +
          iround(layerPercent[i] * (mLayerEnd - mLayerStart) / 100.0);
    }
    //input.resize(5);

    int rowSize = 5;
    int nrow = input.size() / rowSize + (input.size() % rowSize > 0);

    std::vector<std::string>::const_iterator inputIter = input.begin();
    for (int row = 0; row < nrow; ++row) {
      std::vector<Stack*> stackArray;

      int finalWidth = 0;
      int count = 1;
      for (; inputIter != input.end(); ++inputIter) {
        if (count > rowSize) {
          break;
        }
        Stack *stack = Read_Stack_U(inputIter->c_str());
        Stack *stack2 = Crop_Stack(stack, 200, 0, 0, C_Stack::width(stack) - 201,
                                   C_Stack::height(stack), 1, NULL);
        C_Stack::kill(stack);
        Cuboid_I box;
        Stack_Bound_Box(stack2, &box);
        int left = box.cb[0] -50;
        int width = box.ce[0] - box.cb[0] + 100;
        Stack *crop = Crop_Stack(stack2, left, 0, 0, width, stack2->height, 1, NULL);

        if (neuronType != "scale_bar") {
          //Draw body id
          int bodyId = String_Last_Integer(inputIter->c_str());
          int interval = 15;
          int intWidth = C_Stack::integerWidth(bodyId, interval);
          C_Stack::drawInteger(crop, bodyId,
                               (C_Stack::width(crop) - intWidth) / 2,
                               C_Stack::height(crop) - 200, 0, interval);
        }

        finalWidth += width;
        stackArray.push_back(crop);
        C_Stack::kill(stack2);
        ++count;
      }

      int leftMargin = 100;
      int rightMargin = 800;

      Stack *out = C_Stack::make(GREY, finalWidth + leftMargin + rightMargin, C_Stack::height(stackArray[0]), 1);

      Zero_Stack(out);
      uint8_t *outArray = out->array;

      for (int h = 0; h < C_Stack::height(stackArray[0]); ++h) {
        outArray += leftMargin;
        for (size_t i = 0; i < stackArray.size(); ++i) {
          memcpy(outArray,
                 stackArray[i]->array + h * C_Stack::width(stackArray[i]),
                 C_Stack::width(stackArray[i]));
          outArray += C_Stack::width(stackArray[i]);
        }
        outArray += rightMargin;
      }

      for (size_t i = 0; i < stackArray.size(); ++i) {
        C_Stack::kill(stackArray[i]);
      }

      if (neuronType != "scale_bar") {
        //Draw lines
        for (int i = 0; i < C_Stack::width(out); ++i) {
          int v = 250;
          for (int layer = 0; layer < 11; ++layer) {
            if (out->array[i + C_Stack::width(out) * layerArray[layer]] == 0) {
              out->array[i + C_Stack::width(out) * layerArray[layer]] = v;
            }
            for (int w = 1; w <= 2; ++w) {
              if (out->array[i + C_Stack::width(out) * (layerArray[layer] - w)] == 0) {
                out->array[i + C_Stack::width(out) * (layerArray[layer] - w)] = v / (w + 1);
              }
              if (out->array[i + C_Stack::width(out) * (layerArray[layer] + w)] == 0) {
                out->array[i + C_Stack::width(out) * (layerArray[layer] + w)] = v / (w + 1);
              }
            }
          }
        }

        //Draw texts
        for (int layer = 0; layer < 10; ++layer) {
          int dx = C_Stack::width(out) - 250;
          int dy = (layerArray[layer] + layerArray[layer + 1]) / 2 -
              C_Stack::height(textPatchArray[layer]) / 2;
          C_Stack::drawPatch(out, textPatchArray[layer], dx, dy, 0, 0);
        }

        Stack *scaleBar = Read_Stack_U((dataPath + "/" + dataDir + "/row/scale_bar_row1.tif").c_str());
        Stack *croppedScaleBar = C_Stack::boundCrop(scaleBar);
        C_Stack::kill(scaleBar);
        C_Stack::drawPatch(out, croppedScaleBar, C_Stack::width(out) - 700, C_Stack::height(out) - 200, 0, 0);
        C_Stack::kill(croppedScaleBar);
      }

      std::ostringstream stream;
      stream << dataPath + "/" + dataDir + "/row/" + neuronType
             << "_row" << row + 1 << ".tif";

      Write_Stack_U(stream.str().c_str(), out, NULL);
      C_Stack::kill(out);
    }
  }

  for (int layer = 0; layer < 10; ++layer) {
    C_Stack::kill(textPatchArray[layer]);
  }

  std::cout << "Total: " << totalCellNumber << " neurons" << std::endl;
}

void MainWindow::on_actionBinary_SWC_triggered()
{
  on_actionSkeletonization_triggered();
}

void MainWindow::on_actionImportFlyEmDatabase_triggered()
{
  QString fileName = getOpenFileName(tr("Load FlyEM Database"),
                                     tr("Json files (*.json)"));

  if (!fileName.isEmpty()) {
    m_progress->setRange(0, 3);

    int currentProgress = 0;
    m_progress->open();
    m_progress->setLabelText(QString("Loading " + fileName + " ..."));

    m_progress->setValue(++currentProgress);

    ZFlyEmDataFrame *frame = new ZFlyEmDataFrame;
    if (frame->load(fileName.toStdString())) {
      addFlyEmDataFrame(frame);
    } else {
      delete frame;
      reportFileOpenProblem(fileName);
    }
    m_progress->reset();
  }
}

void MainWindow::recordLastOpenPath(const QString &path)
{
  m_lastOpenedFilePath = path;
}

QString MainWindow::getLastOpenPath() const
{
  return m_lastOpenedFilePath;
}

void MainWindow::reportFileOpenProblem(const QString &filePath,
                                       const QString &reason)
{
  QString finalReason = reason;

  if (reason.isEmpty()) {
    finalReason = "unknown reason";
  }

  report(std::string("File Open Error"),
         std::string("Cannot open ") + filePath.toStdString() +
         finalReason.toStdString(), ZMessageReporter::Warning);

#if 0
  QMessageBox::critical(this, tr("Error"),
                      QString("<font size=4 face=Times>I cannot open ") +
                      "<i>" + filePath + "</i>: " + finalReason + "</font>",
                      QMessageBox::Ok);
#endif


}

void MainWindow::addFlyEmDataFrame(ZFlyEmDataFrame *frame)
{
  if (frame  != NULL) {
    QApplication::processEvents();

    QMdiSubWindow *subWindow = mdiArea->addSubWindow(frame);
    connect(frame, SIGNAL(volumeTriggered(const QString&)),
            this, SLOT(openFile(const QString&)));
    frame->setStatusBar(statusBar());

    subWindow->show();
  }
}

void MainWindow::on_actionMake_Movie_triggered()
{
  const NeutubeConfig &config = NeutubeConfig::getInstance();

  m_movieDlg.setScriptPath((config.getPath(NeutubeConfig::DATA) +
                           "/flyem/FIB/movie/reconstruct.json").c_str());
  m_movieDlg.setOutputPath((config.getPath(NeutubeConfig::DATA) +
                            "/flyem/FIB/movie/frame").c_str());

  if (m_movieDlg.exec()) {
    QString fileName = m_movieDlg.getScriptPath();
    /*
        QFileDialog::getOpenFileName(this, tr("Load Movie Script"),
                                     (config.getPath(NeutubeConfig::DATA) +
                                      "/flyem/TEM/movie/script5.json").c_str(),
                                     tr("Movie files (*.json)"));
*/
    if (!fileName.isEmpty()) {
      ZMovieScript script;

      if (script.loadScript(fileName.toStdString())) {
        QString saveFileDir = m_movieDlg.getOutputPath();
        /*
            QFileDialog::getExistingDirectory(this, tr("Movie Output"),
                                              (config.getPath(NeutubeConfig::DATA) +
                                               "/test/movie2").c_str(),
                                              QFileDialog::ShowDirsOnly);
                                              */

        if (!saveFileDir.isEmpty()) {
          m_progress->setRange(0, 3);
          m_progress->show();

          std::string outDir = saveFileDir.toStdString();

          ZMovieMaker director;
          director.setFrameSize(
                m_movieDlg.getFrameWidth(), m_movieDlg.getFrameHeight());
          director.setScript(script);
          director.setFrameInterval(1000 / m_movieDlg.getFrameRate());

          m_progress->setValue(1);

          director.make(outDir);

          m_progress->reset();
        }
      }
    }
  }
}

void MainWindow::on_actionOpen_3D_View_Without_Volume_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->open3DWindow(frame, Z3DWindow::EXCLUDE_VOLUME);
  }
}

void MainWindow::on_actionLoop_Analysis_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->findLoopInStack();
  }
}

void MainWindow::on_actionMorphological_Thinning_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    frame->bwthin();
  }
}

void MainWindow::on_actionAddMask_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    QString fileName = QFileDialog::getOpenFileName(
          this, tr("Load Mask"), m_lastOpenedFilePath,
          tr("Image files (*.tif *.png)"),
          NULL/*, QFileDialog::DontUseNativeDialog*/);
    if (!fileName.isEmpty()) {
      frame->importMask(fileName);
      m_lastOpenedFilePath = fileName;
    }
#if 0
      ZStack *stack = NULL;
      if (ZFileType::fileType(fileName.toStdString()) == ZFileType::PNG_FILE) {
        QImage image;
        image.load(fileName);
        stack = new ZStack(GREY, image.width(), image.height(), 1, 1);

        size_t offset = 0;
        for (int y = 0; y < image.height(); ++y) {
          for (int x = 0; x < image.width(); ++x) {
            QRgb rgb = image.pixel(x, y);
            if (image.hasAlphaChannel()) {
              stack->array8()[offset] = qAlpha(rgb);
            } else {
              stack->array8()[offset] = qRed(rgb);
            }
            ++offset;
          }
        }
      } else {
        ZStackFile stackFile;
        stackFile.import(fileName.toStdString());
        stack = stackFile.readStack();
      }

      if (stack != NULL) {
        if (stack->channelNumber() == 1 && stack->kind() == GREY) {
          ZObject3d *obj = new ZObject3d;
          obj->setColor(QColor(255, 0, 0, 128));
          if (obj->loadStack(stack->c_stack(0))) {
            frame->executeAddObjectCommand(obj, NeuTube::Documentable_OBJ3D);
          } else {
            delete obj;
            report("Loading mask failed", "Cannot convert the image into mask",
                   ZMessageReporter::ERROR);
          }
        } else {
          report("Loading mask failed", "Must be single 8-bit image",
                 ZMessageReporter::ERROR);
        }
        delete stack;
      }
    }
#endif
  }
}

void MainWindow::on_actionMask_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ZString path = frame->document()->stackSourcePath();
    if (!path.empty()) {
      path = path.changeExt("Mask.tif");
    } else {
      path = m_lastOpenedFilePath;
    }

    QString fileName = QFileDialog::getSaveFileName(
          this, tr("Save Mask"), path.c_str(), tr("TIF files (*.tif)"),
          NULL/*, QFileDialog::DontUseNativeDialog*/);
    if (!fileName.isEmpty()) {
      frame->exportObjectMask(NeuTube::RED, fileName);
      m_lastOpenedFilePath = fileName;
    }
  }
}

void MainWindow::on_actionShortcut_triggered()
{
  m_helpDlg->show();
  m_helpDlg->raise();
}

void MainWindow::on_actionMake_Projection_triggered()
{
  ZStackFrame *frame = currentStackFrame();
  if (frame != NULL) {
    ProjectionDialog paramDlg;
    if (paramDlg.exec()) {
      getProgressDialog()->setLabelText("Making projection ...");
      getProgressDialog()->setRange(0, 100);

      ZQtBarProgressReporter reporter;
      reporter.setProgressBar(getProgressBar());

      ZProgressReporter *oldReporter = frame->document()->getProgressReporter();
      frame->document()->setProgressReporter(&reporter);
      getProgressDialog()->open();

      Biocytin::ZStackProjector projector;
      projector.setAdjustingContrast(paramDlg.adjustingContrast());
      projector.setSpeedLevel(paramDlg.speedLevel());
      projector.setSmoothingDepth(paramDlg.smoothingDepth());
      projector.setUsingExisted(paramDlg.usingExisted());

      ZStack *stack = frame->document()->projectBiocytinStack(projector);
      if (stack != NULL) {
        ZStackFrame *newFrame = new ZStackFrame;
        newFrame->setParentFrame(frame);
        newFrame->loadStack(stack);
        addStackFrame(newFrame);
        //stretchStackFrame(newFrame);
        presentStackFrame(newFrame);
      }
      frame->document()->setProgressReporter(oldReporter);

      getProgressDialog()->reset();
      //dlg.reset();
    }
  }
}

QProgressDialog* MainWindow::getProgressDialog()
{
  return m_progress;
}

QProgressBar* MainWindow::getProgressBar()
{
  return getProgressDialog()->findChild<QProgressBar*>();

  /*
  const QObjectList &objList = getProgressDialog()->children();
  foreach (QObject* obj, objList) {
    QProgressBar *bar = qobject_cast<QProgressBar*>(obj);
    if (bar != NULL) {
      return bar;
    }
  }

  return NULL;
  */
}

void MainWindow::on_actionMask_SWC_triggered()
{
  FlyEmSkeletonizationDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {
    ZStackFrame *frame = currentStackFrame();
    if (frame != NULL) {
      //ZStack *stack = frame->getStrokeMask();
      ZStack *stack = frame->getObjectMask(NeuTube::RED);
      if (stack != NULL) {
        Stack *stackData = stack->c_stack();

        ZStackSkeletonizer skeletonizer;
        QProgressDialog *progressDlg = getProgressDialog();
        //progressDlg->setCancelButton(NULL);
        progressDlg->setLabelText("Making SWC ...");
        progressDlg->setRange(0, 100);
        QProgressBar *bar = getProgressBar();

        ZQtBarProgressReporter reporter;
        reporter.setProgressBar(bar);

        skeletonizer.setProgressReporter(&reporter);

        progressDlg->open();

        reporter.start();

        skeletonizer.setRebase(true);
        if (dlg.isExcludingSmallObj()) {
          skeletonizer.setMinObjSize(dlg.sizeThreshold());
        } else {
          skeletonizer.setMinObjSize(0);
        }

        double distThre = -1.0;
        if (!dlg.isConnectingAll()) {
          distThre = dlg.distanceThreshold();
        }
        skeletonizer.setDistanceThreshold(distThre);

        skeletonizer.setLengthThreshold(dlg.lengthThreshold());
        skeletonizer.setKeepingSingleObject(dlg.isKeepingShortObject());

        if (dlg.isLevelChecked()) {
          skeletonizer.setLevel(dlg.level());
        }

        if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
          skeletonizer.setResolution(1.0, NeutubeConfig::getInstance().
                                     getZ3DWindowConfig().getSwcTabConfig().
                                     getZScale());
          skeletonizer.setConnectingBranch(false);
        }

        reporter.startSubprogress(0.5);

        //skeletonizer.setRebase(false);

        ZSwcTree *wholeTree = skeletonizer.makeSkeleton(stackData);
        reporter.endSubprogress(0.5);

        if (wholeTree != NULL) {
#if 1
          if (frame->getParentFrame() != NULL) {
            ZSwcPositionAdjuster adjuster;
            adjuster.setProgressReporter(&reporter);
            adjuster.setSignal(
                  frame->getParentFrame()->document()->stack()->c_stack(0),
                  NeuTube::IMAGE_BACKGROUND_BRIGHT);
            adjuster.getProgressReporter()->startSubprogress(0.3);
            tic();
            adjuster.adjustPosition(*wholeTree);
            ptoc();
            adjuster.getProgressReporter()->endSubprogress(0.3);
          } else {
            if (frame->document()->stack()->channelNumber() == 2) {
              Stack *depthData = frame->document()->stack()->c_stack(1);
              if (depthData != NULL) {
                Biocytin::SwcProcessor::assignZ(wholeTree, *depthData);
              }
            }
          }

          Biocytin::SwcProcessor::breakZJump(wholeTree, 5.0);

          Biocytin::SwcProcessor::removeOrphan(wholeTree);
          Biocytin::SwcProcessor::smoothZ(wholeTree);
          reporter.advance(0.1);

          skeletonizer.setConnectingBranch(true);
          skeletonizer.reconnect(wholeTree);
#endif
          ZStackFrame *swcFrame = frame->getParentFrame();
          if (swcFrame == NULL) {
            swcFrame = new ZStackFrame;
            swcFrame->createDocument();
          }

          reporter.advance(0.1);

          reporter.end();

          progressDlg->reset();

          if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
            wholeTree->setType(
                  ZBiocytinFileNameParser::getTileIndex(
                    frame->getParentFrame()->document()->stack()->sourcePath()));
          }


          swcFrame->document()->blockSignals(true);
          swcFrame->document()->addSwcTree(wholeTree);

          if (frame->getParentFrame() == NULL) {
            delete swcFrame;
          } else {
            swcFrame->document()->estimateSwcRadius();
            Biocytin::SwcProcessor::smoothRadius(wholeTree);
          }

          ZSwcResampler resampler;
          resampler.optimalDownsample(wholeTree);
          swcFrame->document()->blockSignals(false);

          swcFrame->document()->notifySwcModified();

          swcFrame->open3DWindow(this, Z3DWindow::EXCLUDE_VOLUME);
        } else {
          progressDlg->reset();
          report("Skeletonization failed", "No SWC tree generated.",
                 ZMessageReporter::Error);
        }
      }
    }
  }
}

void MainWindow::expandCurrentFrame()
{
  ZStackFrame *frame = activeStackFrame();
  if (frame != NULL) {
    QStringList fileList =
        QFileDialog::getOpenFileNames(this, tr("Load files"),
                                      m_lastOpenedFilePath,
                                      tr("SWC/Image files (*.swc *.png *.tif)"),
                                      NULL/*, QFileDialog::DontUseNativeDialog*/);

    bool swcLoaded = false;
    if (!fileList.isEmpty()) {
      foreach (QString filePath, fileList) {
        switch (ZFileType::fileType(filePath.toStdString())) {
        case ZFileType::SWC_FILE:
          frame->importSwc(filePath);
          swcLoaded = true;
          break;
        case ZFileType::TIFF_FILE:
        case ZFileType::PNG_FILE:
          frame->importMask(filePath);
          break;
        default:
          break;
        }
      }

      m_lastOpenedFilePath = fileList.last();

      if (swcLoaded) {
        if (NeutubeConfig::getInstance().getMainWindowConfig().
            isExpandSwcWith3DWindow()) {
          frame->open3DWindow(this, Z3DWindow::EXCLUDE_VOLUME);
        }
      }
    }
  }
}

void MainWindow::on_actionAutosaved_Files_triggered()
{
  m_autosaveSwcDialog.updateFile();

  m_autosaveSwcDialog.show();
}

void MainWindow::on_actionDiagnosis_triggered()
{
  m_DiagnosisDlg->show();
  m_DiagnosisDlg->scrollToBottom();
  m_DiagnosisDlg->raise();
}

void MainWindow::on_actionSave_SWC_triggered()
{
  ZStackFrame *frame= currentStackFrame();
  if (frame != NULL) {
    if (frame->document()->hasSwc()) {
      std::string swcSource = frame->document()->getSwcSource();
      if (swcSource.empty()) {
        swcSource = "./untitled.swc";
      }

      QString fileName =
          QFileDialog::getSaveFileName(this, tr("Save neuron as SWC"),
                                       swcSource.c_str(),
                                       tr("SWC file (*.swc) "));

      if (!fileName.isEmpty()) {
        frame->document()->saveSwc(fileName.toStdString());
      }
    } else {
      m_reporter->report("Warning", "No SWC found", ZMessageReporter::Warning);
    }
  }
}

void MainWindow::on_actionSimilarity_Matrix_triggered()
{
  ZFlyEmDataFrame *frame =
      dynamic_cast<ZFlyEmDataFrame*>(mdiArea->currentSubWindow());

  if (frame != NULL) {
    QString fileName = getSaveFileName("Export Similarity Matrix", "*.txt");

    if (!fileName.isEmpty()) {
      frame->exportSimilarityMatrix(fileName);
    }
  }

}

void MainWindow::on_actionSparse_objects_triggered()
{
  QStringList fileList =
      getOpenFileNames("Load Multiple Objects", "*.sobj");

  if (!fileList.isEmpty()) {
    ZStackFrame *frame = new ZStackFrame;
    frame->importSobj(fileList);
    addStackFrame(frame);
  }
}
