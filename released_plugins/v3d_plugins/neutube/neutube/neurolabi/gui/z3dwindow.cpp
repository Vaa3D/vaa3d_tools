#include "z3dwindow.h"

#include <iostream>
#include <sstream>

#include <z3dgl.h>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include <limits>

#include "zstack.hxx"
#include "zstackdoc.h"
#include "zstackframe.h"

#include "neutubeconfig.h"
#include "z3dinteractionhandler.h"
#include "z3dapplication.h"
#include "z3dnetworkevaluator.h"
#include "z3dcanvasrenderer.h"
#include "z3daxis.h"
#include "z3dpunctafilter.h"
#include "z3dswcfilter.h"
#include "z3dcompositor.h"
#include "z3dvolumesource.h"
#include "z3dvolumeraycaster.h"
#include "zpunctum.h"
#include "zlocsegchain.h"
#include "z3dtakescreenshotwidget.h"
#include "z3dcanvas.h"
#include <QThread>
#include "z3dgraphfilter.h"
#include "zswcnetwork.h"
#include "zcloudnetwork.h"
#include "znormcolormap.h"
#include "swctreenode.h"
#include "swctypedialog.h"
#include "swcsizedialog.h"
#include "swcskeletontransformdialog.h"
#include "zswcbranch.h"
#include "zswcdisttrunkanalyzer.h"
#include "zswcbranchingtrunkanalyzer.h"
#include "zfiletype.h"
#include "zswcsizetrunkanalyzer.h"
#include "zswcweighttrunkanalyzer.h"
#include "tubemodel.h"
#include "informationdialog.h"
#include "zmoviescene.h"
#include "zmovieactor.h"
#include "zswcmovieactor.h"
#include "zmoviemaker.h"
#include "zmoviescript.h"
#include "zobjsmanagerwidget.h"
#include "zswcobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "qcolordialog.h"
#include "zalphadialog.h"
#include "zstring.h"
#include "zpunctumio.h"
#include "zswcglobalfeatureanalyzer.h"
#include "misc/miscutility.h"

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};


Z3DWindow::Z3DWindow(std::tr1::shared_ptr<ZStackDoc> doc, Z3DWindow::EInitMode initMode,
                     bool stereoView, QWidget *parent)
  : QMainWindow(parent)
  , m_doc(doc)
  , m_networkEvaluator(NULL)
  , m_canvas(NULL)
  , m_canvasRenderer(NULL)
  , m_axis(NULL)
  , m_volumeRaycaster(NULL)
  , m_punctaFilter(NULL)
  , m_compositor(NULL)
  , m_volumeBoundBox(6)
  , m_swcBoundBox(6)
  , m_punctaBoundBox(6)
  , m_graphBoundBox(6)
  , m_boundBox(6)
  , m_isClean(false)
  , m_screenShotWidget(NULL)
  , m_widgetsGroup(NULL)
  , m_settingsDockWidget(NULL)
  , m_objectsDockWidget(NULL)
  , m_advancedSettingDockWidget(NULL)
  , m_isStereoView(stereoView)
{
  if (m_doc->stack() != NULL) {
    setWindowTitle(m_doc->stack()->sourcePath());
  }
  setAttribute(Qt::WA_DeleteOnClose);
  setFocusPolicy(Qt::StrongFocus);
  createActions();
  createMenus();
  createStatusBar();
  m_viewMenu->addAction("Reset Camera", this, SLOT(resetCamera()));
  init(initMode);
  createDockWindows();
  setAcceptDrops(true);
  m_mergedContextMenu = new QMenu(this);
}

Z3DWindow::~Z3DWindow()
{
  cleanup();
}

void Z3DWindow::createStatusBar()
{
  statusBar()->showMessage("3D window ready.");
}

void Z3DWindow::gotoPosition(double x, double y, double z, double radius)
{
  double xsize = radius;
  double ysize = radius;
  double zsize = radius;
  std::vector<double> bound(6);
  bound[0] = x - xsize;
  bound[1] = x + xsize;
  bound[2] = y - ysize;
  bound[3] = y + ysize;
  bound[4] = z - zsize;
  bound[5] = z + zsize;
  setupCamera(bound, Z3DCamera::ResetAll);
}

void Z3DWindow::gotoPosition(std::vector<double> bound, double minRadius,
                             double range)
{

  if (bound[1] - bound[0] < minRadius * 2) {
    double expand = range - bound[1] + bound[0];
    bound[1] += expand/2;
    bound[0] -= expand/2;
  }
  if (bound[3] - bound[2] < minRadius * 2) {
    double expand = range - bound[3] + bound[2];
    bound[3] += expand/2;
    bound[2] -= expand/2;
  }

  if (bound[5] - bound[4] < minRadius * 2) {
    double expand = range - bound[5] + bound[4];
    bound[5] += expand/2;
    bound[4] -= expand/2;
  }

  setupCamera(bound, Z3DCamera::ResetAll);
}

void Z3DWindow::init(EInitMode mode)
{
  // processors
  m_axis = new Z3DAxis();

  // more processors: init volumes
  if (mode == EXCLUDE_VOLUME) {
    m_volumeSource = new Z3DVolumeSource(NULL);
  } else {
    m_volumeSource = new Z3DVolumeSource(m_doc.get());
    connect(m_volumeSource, SIGNAL(xScaleChanged()), this, SLOT(volumeScaleChanged()));
    connect(m_volumeSource, SIGNAL(yScaleChanged()), this, SLOT(volumeScaleChanged()));
    connect(m_volumeSource, SIGNAL(zScaleChanged()), this, SLOT(volumeScaleChanged()));
    connect(getDocument(), SIGNAL(stackModified()), this, SLOT(volumeChanged()));
  }

  // more processors: init geometry filters
  m_compositor = new Z3DCompositor();
  m_punctaFilter = new Z3DPunctaFilter();
  m_punctaFilter->setData(m_doc->punctaList());
  m_swcFilter = new Z3DSwcFilter();
  m_swcFilter->setData(m_doc->swcList());
  m_graphFilter = new Z3DGraphFilter();
  if (m_doc->swcNetwork() != NULL) {
    ZPointNetwork *network = m_doc->swcNetwork()->toPointNetwork();
    //ZNormColorMap colorMap;
    //m_graphFilter->setData(*network, &colorMap);
    m_graphFilter->setData(*network, NULL);

    delete network;
  } else if (ZFileType::fileType(m_doc->additionalSource()) ==
             ZFileType::JSON_FILE) {
    Z3DGraph graph;
    graph.importJsonFile(m_doc->additionalSource());
    m_graphFilter->setData(graph);
  }

  connect(getDocument(), SIGNAL(punctaModified()), this, SLOT(punctaChanged()));
  connect(getDocument(), SIGNAL(swcModified()), this, SLOT(swcChanged()));
  connect(getDocument(), SIGNAL(swcNetworkModified()),
          this, SLOT(updateNetworkDisplay()));
  connect(getDocument(),
          SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
          this, SLOT(punctaSelectionChanged()));
  connect(getDocument(),
          SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          this, SLOT(swcSelectionChanged()));
  connect(getDocument(),
          SIGNAL(swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
          this, SLOT(swcTreeNodeSelectionChanged()));
  connect(getDocument(),
          SIGNAL(punctumVisibleStateChanged(ZPunctum*, bool)),
          m_punctaFilter, SLOT(updatePunctumVisibleState()));
  connect(getDocument(),
          SIGNAL(swcVisibleStateChanged(ZSwcTree*, bool)),
          m_swcFilter, SLOT(updateSwcVisibleState()));
  connect(m_punctaFilter->getRendererBase(), SIGNAL(coordScalesChanged()),
          this, SLOT(punctaCoordScaleChanged()));
  connect(m_swcFilter->getRendererBase(), SIGNAL(coordScalesChanged()),
          this, SLOT(swcCoordScaleChanged()));
  connect(m_punctaFilter->getRendererBase(), SIGNAL(sizeScaleChanged()),
          this, SLOT(punctaSizeScaleChanged()));
  connect(m_swcFilter->getRendererBase(), SIGNAL(sizeScaleChanged()),
          this, SLOT(swcSizeScaleChanged()));
  connect(m_punctaFilter, SIGNAL(punctumSelected(ZPunctum*, bool)),
          this, SLOT(selectedPunctumChangedFrom3D(ZPunctum*, bool)));
  connect(m_swcFilter, SIGNAL(treeSelected(ZSwcTree*,bool)),
          this, SLOT(selectedSwcChangedFrom3D(ZSwcTree*,bool)));
  connect(m_swcFilter, SIGNAL(treeNodeSelected(Swc_Tree_Node*,bool)),
          this, SLOT(selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node*,bool)));

  connect(m_swcFilter, SIGNAL(treeNodeSelectConnection()),
          m_doc.get(), SLOT(selectSwcNodeConnection()));
  connect(m_swcFilter, SIGNAL(addNewSwcTreeNode(double, double, double, double)),
          this, SLOT(addNewSwcTreeNode(double, double, double, double)));
  connect(m_swcFilter, SIGNAL(extendSwcTreeNode(double, double, double)),
          this, SLOT(extendSwcTreeNode(double, double, double)));
  connect(m_swcFilter, SIGNAL(connectingSwcTreeNode(Swc_Tree_Node*)), this,
          SLOT(connectSwcTreeNode(Swc_Tree_Node*)));

  m_swcFilter->setSelectedSwcs(m_doc->selectedSwcs());
  m_swcFilter->setSelectedSwcTreeNodes(m_doc->selectedSwcTreeNodes());
  m_punctaFilter->setSelectedPuncta(m_doc->selectedPuncta());

  // init windows size based on data
  setWindowSize();

  // init canvas and opengl context
  QGLFormat format = QGLFormat();
  format.setAlpha(true);
  format.setDepth(true);
  format.setDoubleBuffer(true);
  format.setRgba(true);
  format.setSampleBuffers(true);
  if (m_isStereoView)
    format.setStereo(true);
  m_canvas = new Z3DCanvas("", 512, 512, format);
  setCentralWidget(m_canvas);
  m_canvas->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_canvas, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(show3DViewContextMenu(QPoint)));
  m_canvas->getGLFocus();

  m_canvasRenderer = new Z3DCanvasRenderer();
  m_canvasRenderer->setCanvas(m_canvas);

  // more processors: init raycaster
  m_volumeRaycaster = new Z3DVolumeRaycaster();
  connect(m_volumeRaycaster, SIGNAL(pointInVolumeLeftClicked(QPoint, glm::ivec3)),
          this, SLOT(pointInVolumeLeftClicked(QPoint, glm::ivec3)));

  // register processors to receive QGLWidget event
  m_canvas->addEventListenerToBack(m_swcFilter);
  m_canvas->addEventListenerToBack(m_punctaFilter);
  m_canvas->addEventListenerToBack(m_volumeRaycaster);      // for trace
  m_canvas->addEventListenerToBack(m_compositor);  // for interaction
  m_canvas->addEventListenerToBack(m_graphFilter);

  // build network
  for (int i=0; i<5; i++) {  // max supported channel is 5
    m_volumeSource->getOutputPort(QString("Volume%1").arg(i+1))->connect(m_volumeRaycaster->getInputPort("Volumes"));
  }
  m_volumeSource->getOutputPort("Stack")->connect(m_volumeRaycaster->getInputPort("Stack"));
  m_volumeRaycaster->getOutputPort("Image")->connect(m_compositor->getInputPort("Image"));
  m_volumeRaycaster->getOutputPort("LeftEyeImage")->connect(m_compositor->getInputPort("LeftEyeImage"));
  m_volumeRaycaster->getOutputPort("RightEyeImage")->connect(m_compositor->getInputPort("RightEyeImage"));
  m_punctaFilter->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_swcFilter->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_graphFilter->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_axis->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_compositor->getOutputPort("Image")->connect(m_canvasRenderer->getInputPort("Image"));
  m_compositor->getOutputPort("LeftEyeImage")->connect(m_canvasRenderer->getInputPort("LeftEyeImage"));
  m_compositor->getOutputPort("RightEyeImage")->connect(m_canvasRenderer->getInputPort("RightEyeImage"));

  // connection: canvas <-----> networkevaluator <-----> canvasrender
  m_networkEvaluator = new Z3DNetworkEvaluator();
  m_canvas->setNetworkEvaluator(m_networkEvaluator);

  // pass the canvasrender to the network evaluator
  m_networkEvaluator->setNetworkSink(m_canvasRenderer);

  // initializes all connected processors
  m_networkEvaluator->initializeNetwork();

  //get objects size
  updateVolumeBoundBox();
  updateSwcBoundBox();
  updatePunctaBoundBox();
  updateGraphBoundBox();
  updateOverallBoundBox();

  // adjust camera
  resetCamera();
  m_volumeRaycaster->getCamera()->dependsOn(m_compositor->getCamera());

  connect(getInteractionHandler(), SIGNAL(cameraMoved()), this, SLOT(resetCameraClippingRange()));
  connect(getInteractionHandler(), SIGNAL(objectsMoved(double,double,double)), this,
          SLOT(moveSelectedObjects(double,double,double)));

  //  // if have image, try black background
  //  if (channelNumber() > 0) {
  //    m_background->setFirstColor(glm::vec3(0.f));
  //    m_background->setSecondColor(glm::vec3(0.f));
  //  }
}

void Z3DWindow::setWindowSize()
{
  int width = 512;
  int height = 512;

  float objectWidth = m_boundBox[1] - m_boundBox[0];
  float objectHeight = m_boundBox[3] - m_boundBox[2];

  //get screen size
  QDesktopWidget *desktop = QApplication::desktop();
  QRect screenSize = desktop->availableGeometry();
  float screenWidth = screenSize.width() * 0.6;
  float screenHeight = screenSize.height() * 0.6;

  if (objectWidth > screenWidth || objectHeight > screenHeight) {
    float scale = std::max(objectWidth/screenWidth, objectHeight/screenHeight);
    width = std::max(width, (int)(objectWidth/scale));
    height = std::max(height, (int)(objectHeight/scale));
  } else {
    width = std::max(width, (int)(objectWidth));
    height = std::max(height, (int)(objectHeight));
  }
  resize(width+500, height);   //500 for dock widgets
}

void Z3DWindow::createActions()
{
  /*
  m_undoAction = m_doc->undoStack()->createUndoAction(this, tr("&Undo"));
  m_undoAction->setIcon(QIcon(":/images/undo.png"));
  m_undoAction->setShortcuts(QKeySequence::Undo);

  m_redoAction = m_doc->undoStack()->createRedoAction(this, tr("&Redo"));
  m_redoAction->setIcon(QIcon(":/images/redo.png"));
  m_redoAction->setShortcuts(QKeySequence::Redo);
  */

  m_undoAction = m_doc->getUndoAction();
  m_redoAction = m_doc->getRedoAction();

  m_removeSelectedObjectsAction = new QAction("Delete", this);
  if (NeutubeConfig::getInstance().getApplication() != "Biocytin") {
    connect(m_removeSelectedObjectsAction, SIGNAL(triggered()), this,
            SLOT(removeSelectedObject()));
  } else {
    connect(m_removeSelectedObjectsAction, SIGNAL(triggered()), this,
        SLOT(deleteSelectedSwcNode()));
  }

  m_locateSwcNodeIn2DAction = new QAction("Locate node in 2D View", this);
  connect(m_locateSwcNodeIn2DAction, SIGNAL(triggered()), this,
          SLOT(locateSwcNodeIn2DView()));

  m_toogleAddSwcNodeModeAction = new QAction("Click to add swc node", this);
  m_toogleAddSwcNodeModeAction->setCheckable(true);
  connect(m_toogleAddSwcNodeModeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleAddSwcNodeMode(bool)));

  m_toogleMoveSelectedObjectsAction = new QAction("Shift+Mouse to move selected objects", this);
  m_toogleMoveSelectedObjectsAction->setCheckable(true);
  connect(m_toogleMoveSelectedObjectsAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleMoveSelectedObjectsMode(bool)));

  m_toogleExtendSelectedSwcNodeAction = new QAction("Click to extend selected node", this);
  m_toogleExtendSelectedSwcNodeAction->setCheckable(true);
  connect(m_toogleExtendSelectedSwcNodeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleExtendSelectedSwcNodeMode(bool)));

  m_toogleSmartExtendSelectedSwcNodeAction = new QAction("Click to smart extend selected node", this);
  m_toogleSmartExtendSelectedSwcNodeAction->setCheckable(true);
  connect(m_toogleSmartExtendSelectedSwcNodeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleSmartExtendSelectedSwcNodeMode(bool)));

  m_changeSwcNodeTypeAction = new QAction("Change type", this);
  connect(m_changeSwcNodeTypeAction, SIGNAL(triggered()),
          this, SLOT(changeSelectedSwcNodeType()));

  m_setSwcRootAction = new QAction("Set as root", this);
  connect(m_setSwcRootAction, SIGNAL(triggered()),
          this, SLOT(setRootAsSelectedSwcNode()));

  m_breakSwcConnectionAction = new QAction("Break", this);
  connect(m_breakSwcConnectionAction, SIGNAL(triggered()), this,
          SLOT(breakSelectedSwcNode()));

  m_connectSwcNodeAction = new QAction("Connect", this);
  connect(m_connectSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(connectSelectedSwcNode()));

  m_connectToSwcNodeAction = new QAction("Connect to", this);
  connect(m_connectToSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(startConnectingSwcNode()));

  m_mergeSwcNodeAction = new QAction("Merge", this);
  connect(m_mergeSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(mergeSelectedSwcNode()));



  m_selectSwcConnectionAction = new QAction("Select Connection", this);
  connect(m_selectSwcConnectionAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectSwcNodeConnection()));
/*
  m_selectSwcNodeUpstreamAction = new QAction("Upstream", this);
  connect(m_selectSwcNodeUpstreamAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectUpstreamNode()));
          */
  /*
  m_selectSwcNodeDownstreamAction = new QAction("Downstream", this);
  connect(m_selectSwcNodeDownstreamAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectDownstreamNode()));
          */

  m_selectSwcNodeDownstreamAction =
      m_doc->getAction(ZStackDoc::ACTION_SELECT_DOWNSTREAM);

  m_selectSwcNodeUpstreamAction =
      m_doc->getAction(ZStackDoc::ACTION_SELECT_UPSTREAM);

  m_selectSwcNodeBranchAction = new QAction("Branch", this);
  connect(m_selectSwcNodeBranchAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectBranchNode()));

  m_selectSwcNodeTreeAction = new QAction("Tree", this);
  connect(m_selectSwcNodeTreeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectTreeNode()));

  m_selectAllConnectedSwcNodeAction = new QAction("All Connected Nodes", this);
  connect(m_selectAllConnectedSwcNodeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectConnectedNode()));

  m_selectAllSwcNodeAction = new QAction("All Nodes", this);
  connect(m_selectAllSwcNodeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectAllSwcTreeNode()));

  m_translateSwcNodeAction = new QAction("Translate", this);
  connect(m_translateSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(tranlateSelectedSwcNode()));

  m_changeSwcNodeSizeAction = new QAction("Change size", this);
  connect(m_changeSwcNodeSizeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcNodeSize()));

  m_saveSwcAction = new QAction("Save as", this);
  connect(m_saveSwcAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedSwc()));

  m_changeSwcTypeAction = new QAction("Change type", this);
  connect(m_changeSwcTypeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcType()));

  m_changeSwcSizeAction = new QAction("Change size", this);
  connect(m_changeSwcSizeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcSize()));

  m_transformSwcAction = new QAction("Transform skeleton", this);
  connect(m_transformSwcAction, SIGNAL(triggered()), this,
          SLOT(transformSelectedSwc()));

  m_groupSwcAction = new QAction("Group", this);
  connect(m_groupSwcAction, SIGNAL(triggered()), this,
          SLOT(groupSelectedSwc()));

  m_breakForestAction = new QAction("Break forest", this);
  connect(m_breakForestAction, SIGNAL(triggered()), this,
          SLOT(breakSelectedSwc()));

  m_changeSwcColorAction = new QAction("Change color", this);
  connect(m_changeSwcColorAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcColor()));

  m_changeSwcAlphaAction = new QAction("Change transparency", this);
  connect(m_changeSwcAlphaAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcAlpha()));

  m_swcInfoAction = new QAction("Get Info", this);
  connect(m_swcInfoAction, SIGNAL(triggered()), this,
          SLOT(showSelectedSwcInfo()));

  m_swcNodeLengthAction = new QAction("Calculate length", this);
  connect(m_swcNodeLengthAction, SIGNAL(triggered()), this,
          SLOT(showSeletedSwcNodeLength()));

  m_refreshTraceMaskAction = new QAction("Refresh tracing mask", this);
  connect(m_refreshTraceMaskAction, SIGNAL(triggered()), this,
          SLOT(refreshTraceMask()));

  m_removeSwcTurnAction = new QAction("Remove turn", this);
  connect(m_removeSwcTurnAction, SIGNAL(triggered()),
          this, SLOT(removeSwcTurn()));

  m_resolveCrossoverAction = new QAction("Resolve crossover", this);
  connect(m_resolveCrossoverAction, SIGNAL(triggered()),
          m_doc.get(), SLOT(executeResolveCrossoverCommand()));
}

void Z3DWindow::createMenus()
{
  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_editMenu = menuBar()->addMenu(tr("&Edit"));
  m_editMenu->addAction(m_undoAction);
  m_editMenu->addAction(m_redoAction);

  createContextMenu();
  customizeContextMenu();
}

void Z3DWindow::createContextMenu()
{
  QMenu *contextMenu = new QMenu(this);
  //  contextMenu->addAction("mark selected Puncta Property 1",
  //                         this, SLOT(markSelectedPunctaProperty1()));
  //  contextMenu->addAction("mark selected Puncta Property 2",
  //                         this, SLOT(markSelectedPunctaProperty2()));
  //  contextMenu->addAction("mark selected Puncta Property 3",
  //                         this, SLOT(markSelectedPunctaProperty3()));
  //  contextMenu->addAction("unmark selected Puncta Property 1",
  //                         this, SLOT(unmarkSelectedPunctaProperty1()));
  //  contextMenu->addAction("unmark selected Puncta Property 2",
  //                         this, SLOT(unmarkSelectedPunctaProperty2()));
  //  contextMenu->addAction("unmark selected Puncta Property 3",
  //                         this, SLOT(unmarkSelectedPunctaProperty3()));
  m_saveSelectedPunctaAsAction = new QAction("save selected puncta as ...", this);
  connect(m_saveSelectedPunctaAsAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedPunctaAs()));
  m_saveAllPunctaAsAction = new QAction("save all puncta as ...", this);
  connect(m_saveAllPunctaAsAction, SIGNAL(triggered()), this,
          SLOT(saveAllPunctaAs()));
  m_locatePunctumIn2DAction = new QAction("Locate punctum in 2D View", this);
   connect(m_locatePunctumIn2DAction, SIGNAL(triggered()), this,
           SLOT(locatePunctumIn2DView()));
  contextMenu->addAction(m_saveSelectedPunctaAsAction);
  contextMenu->addAction(m_saveAllPunctaAsAction);
  contextMenu->addAction(m_locatePunctumIn2DAction);
  contextMenu->addAction("Transform selected puncta",
                         this, SLOT(transformSelectedPuncta()));
  contextMenu->addAction("Transform all puncta",
                         this, SLOT(transformAllPuncta()));
  contextMenu->addAction("Convert to swc",
                         this, SLOT(convertPunctaToSwc()));
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toogleMoveSelectedObjectsAction);
  m_contextMenuGroup["puncta"] = contextMenu;

  //Swc node
  contextMenu = new QMenu(this);
  QMenu *selectMenu = new QMenu("Select", contextMenu);
  contextMenu->addAction(m_changeSwcNodeTypeAction);
  contextMenu->addAction(m_setSwcRootAction);
  contextMenu->addAction(m_breakSwcConnectionAction);
  contextMenu->addAction(m_connectToSwcNodeAction);
  contextMenu->addAction(m_connectSwcNodeAction);
  contextMenu->addAction(m_mergeSwcNodeAction);
  selectMenu->addAction(m_selectSwcNodeDownstreamAction);
  selectMenu->addAction(m_selectSwcConnectionAction);
  selectMenu->addAction(m_selectSwcNodeUpstreamAction);
  selectMenu->addAction(m_selectSwcNodeBranchAction);
  selectMenu->addAction(m_selectSwcNodeTreeAction);
  selectMenu->addAction(m_selectAllConnectedSwcNodeAction);
  selectMenu->addAction(m_selectAllSwcNodeAction);
  contextMenu->addMenu(selectMenu);
  contextMenu->addAction(m_locateSwcNodeIn2DAction);
  contextMenu->addAction(m_translateSwcNodeAction);
  contextMenu->addAction(m_changeSwcNodeSizeAction);
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toogleExtendSelectedSwcNodeAction);
  contextMenu->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
  contextMenu->addAction(m_toogleAddSwcNodeModeAction);
  contextMenu->addAction(m_toogleMoveSelectedObjectsAction);
  contextMenu->addAction(m_removeSwcTurnAction);
  contextMenu->addAction(m_resolveCrossoverAction);
  contextMenu->addAction(m_swcNodeLengthAction);
  m_contextMenuGroup["swcnode"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction(m_saveSwcAction);
  contextMenu->addAction(m_changeSwcTypeAction);
  contextMenu->addAction(m_changeSwcSizeAction);
  contextMenu->addAction(m_transformSwcAction);
  contextMenu->addAction(m_groupSwcAction);
  contextMenu->addAction(m_breakForestAction);
  contextMenu->addAction(m_changeSwcColorAction);
  contextMenu->addAction(m_changeSwcAlphaAction);
  contextMenu->addAction(m_swcInfoAction);

#ifdef _DEBUG_2
  contextMenu->addAction("Test", this, SLOT(test()));
#endif
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toogleMoveSelectedObjectsAction);
  m_contextMenuGroup["swc"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction("Accept", this, SLOT(convertSelectedChainToSwc()));
  contextMenu->addAction(m_removeSelectedObjectsAction);
  m_contextMenuGroup["chain"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction("Trace", this, SLOT(traceTube()));
  //contextMenu->addAction("Trace Exp", this, SLOT(traceTube_Exp()));
  m_contextMenuGroup["trace"] = contextMenu;

  contextMenu = new QMenu(this);
  m_openVolumeZoomInViewAction = new QAction("Open Zoom In View", this);
  connect(m_openVolumeZoomInViewAction, SIGNAL(triggered()), this,
          SLOT(openZoomInView()));
  m_exitVolumeZoomInViewAction = new QAction("Exit Zoom In View", this);
  connect(m_exitVolumeZoomInViewAction, SIGNAL(triggered()), this,
          SLOT(exitZoomInView()));
  m_markPunctumAction = new QAction("Mark Punctum", this);
  connect(m_markPunctumAction, SIGNAL(triggered()), this,
          SLOT(markPunctum()));
  m_contextMenuGroup["volume"] = contextMenu;

  contextMenu = new QMenu(this);
  m_changeBackgroundAction = new QAction("Change Background", this);
  connect(m_changeBackgroundAction, SIGNAL(triggered()), this,
          SLOT(changeBackground()));
  m_contextMenuGroup["empty"] = contextMenu;
}

void Z3DWindow::customizeContextMenu()
{
  //Need modification
  m_selectSwcNodeTreeAction->setVisible(false);

  if (GET_APPLICATION_NAME == "Biocytin") {
    m_toogleAddSwcNodeModeAction->setVisible(false);
    m_toogleMoveSelectedObjectsAction->setVisible(false);
    m_toogleExtendSelectedSwcNodeAction->setVisible(false);
    m_toogleSmartExtendSelectedSwcNodeAction->setVisible(false);
    m_translateSwcNodeAction->setVisible(false);
    //m_selectSwcNodeDownstreamAction->setVisible(false);
    //m_changeSwcNodeTypeAction->setVisible(false);
    m_mergeSwcNodeAction->setVisible(false);
    m_changeSwcNodeSizeAction->setVisible(false);
    //m_setSwcRootAction->setVisible(false);
    //m_changeSwcTypeAction->setVisible(false);
    m_changeSwcSizeAction->setVisible(false);
    m_transformSwcAction->setVisible(false);
    m_changeSwcColorAction->setVisible(false);
    m_changeSwcAlphaAction->setVisible(false);
    m_refreshTraceMaskAction->setVisible(false);
  } else if (GET_APPLICATION_NAME == "FlyEM") {
    m_toogleExtendSelectedSwcNodeAction->setVisible(false);
    m_toogleAddSwcNodeModeAction->setVisible(false);
    //m_toogleMoveSelectedObjectsAction->setVisible(false);
    m_toogleSmartExtendSelectedSwcNodeAction->setVisible(false);
  }
}

Z3DVolumeRaycasterRenderer* Z3DWindow::getVolumeRaycasterRenderer() {
  return m_volumeRaycaster->getRenderer();
}

void Z3DWindow::createDockWindows()
{
  m_settingsDockWidget = new QDockWidget(tr("Control and Settings"), this);
  m_settingsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);

  m_widgetsGroup = new ZWidgetsGroup("All", NULL, 1);
  // reset camera button and some other utils
  QPushButton *resetCameraButton = new QPushButton(tr("Reset Camera"));
  resetCameraButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(resetCameraButton, SIGNAL(clicked()), this, SLOT(resetCamera()));

#ifdef _FLYEM_
  QPushButton *flipViewButton = new QPushButton(tr("Back view"));
  flipViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(flipViewButton, SIGNAL(clicked()), this, SLOT(flipView()));

#  ifdef _DEBUG_
  QPushButton *recordViewButton = new QPushButton(tr("Record view"));
  recordViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(recordViewButton, SIGNAL(clicked()), this, SLOT(recordView()));

  QPushButton *diffViewButton = new QPushButton(tr("Calculate view difference"));
  diffViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(diffViewButton, SIGNAL(clicked()), this, SLOT(diffView()));
#  endif

#endif

  const NeutubeConfig &config = NeutubeConfig::getInstance();


  if (config.getZ3DWindowConfig().isUtilsOn()) {
    ZWidgetsGroup *utils = new ZWidgetsGroup("Utils", m_widgetsGroup, 1);
    new ZWidgetsGroup(resetCameraButton, utils, 1);
#ifdef _FLYEM_
    new ZWidgetsGroup(flipViewButton, utils, 1);

#  ifdef _DEBUG_
    new ZWidgetsGroup(recordViewButton, utils, 1);
    new ZWidgetsGroup(diffViewButton, utils, 1);
#  endif
#endif
    new ZWidgetsGroup(getCamera(), utils, 1);
    new ZWidgetsGroup(m_compositor->getParameter("Multisample Anti-Aliasing"), utils, 1);
    new ZWidgetsGroup(m_compositor->getParameter("Transparency"), utils, 1);
  }

  // capture function
  m_screenShotWidget = new Z3DTakeScreenShotWidget(false, this);
  m_screenShotWidget->setCaptureStereoImage(m_isStereoView);
  connect(m_screenShotWidget, SIGNAL(takeScreenShot(QString, Z3DScreenShotType)),
          this, SLOT(takeScreenShot(QString, Z3DScreenShotType)));
  connect(m_screenShotWidget, SIGNAL(takeScreenShot(QString,int,int,Z3DScreenShotType)),
          this, SLOT(takeScreenShot(QString,int,int,Z3DScreenShotType)));
  connect(m_screenShotWidget, SIGNAL(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,Z3DScreenShotType)),
          this, SLOT(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,Z3DScreenShotType)));
  connect(m_screenShotWidget, SIGNAL(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,int,int,Z3DScreenShotType)),
          this, SLOT(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,int,int,Z3DScreenShotType)));
  ZWidgetsGroup *capture = new ZWidgetsGroup("Capture", m_widgetsGroup, 1);
  new ZWidgetsGroup(m_screenShotWidget, capture, 1);

  //volume
  if (config.getZ3DWindowConfig().isVolumeOn()) {
    if (m_volumeSource != NULL) {
      ZWidgetsGroup *wg = m_volumeRaycaster->getWidgetsGroup();
      wg->mergeGroup(m_volumeSource->getWidgetsGroup(), false);
      connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
      m_widgetsGroup->addChildGroup(wg);
    }
  }

  ZWidgetsGroup *wg = NULL;

  if (config.getZ3DWindowConfig().isGraphOn()) {
#if defined(_FLYEM_)
    //graph
    wg = m_graphFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
#endif
  }

  if (config.getZ3DWindowConfig().isSwcsOn()) {
    //swc
    wg = m_swcFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)),
            this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
  }

#if !defined(_NEUTUBE_LIGHT_)
  //puncta
  if (config.getZ3DWindowConfig().isPunctaOn()) {
    wg = m_punctaFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
  }
#endif

  //background
  wg = m_compositor->getBackgroundWidgetsGroup();
  connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
  m_widgetsGroup->addChildGroup(wg);

  //axis
  wg = m_axis->getWidgetsGroup();
  connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
  m_widgetsGroup->addChildGroup(wg);

  //QWidget *widget = m_widgetsGroup->createWidget(this, true);

  //m_settingsDockWidget->setWidget(widget);

  QTabWidget *tabs = createBasicSettingTabWidget();
  m_settingsDockWidget->setWidget(tabs);

  addDockWidget(Qt::RightDockWidgetArea, m_settingsDockWidget);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_settingsDockWidget->toggleViewAction());
  connect(m_widgetsGroup, SIGNAL(widgetsGroupChanged()), this, SLOT(updateSettingsDockWidget()));

  m_objectsDockWidget = new QDockWidget(tr("Objects Manager"), this);
  m_objectsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  ZObjsManagerWidget* omw = new ZObjsManagerWidget(getDocument(), m_objectsDockWidget);
  connect(omw, SIGNAL(swcDoubleClicked(ZSwcTree*)), this, SLOT(swcDoubleClicked(ZSwcTree*)));
  connect(omw, SIGNAL(swcNodeDoubleClicked(Swc_Tree_Node*)), this, SLOT(swcNodeDoubleClicked(Swc_Tree_Node*)));
  connect(omw, SIGNAL(punctaDoubleClicked(ZPunctum*)), this, SLOT(punctaDoubleClicked(ZPunctum*)));
  m_objectsDockWidget->setWidget(omw);
  addDockWidget(Qt::RightDockWidgetArea, m_objectsDockWidget);
  m_viewMenu->addAction(m_objectsDockWidget->toggleViewAction());

  customizeDockWindows(tabs);
}

int Z3DWindow::channelNumber()
{
  if (m_volumeSource == NULL) {
    return 0;
  }

  if (m_volumeSource->isEmpty()) {
    return 0;
  }

  if (m_doc == NULL) {
    return 0;
  }

  if (m_doc->hasStackData()) {
    return m_doc->stack()->channelNumber();
  } else {
    return 0;
  }
}

void Z3DWindow::customizeDockWindows(QTabWidget *m_settingTab)
{
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_settingTab->setCurrentIndex(1);
  }
}

bool Z3DWindow::hasVolume()
{
  return channelNumber() > 0;
}

void Z3DWindow::resetCamera()
{
  setupCamera(m_boundBox, Z3DCamera::ResetAll);
}

void Z3DWindow::setupCamera(const std::vector<double> &bound,
                            Z3DCamera::ResetCameraOptions options)
{
  getCamera()->resetCamera(bound, options);
}

void Z3DWindow::flipView()
{
  getCamera()->flipViewDirection();
}

void Z3DWindow::recordView()
{
  m_cameraRecord = getCamera()->get();
}

void Z3DWindow::diffView()
{
  std::cout << "Eye: " << getCamera()->getEye() - m_cameraRecord.getEye()
            << std::endl;
  std::cout << "Center: "
            << getCamera()->getCenter() - m_cameraRecord.getCenter() << std::endl;
  std::cout << "Up vector: "
            << getCamera()->getUpVector() - m_cameraRecord.getUpVector() << std::endl;
}

void Z3DWindow::resetCameraClippingRange()
{
  getCamera()->resetCameraNearFarPlane(m_boundBox);
}

void Z3DWindow::updateVolumeBoundBox()
{
  m_volumeBoundBox[0] = m_volumeBoundBox[2] = m_volumeBoundBox[4] = std::numeric_limits<double>::max();
  m_volumeBoundBox[1] = m_volumeBoundBox[3] = m_volumeBoundBox[5] = -std::numeric_limits<double>::max();
  if (hasVolume()) {
    m_volumeBoundBox = m_volumeSource->getVolume(0)->getWorldBoundBox();
  }
}

void Z3DWindow::updateSwcBoundBox()
{
  m_swcBoundBox[0] = m_swcBoundBox[2] = m_swcBoundBox[4] = std::numeric_limits<double>::max();
  m_swcBoundBox[1] = m_swcBoundBox[3] = m_swcBoundBox[5] = -std::numeric_limits<double>::max();
  for (int i=0; i<m_doc->swcList()->size(); i++) {
    std::vector<double> boundBox = m_swcFilter->getTreeBound(m_doc->swcList()->at(i));
    m_swcBoundBox[0] = std::min(boundBox[0], m_swcBoundBox[0]);
    m_swcBoundBox[1] = std::max(boundBox[1], m_swcBoundBox[1]);
    m_swcBoundBox[2] = std::min(boundBox[2], m_swcBoundBox[2]);
    m_swcBoundBox[3] = std::max(boundBox[3], m_swcBoundBox[3]);
    m_swcBoundBox[4] = std::min(boundBox[4], m_swcBoundBox[4]);
    m_swcBoundBox[5] = std::max(boundBox[5], m_swcBoundBox[5]);
  }
}

void Z3DWindow::updateGraphBoundBox()
{
  m_graphBoundBox = m_graphFilter->boundBox();
}

void Z3DWindow::updatePunctaBoundBox()
{
  m_punctaBoundBox[0] = m_punctaBoundBox[2] = m_punctaBoundBox[4] = std::numeric_limits<double>::max();
  m_punctaBoundBox[1] = m_punctaBoundBox[3] = m_punctaBoundBox[5] = -std::numeric_limits<double>::max();
  for (int i=0; i<m_doc->punctaList()->size(); i++) {
    std::vector<double> boundBox = m_punctaFilter->getPunctumBound(m_doc->punctaList()->at(i));
    m_punctaBoundBox[0] = std::min(boundBox[0], m_punctaBoundBox[0]);
    m_punctaBoundBox[1] = std::max(boundBox[1], m_punctaBoundBox[1]);
    m_punctaBoundBox[2] = std::min(boundBox[2], m_punctaBoundBox[2]);
    m_punctaBoundBox[3] = std::max(boundBox[3], m_punctaBoundBox[3]);
    m_punctaBoundBox[4] = std::min(boundBox[4], m_punctaBoundBox[4]);
    m_punctaBoundBox[5] = std::max(boundBox[5], m_punctaBoundBox[5]);
  }
}

Z3DCameraParameter *Z3DWindow::getCamera()
{
  return m_compositor->getCamera();
}

Z3DTrackballInteractionHandler *Z3DWindow::getInteractionHandler()
{
  return m_compositor->getInteractionHandler();
}

void Z3DWindow::cleanup()
{
  if (!m_isClean) {
    delete m_widgetsGroup;
    m_widgetsGroup = NULL;
    m_canvas->setNetworkEvaluator(NULL);
    m_networkEvaluator->deinitializeNetwork();
    delete m_networkEvaluator;
    m_networkEvaluator = NULL;

    delete m_volumeSource;
    m_volumeSource = NULL;
    delete m_volumeRaycaster;
    m_volumeRaycaster = NULL;
    delete m_punctaFilter;
    m_punctaFilter = NULL;
    delete m_swcFilter;
    m_swcFilter = NULL;
    delete m_graphFilter;
    m_graphFilter = NULL;
    delete m_compositor;
    m_compositor = NULL;
    delete m_axis;
    m_axis = NULL;
    delete m_canvasRenderer;
    m_canvasRenderer = NULL;

    delete m_canvas;
    m_isClean = true;
  }
}

void Z3DWindow::volumeChanged()
{
  if (m_volumeSource == NULL) {
    m_volumeSource = new Z3DVolumeSource(getDocument());
  }

  m_volumeSource->reloadVolume();
  updateVolumeBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::swcChanged()
{
  m_swcFilter->setData(m_doc->swcList());
  updateSwcBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::updateNetworkDisplay()
{
  if (m_doc->swcNetwork() != NULL) {
    ZPointNetwork *network = m_doc->swcNetwork()->toPointNetwork();
    m_graphFilter->setData(*network, NULL);

    delete network;
  }
}

void Z3DWindow::updateDisplay()
{
  volumeChanged();
  swcChanged();
  punctaChanged();
  updateNetworkDisplay();
}

void Z3DWindow::punctaChanged()
{
  m_punctaFilter->setData(m_doc->punctaList());
  updatePunctaBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::volumeScaleChanged()
{
  if (!m_doc->hasStackData())
    return;
  updateVolumeBoundBox();
  updateOverallBoundBox();
  //setupCamera(m_boundBox, Z3DCamera::PreserveCenterDistance | Z3DCamera::PreserveViewVector);
  resetCameraClippingRange();
}

void Z3DWindow::swcCoordScaleChanged()
{
  if (m_doc->swcList()->empty())
    return;
  updateSwcBoundBox();
  updateOverallBoundBox();
  //setupCamera(m_boundBox, Z3DCamera::PreserveCenterDistance | Z3DCamera::PreserveViewVector);
  resetCameraClippingRange();
}

void Z3DWindow::punctaCoordScaleChanged()
{
  if (m_doc->punctaList()->empty())
    return;
  updatePunctaBoundBox();
  updateOverallBoundBox();
  //setupCamera(m_boundBox, Z3DCamera::PreserveCenterDistance | Z3DCamera::PreserveViewVector);
  resetCameraClippingRange();
}

void Z3DWindow::swcSizeScaleChanged()
{
  updateSwcBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::punctaSizeScaleChanged()
{
  updatePunctaBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::selectedPunctumChangedFrom3D(ZPunctum *p, bool append)
{
  if (p == NULL) {
    if (!append)
      m_doc->deselectAllPuncta();
    return;
  }

  if (append) {
    m_doc->setPunctumSelected(p, true);
  } else {
    m_doc->deselectAllObject();
    m_doc->setPunctumSelected(p, true);
  }

  statusBar()->showMessage(p->toString().c_str());
}

void Z3DWindow::selectedSwcChangedFrom3D(ZSwcTree *p, bool append)
{
  if (p == NULL) {
    if (!append)
      m_doc->deselectAllSwcs();
    return;
  }

  if (append) {
    m_doc->setSwcSelected(p, true);
  } else {
    m_doc->deselectAllObject();
    m_doc->setSwcSelected(p, true);
  }

  statusBar()->showMessage(p->source().c_str());
}

void Z3DWindow::selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node *p, bool append)
{
  if (p == NULL) {
    if (!append) {
      m_doc->deselectAllSwcTreeNodes();
    }
    return;
  }

  if (!append) {
    m_doc->deselectAllObject();
  }

  if (m_doc->selectedSwcTreeNodes()->count(p) == 0) {
    m_doc->setSwcTreeNodeSelected(p, true);
  } else {
    m_doc->setSwcTreeNodeSelected(p, false);
  }

  statusBar()->showMessage(SwcTreeNode::toString(p).c_str());
}

void Z3DWindow::addNewSwcTreeNode(double x, double y, double z, double r)
{
  m_doc->executeAddSwcNodeCommand(ZPoint(x, y, z), r);
      /*
  QUndoCommand *insertNewSwcTreeNodeCommand =
      new ZStackDocAddSwcNodeCommand(m_doc.get(), p);
  m_doc->undoStack()->push(insertNewSwcTreeNodeCommand);
      */
}

void Z3DWindow::extendSwcTreeNode(double x, double y, double z)
{
  m_doc->executeSwcNodeExtendCommand(ZPoint(x, y, z));
  /*
  QUndoCommand *extendSwcTreeNodeCommand =
      new ZStackDocExtendSwcNodeCommand(m_doc.get(), node, parentNode);
  m_doc->undoStack()->push(extendSwcTreeNodeCommand);
  */
}

void Z3DWindow::removeSwcTurn()
{
  m_doc->executeRemoveTurnCommand();
}

void Z3DWindow::startConnectingSwcNode()
{
  getSwcFilter()->setInteractionMode(Z3DSwcFilter::ConnectSwcNode);
  m_canvas->setCursor(Qt::PointingHandCursor);
}

void Z3DWindow::connectSwcTreeNode(Swc_Tree_Node *tn)
{
  if (!getDocument()->selectedSwcTreeNodes()->empty()) {
    Swc_Tree_Node *target = SwcTreeNode::findClosestNode(
          *getDocument()->selectedSwcTreeNodes(), tn);
    m_doc->executeConnectSwcNodeCommand(target, tn);
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->setCursor(Qt::ArrowCursor);
  }
}

void Z3DWindow::punctaSelectionChanged()
{
  m_punctaFilter->invalidate();
}

void Z3DWindow::swcSelectionChanged()
{
  m_swcFilter->invalidate();
}

void Z3DWindow::swcTreeNodeSelectionChanged()
{
  m_swcFilter->invalidate();
}

void Z3DWindow::swcDoubleClicked(ZSwcTree *tree)
{
  std::vector<double> boundBox = m_swcFilter->getTreeBound(tree);
  gotoPosition(boundBox, 0);
}

void Z3DWindow::swcNodeDoubleClicked(Swc_Tree_Node *node)
{
  std::vector<double> boundBox = m_swcFilter->getTreeNodeBound(node);
  gotoPosition(boundBox, 0);
}

void Z3DWindow::punctaDoubleClicked(ZPunctum *p)
{
  std::vector<double> boundBox = m_punctaFilter->getPunctumBound(p);
  if (hasVolume() > 0) {
    if (m_volumeSource->isSubvolume())
      m_volumeSource->exitZoomInView();
  }
  gotoPosition(boundBox);
}

void Z3DWindow::pointInVolumeLeftClicked(QPoint pt, glm::ivec3 pos)
{
  glm::vec3 fpos = glm::vec3(pos);
  fpos /= glm::vec3(m_volumeSource->getXScale(), m_volumeSource->getYScale(), m_volumeSource->getZScale());
  m_lastClickedPosInVolume = glm::ivec3(fpos);
  LDEBUG() << "Point in volume left clicked" << fpos;
  // only do tracing when we are not editing swc nodes or the preconditions for editing swc node are not met
  if (hasVolume() && channelNumber() == 1 && m_toogleSmartExtendSelectedSwcNodeAction->isChecked() &&
      m_doc->selectedSwcTreeNodes()->size() == 1) {
    m_doc->executeSwcNodeSmartExtendCommand(ZPoint(fpos[0], fpos[1], fpos[2]));
    return;
  }
  if (m_toogleExtendSelectedSwcNodeAction->isChecked() && m_doc->selectedSwcTreeNodes()->size() == 1) {
    return;
  }
  if (hasVolume() && channelNumber() == 1 && !m_toogleAddSwcNodeModeAction->isChecked()) {
    m_contextMenuGroup["trace"]->popup(m_canvas->mapToGlobal(pt));
  }
}

void Z3DWindow::show3DViewContextMenu(QPoint pt)
{
  QList<QAction*> actions;

  if (m_doc->selectedSwcTreeNodes()->size() > 0) {
    updateContextMenu("swcnode");
    //m_contextMenuGroup["swcnode"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["swcnode"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  }

  if (m_doc->selectedSwcs()->size() > 0) {
    //m_contextMenuGroup["swc"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["swc"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  }

  if (m_doc->selectedPuncta()->size() > 0) {
    updateContextMenu("puncta");
    //m_contextMenuGroup["puncta"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["puncta"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  }

  if (m_doc->selectedPuncta()->empty() &&
      m_doc->selectedSwcs()->empty() &&
      m_doc->selectedSwcTreeNodes()->empty()) {

    // first see if pt hit any position in volume
    if (channelNumber() > 0) {
      bool success;
      glm::vec3 fpos = m_volumeRaycaster->getFirstHit3DPosition(
            pt.x(), pt.y(), m_canvas->width(), m_canvas->height(), success);
      if (success) {
        fpos /= glm::vec3(m_volumeSource->getXScale(),
                          m_volumeSource->getYScale(),
                          m_volumeSource->getZScale());
        m_lastClickedPosInVolume = glm::ivec3(fpos);
        updateContextMenu("volume");
        //m_contextMenuGroup["volume"]->popup(m_canvas->mapToGlobal(pt));
        QList<QAction*> acts = m_contextMenuGroup["volume"]->actions();
        if (actions.empty()) {
          actions = acts;
        } else {
          for (int i=0; i<acts.size(); ++i)
            if (!actions.contains(acts[i]))
              actions.push_back(acts[i]);
        }
        if (!actions.empty()) {
          m_mergedContextMenu->clear();
          m_mergedContextMenu->addActions(actions);
          m_mergedContextMenu->popup(m_canvas->mapToGlobal(pt));
        }
        return;
      }
    }

    updateContextMenu("empty");
    //m_contextMenuGroup["empty"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["empty"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      for (int i=0; i<acts.size(); ++i)
        if (!actions.contains(acts[i]))
          actions.push_back(acts[i]);
    }
  }

  if (!actions.empty()) {
    m_mergedContextMenu->clear();
    m_mergedContextMenu->addActions(actions);
    m_mergedContextMenu->popup(m_canvas->mapToGlobal(pt));
  }
}

void Z3DWindow::traceTube()
{
  /*
  m_doc->executeTraceTubeCommand(m_lastClickedPosInVolume[0],
      m_lastClickedPosInVolume[1],
      m_lastClickedPosInVolume[2]);
      */
  m_doc->executeTraceSwcBranchCommand(m_lastClickedPosInVolume[0],
      m_lastClickedPosInVolume[1],
      m_lastClickedPosInVolume[2]);
}

void Z3DWindow::openZoomInView()
{
  if (hasVolume()) {
    if (m_volumeSource->openZoomInView(m_lastClickedPosInVolume)) {
      gotoPosition(m_volumeSource->getZoomInBound());
    }
  }
}

void Z3DWindow::exitZoomInView()
{
  if (hasVolume()) {
    m_volumeSource->exitZoomInView();
  }
}

void Z3DWindow::removeSelectedObject()
{
  m_doc->executeRemoveObjectCommand();
#if 0
  if (!m_doc->selectedSwcTreeNodes()->empty()) {
    m_doc->executeDeleteSwcNodeCommand();
  }

  if (!(m_doc->selectedChains()->empty() && m_doc->selectedPuncta()->empty() &&
        m_doc->selectedSwcs()->empty() )) {
    QUndoCommand *removeSelectedObjectsCommand =
        new ZStackDocRemoveSelectedObjectCommand(m_doc.get());
    m_doc->undoStack()->push(removeSelectedObjectsCommand);
  }
#endif
}

void Z3DWindow::markSelectedPunctaProperty1()
{
  for (std::set<ZPunctum*>::iterator it=m_doc->selectedPuncta()->begin(); it != m_doc->selectedPuncta()->end(); it++) {
    (*it)->setProperty1("true");
    m_doc->punctaObjsModel()->updateData(*it);
  }
}

void Z3DWindow::markSelectedPunctaProperty2()
{
  for (std::set<ZPunctum*>::iterator it=m_doc->selectedPuncta()->begin(); it != m_doc->selectedPuncta()->end(); it++) {
    (*it)->setProperty2("true");
    m_doc->punctaObjsModel()->updateData(*it);
  }
}

void Z3DWindow::markSelectedPunctaProperty3()
{
  for (std::set<ZPunctum*, int>::iterator it=m_doc->selectedPuncta()->begin(); it != m_doc->selectedPuncta()->end(); it++) {
    (*it)->setProperty3("true");
    m_doc->punctaObjsModel()->updateData(*it);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty1()
{
  for (std::set<ZPunctum*>::iterator it=m_doc->selectedPuncta()->begin(); it != m_doc->selectedPuncta()->end(); it++) {
    (*it)->setProperty1("");
    m_doc->punctaObjsModel()->updateData(*it);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty2()
{
  for (std::set<ZPunctum*>::iterator it=m_doc->selectedPuncta()->begin(); it != m_doc->selectedPuncta()->end(); it++) {
    (*it)->setProperty2("");
    m_doc->punctaObjsModel()->updateData(*it);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty3()
{
  for (std::set<ZPunctum*>::iterator it=m_doc->selectedPuncta()->begin(); it != m_doc->selectedPuncta()->end(); it++) {
    (*it)->setProperty3("");
    m_doc->punctaObjsModel()->updateData(*it);
  }
}

void Z3DWindow::saveSelectedPunctaAs()
{
  QString filename =
    QFileDialog::getSaveFileName(this, tr("Save Selected Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo) "));

  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    ZPunctumIO::save(filename, m_doc->selectedPuncta()->begin(), m_doc->selectedPuncta()->end());
  }
}

void Z3DWindow::saveAllPunctaAs()
{
  QString filename =
    QFileDialog::getSaveFileName(this, tr("Save All Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo) "));

  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    ZPunctumIO::save(filename, m_doc->punctaList()->begin(), m_doc->punctaList()->end());
  }
}

void Z3DWindow::markPunctum()
{
  m_doc->markPunctum(m_lastClickedPosInVolume[0], m_lastClickedPosInVolume[1], m_lastClickedPosInVolume[2]);
}

void Z3DWindow::locatePunctumIn2DView()
{
  if (m_doc->selectedPuncta()->size() == 1) {
    if (m_doc->getParentFrame() != NULL) {
      ZPunctum* punc = *(m_doc->selectedPuncta()->begin());
      m_doc->getParentFrame()->viewRoi(punc->x(), punc->y(), iround(punc->z()), punc->radius() * 4);
    }
  }
}

void Z3DWindow::takeScreenShot(QString filename, int width, int height, Z3DScreenShotType sst)
{
  if (!m_canvasRenderer->renderToImage(filename, width, height, sst)) {
    QMessageBox::critical(this, "Error", m_canvasRenderer->getRenderToImageError());
  }
}

void Z3DWindow::takeScreenShot(QString filename, Z3DScreenShotType sst)
{
  int h = m_canvas->height();
  if (h % 2 == 1) {
    ++h;
  }
  int w = m_canvas->width();
  if (w % 2 == 1) {
    ++w;
  }
  if (m_canvas->width() % 2 == 1 || m_canvas->height() % 2 == 1) {
    LINFO() << "Resize canvas size from (" << m_canvas->width() << "," << m_canvas->height() << ") to (" << w << "," << h << ").";
    m_canvas->resize(w, h);
  }
  if (!m_canvasRenderer->renderToImage(filename, sst)) {
    QMessageBox::critical(this, "Error", m_canvasRenderer->getRenderToImageError());
  }
  //m_compositor->savePickingBufferToImage(filename + "_pickingBuffer.tif");
}

void Z3DWindow::openAdvancedSetting(const QString &name)
{
  if (!m_advancedSettingDockWidget) {
    m_advancedSettingDockWidget = new QDockWidget(tr("Advanced Settings"), this);
    m_advancedSettingDockWidget->setContentsMargins(20, 20, 20, 20);
    m_advancedSettingDockWidget->setMinimumSize(512, 512);
    QTabWidget *tabs = createAdvancedSettingTabWidget();
    m_advancedSettingDockWidget->setWidget(tabs);
    addDockWidget(Qt::RightDockWidgetArea, m_advancedSettingDockWidget);
    m_advancedSettingDockWidget->setFloating(true);
    m_viewMenu->addAction(m_advancedSettingDockWidget->toggleViewAction());
  }
  m_advancedSettingDockWidget->showNormal();
  QTabWidget *tabWidget = dynamic_cast<QTabWidget*>(m_advancedSettingDockWidget->widget());
  for (int i=0; i<tabWidget->count(); i++) {
    if (tabWidget->tabText(i) == name) {
      tabWidget->setCurrentIndex(i);
      break;
    }
  }
}

void Z3DWindow::takeSeriesScreenShot(const QDir &dir, const QString &namePrefix, glm::vec3 axis,
                                 bool clockWise, int numFrame, int width, int height, Z3DScreenShotType sst)
{
  QString title = "Capturing Images...";
  if (sst == HalfSideBySideStereoView)
    title = "Capturing Half Side-By-Side Stereo Images...";
  else if (sst == FullSideBySideStereoView)
    title = "Capturing Full Side-By-Side Stereo Images...";
  QProgressDialog progress(title, "Cancel", 0, numFrame, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();
  double rAngle = 360.0 / numFrame;
  for (int i=0; i<numFrame; i++) {
    progress.setValue(i);
    if (progress.wasCanceled())
      break;

    if (clockWise)
      getCamera()->rotate(rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    else
      getCamera()->rotate(-rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    resetCameraClippingRange();
    int fieldWidth = numDigits(numFrame);
    QString filename = QString("%1%2.tif").arg(namePrefix).arg(i, fieldWidth, 10, QChar('0'));
    QString filepath = dir.filePath(filename);
    takeScreenShot(filepath, width, height, sst);
  }
  progress.setValue(numFrame);
}

void Z3DWindow::takeSeriesScreenShot(const QDir &dir, const QString &namePrefix, glm::vec3 axis,
                                 bool clockWise, int numFrame, Z3DScreenShotType sst)
{
  QString title = "Capturing Images...";
  if (sst == HalfSideBySideStereoView)
    title = "Capturing Half Side-By-Side Stereo Images...";
  else if (sst == FullSideBySideStereoView)
    title = "Capturing Full Side-By-Side Stereo Images...";
  QProgressDialog progress(title, "Cancel", 0, numFrame, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();
  double rAngle = 360.0 / numFrame;
  for (int i=0; i<numFrame; i++) {
    progress.setValue(i);
    if (progress.wasCanceled())
      break;

    if (clockWise)
      getCamera()->rotate(rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    else
      getCamera()->rotate(-rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    resetCameraClippingRange();
    int fieldWidth = numDigits(numFrame);
    QString filename = QString("%1%2.tif").arg(namePrefix).arg(i, fieldWidth, 10, QChar('0'));
    QString filepath = dir.filePath(filename);
    takeScreenShot(filepath, sst);
  }
  progress.setValue(numFrame);
}

void Z3DWindow::updateSettingsDockWidget()
{
  //  QScrollArea *oldSA = dynamic_cast<QScrollArea*>(m_settingsDockWidget->widget());
  //  int oldScrollBarValue = 0;
  //  if (oldSA) {
  //    QScrollBar *bar = oldSA->verticalScrollBar();
  //    oldScrollBarValue = bar->value();
  //  }
  //  QScrollArea *newSA = dynamic_cast<QScrollArea*>(m_widgetsGroup->createWidget(this, true));
  //  newSA->verticalScrollBar()->setValue(oldScrollBarValue);
  QTabWidget *old = dynamic_cast<QTabWidget*>(m_settingsDockWidget->widget());
  int oldIndex = 0;
  if (old)
    oldIndex = old->currentIndex();
  QTabWidget *tabs = createBasicSettingTabWidget();
  if (oldIndex >= 0 && oldIndex < tabs->count())
    tabs->setCurrentIndex(oldIndex);
  m_settingsDockWidget->setUpdatesEnabled(false);
  m_settingsDockWidget->setWidget(tabs);
  m_settingsDockWidget->setUpdatesEnabled(true);
  if (old) {
    old->setParent(NULL);
    delete old;
  }

  // for advanced setting widget
  if (m_advancedSettingDockWidget) {
    QTabWidget *old = dynamic_cast<QTabWidget*>(m_advancedSettingDockWidget->widget());
    int oldIndex = 0;
    if (old)
      oldIndex = old->currentIndex();
    QTabWidget *tabs = createAdvancedSettingTabWidget();
    if (oldIndex >= 0 && oldIndex < tabs->count())
      tabs->setCurrentIndex(oldIndex);
    m_advancedSettingDockWidget->setUpdatesEnabled(false);
    m_advancedSettingDockWidget->setWidget(tabs);
    m_advancedSettingDockWidget->setUpdatesEnabled(true);
    if (old) {
      old->setParent(NULL);
      delete old;
    }
  }
}

void Z3DWindow::toogleAddSwcNodeMode(bool checked)
{
  if (checked) {
    if (m_toogleExtendSelectedSwcNodeAction->isChecked()) {
      m_toogleExtendSelectedSwcNodeAction->blockSignals(true);
      m_toogleExtendSelectedSwcNodeAction->setChecked(false);
      m_toogleExtendSelectedSwcNodeAction->blockSignals(false);
    }
    if (m_toogleSmartExtendSelectedSwcNodeAction->isChecked()) {
      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(true);
      m_toogleSmartExtendSelectedSwcNodeAction->setChecked(false);
      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(false);
    }
    m_swcFilter->setInteractionMode(Z3DSwcFilter::AddSwcNode);
    m_canvas->setCursor(Qt::PointingHandCursor);
  } else {
    m_swcFilter->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->setCursor(Qt::ArrowCursor);
  }
}

void Z3DWindow::toogleExtendSelectedSwcNodeMode(bool checked)
{
  if (checked) {
    if (m_toogleAddSwcNodeModeAction->isChecked()) {
      m_toogleAddSwcNodeModeAction->blockSignals(true);
      m_toogleAddSwcNodeModeAction->setChecked(false);
      m_toogleAddSwcNodeModeAction->blockSignals(false);
    }
    if (m_toogleSmartExtendSelectedSwcNodeAction->isChecked()) {
      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(true);
      m_toogleSmartExtendSelectedSwcNodeAction->setChecked(false);
      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(false);
    }
    m_swcFilter->setInteractionMode(Z3DSwcFilter::ExtendSwcNode);
    m_canvas->setCursor(Qt::PointingHandCursor);
  } else {
    m_swcFilter->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->setCursor(Qt::ArrowCursor);
  }
}

void Z3DWindow::toogleSmartExtendSelectedSwcNodeMode(bool checked)
{
  if (checked) {
    if (m_toogleAddSwcNodeModeAction->isChecked()) {
      m_toogleAddSwcNodeModeAction->blockSignals(true);
      m_toogleAddSwcNodeModeAction->setChecked(false);
      m_toogleAddSwcNodeModeAction->blockSignals(false);
    }
    if (m_toogleExtendSelectedSwcNodeAction->isChecked()) {
      m_toogleExtendSelectedSwcNodeAction->blockSignals(true);
      m_toogleExtendSelectedSwcNodeAction->setChecked(false);
      m_toogleExtendSelectedSwcNodeAction->blockSignals(false);
    }
    m_swcFilter->setInteractionMode(Z3DSwcFilter::SmartExtendSwcNode);
    m_canvas->setCursor(Qt::PointingHandCursor);
  } else {
    m_swcFilter->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->setCursor(Qt::ArrowCursor);
  }
}

void Z3DWindow::changeBackground()
{
  m_settingsDockWidget->show();
  int index = m_widgetsGroup->getChildGroups().indexOf(m_compositor->getBackgroundWidgetsGroup());
  QTabWidget *tab = dynamic_cast<QTabWidget*>(m_settingsDockWidget->widget());
  tab->setCurrentIndex(index);
}

void Z3DWindow::toogleMoveSelectedObjectsMode(bool checked)
{
  getInteractionHandler()->setMoveObjects(checked);
}

void Z3DWindow::moveSelectedObjects(double x, double y, double z)
{
  m_doc->executeMoveObjectCommand(x, y, z, m_punctaFilter->getCoordScales().x,
                                  m_punctaFilter->getCoordScales().y,
                                  m_punctaFilter->getCoordScales().z,
                                  m_swcFilter->getCoordScales().x,
                                  m_swcFilter->getCoordScales().y,
                                  m_swcFilter->getCoordScales().z);

#if 0
  if (m_doc->selectedSwcs()->empty() && m_doc->selectedPuncta()->empty() &&
      m_doc->selectedSwcTreeNodes()->empty())
    return;
  ZStackDocMoveSelectedObjectCommand *moveSelectedObjectCommand =
      new ZStackDocMoveSelectedObjectCommand(m_doc.get(),
                                             x, y, z);
  moveSelectedObjectCommand->setPunctaCoordScale(m_punctaFilter->getCoordScales().x,
                                                 m_punctaFilter->getCoordScales().y,
                                                 m_punctaFilter->getCoordScales().z);
  moveSelectedObjectCommand->setSwcCoordScale(m_swcFilter->getCoordScales().x,
                                              m_swcFilter->getCoordScales().y,
                                              m_swcFilter->getCoordScales().z);
  m_doc->undoStack()->push(moveSelectedObjectCommand);
#endif
}

void Z3DWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void Z3DWindow::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  m_doc->loadFileList(urls);
}

void Z3DWindow::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
  {
    deleteSelectedSwcNode();
    removeSelectedObject();
  }
    break;
  case Qt::Key_C:
  {
    if (event->modifiers() == Qt::ControlModifier) {
      std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
      if (nodeSet->size() > 0) {
        SwcTreeNode::clearClipboard();
      }
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        SwcTreeNode::addToClipboard(*iter);
      }
    }
  }
    break;
  case Qt::Key_P:
  {
    if (event->modifiers() == Qt::ControlModifier) {
      for (size_t i = 0; i < SwcTreeNode::clipboard().size(); ++i) {
        std::cout << SwcTreeNode::clipboard()[i] << std::endl;
      }
    }
  }
    break;
  case Qt::Key_S:
    if (event->modifiers() == Qt::ControlModifier) {
      m_doc->saveSwc(this);
    }
    break;
  case Qt::Key_B:
#ifdef _DEBUG_
    if (event->modifiers() == Qt::ControlModifier) {
      QList<ZSwcTree*> *treeList = m_doc->swcList();
      foreach (ZSwcTree *tree, *treeList) {
        ZString source = tree->source();
        std::cout << source.lastInteger() << std::endl;
      }
    }
#endif
    break;
  default:
    break;
  }
}

QTabWidget *Z3DWindow::createBasicSettingTabWidget()
{
  QTabWidget *tabs = new QTabWidget();
  tabs->setElideMode(Qt::ElideNone);
  tabs->setUsesScrollButtons(true);
  const QList<ZWidgetsGroup*>& groups = m_widgetsGroup->getChildGroups();
  for (int i=0; i<groups.size(); i++) {
    if (groups[i]->isGroup()) {
      QWidget *widget = groups[i]->createWidget(this, true);
      tabs->addTab(widget, groups[i]->getGroupName());
      //widget->setVisible(groups[i]->isVisible());
    }
  }
  return tabs;
}

QTabWidget *Z3DWindow::createAdvancedSettingTabWidget()
{
  QTabWidget *tabs = new QTabWidget();
  tabs->setElideMode(Qt::ElideNone);
  const QList<ZWidgetsGroup*>& groups = m_widgetsGroup->getChildGroups();
  for (int i=0; i<groups.size(); i++) {
    if (groups[i]->isGroup() && groups[i]->getGroupName() != "Capture" && groups[i]->getGroupName() != "Utils") {
      tabs->addTab(groups[i]->createWidget(this, false), groups[i]->getGroupName());
    }
  }
  return tabs;
}

void Z3DWindow::updateContextMenu(const QString &group)
{
  if (group == "empty") {
    m_contextMenuGroup["empty"]->clear();
    if (channelNumber() > 0 && m_volumeSource->volumeNeedDownsample() && m_volumeSource->isSubvolume()) {
      m_contextMenuGroup["empty"]->addAction(m_exitVolumeZoomInViewAction);
    }
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toogleExtendSelectedSwcNodeAction);
    if (channelNumber() > 0 && !m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toogleAddSwcNodeModeAction);
    if (!m_doc->swcList()->empty() || !m_doc->punctaList()->empty())
      m_contextMenuGroup["empty"]->addAction(m_toogleMoveSelectedObjectsAction);
    m_contextMenuGroup["empty"]->addAction(m_changeBackgroundAction);
  }
  if (group == "volume") {
    m_contextMenuGroup["volume"]->clear();
    if (m_volumeSource->volumeNeedDownsample()) {
      if (m_volumeSource->isSubvolume()) {
        m_contextMenuGroup["volume"]->addAction(m_exitVolumeZoomInViewAction);
      } else {
        m_contextMenuGroup["volume"]->addAction(m_openVolumeZoomInViewAction);
      }
    }
    m_contextMenuGroup["volume"]->addAction(m_markPunctumAction);
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toogleExtendSelectedSwcNodeAction);
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toogleAddSwcNodeModeAction);
    if (!m_doc->swcList()->empty() || !m_doc->punctaList()->empty())
      m_contextMenuGroup["volume"]->addAction(m_toogleMoveSelectedObjectsAction);
    m_contextMenuGroup["volume"]->addAction(m_changeBackgroundAction);
    m_contextMenuGroup["volume"]->addAction(m_refreshTraceMaskAction);
  }
  if (group == "swcnode") {

  }
  if (group == "puncta") {
    m_saveSelectedPunctaAsAction->setEnabled(!m_doc->selectedPuncta()->empty());
    m_saveAllPunctaAsAction->setEnabled(!m_doc->punctaList()->isEmpty());
    m_locatePunctumIn2DAction->setEnabled(m_doc->selectedPuncta()->size() == 1);
  }
}

void Z3DWindow::updateOverallBoundBox(std::vector<double> bound)
{
  m_boundBox[0] = std::min(bound[0], m_boundBox[0]);
  m_boundBox[1] = std::max(bound[1], m_boundBox[1]);
  m_boundBox[2] = std::min(bound[2], m_boundBox[2]);
  m_boundBox[3] = std::max(bound[3], m_boundBox[3]);
  m_boundBox[4] = std::min(bound[4], m_boundBox[4]);
  m_boundBox[5] = std::max(bound[5], m_boundBox[5]);
}

void Z3DWindow::updateOverallBoundBox()
{
  m_boundBox[0] = m_boundBox[2] = m_boundBox[4] = std::numeric_limits<double>::max();
  m_boundBox[1] = m_boundBox[3] = m_boundBox[5] = -std::numeric_limits<double>::max();
  if (hasVolume()) {
    updateOverallBoundBox(m_volumeBoundBox);
  }
  updateOverallBoundBox(m_swcBoundBox);
  updateOverallBoundBox(m_punctaBoundBox);
  updateOverallBoundBox(m_graphBoundBox);
  if (m_boundBox[0] > m_boundBox[1] || m_boundBox[2] > m_boundBox[3] || m_boundBox[4] > m_boundBox[5]) {
    // nothing visible
    m_boundBox[0] = m_boundBox [2] = m_boundBox[4] = 0.0f;
    m_boundBox[1] = m_boundBox [3] = m_boundBox[5] = 1.0f;
  }
}

void Z3DWindow::changeSelectedSwcNodeType()
{
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (nodeSet->size() > 0) {
    SwcTypeDialog dlg(ZSwcTree::SWC_NODE, NULL);
    if (dlg.exec()) {
      switch (dlg.pickingMode()) {
      case SwcTypeDialog::INDIVIDUAL:
        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
             iter != nodeSet->end(); ++iter) {
          SwcTreeNode::setType(*iter, dlg.type());
        }
        break;
      case SwcTypeDialog::DOWNSTREAM:
        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
             iter != nodeSet->end(); ++iter) {
          SwcTreeNode::setDownstreamType(*iter, dlg.type());
        }
        break;
      case SwcTypeDialog::CONNECTION:
      {
        Swc_Tree_Node *ancestor = *(nodeSet->begin());

        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
             iter != nodeSet->end(); ++iter) {
          ancestor = SwcTreeNode::commonAncestor(ancestor, *iter);
        }

        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
             iter != nodeSet->end(); ++iter) {
          SwcTreeNode::setUpstreamType(*iter, dlg.type(), ancestor);
        }
      }
        break;
      case SwcTypeDialog::LONGEST_LEAF:
      {
        Swc_Tree_Node *tn = SwcTreeNode::furthestNode(*(nodeSet->begin()),
                                                      SwcTreeNode::GEODESIC);
        SwcTreeNode::setPathType(*(nodeSet->begin()), tn, dlg.type());
      }
        break;
      default:
        break;
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::setRootAsSelectedSwcNode()
{
  m_doc->executeSetRootCommand();
      /*
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    m_doc->executeSetRootCommand();

    SwcTreeNode::setAsRoot(*nodeSet->begin());
    m_doc->notifySwcModified();

  }
      */
}

void Z3DWindow::breakSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (nodeSet->size() >= 2) {
    m_doc->executeBreakSwcConnectionCommand();
#if 0
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      if (nodeSet->count((*iter)->parent) > 0) {
        SwcTreeNode::detachParent(*iter);
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(*iter);
        /*
        std::ostringstream stream;
        stream << "node-break" << "-" << tree;
        tree->setSource(stream.str());
        */
        m_doc->addSwcTree(tree, false);
      }
    }

    m_doc->notifySwcModified();
#endif
    /*
    std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    Swc_Tree_Node *child = NULL;
    if (tn1->parent == tn2) {
      child = tn1;
    } else if (tn2->parent == tn1) {
      child = tn2;
    }
    if (child != NULL) {
      Swc_Tree_Node_Detach_Parent(child);
      ZSwcTree *tree = new ZSwcTree();
      tree->setDataFromNode(child);
      m_doc->addSwcTree(tree);
      m_doc->requestRedrawSwc();
    }
    */
  }
}

void Z3DWindow::mergeSelectedSwcNode()
{
  m_doc->executeMergeSwcNodeCommand();
#if 0
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();

  if (nodeSet->size() > 1) {
    Swc_Tree_Node *coreNode = SwcTreeNode::merge(*nodeSet);
    if (coreNode != NULL) {
      if (SwcTreeNode::parent(coreNode) == NULL) {
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(coreNode);
        //m_doc->addSwcTree(tree, false);
        m_doc->executeAddSwcCommand(tree);
      }

      m_doc->executeDeleteSwcNodeCommand();
      //SwcTreeNode::kill(*nodeSet);

      //m_doc->removeEmptySwcTree();
      m_doc->notifySwcModified();
    }
  }
#endif
}

void Z3DWindow::deleteSelectedSwcNode()
{
  m_doc->executeDeleteSwcNodeCommand();
}

void Z3DWindow::locateSwcNodeIn2DView()
{
  if (!m_doc->selectedSwcTreeNodes()->empty()) {
    if (m_doc->getParentFrame() != NULL) {
      ZCuboid cuboid = SwcTreeNode::boundBox(*m_doc->selectedSwcTreeNodes());
      int cx, cy, cz;
      ZPoint center = cuboid.center();
      cx = iround(center.x());
      cy = iround(center.y());
      cz = iround(center.z());
      int radius = iround(std::max(cuboid.width(), cuboid.height()) / 2.0);
      m_doc->getParentFrame()->viewRoi(cx, cy, cz, radius);
    }
  }
}

void Z3DWindow::tranlateSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::pos(node + 1) - SwcTreeNode::pos(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    }
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        if (dlg.isTranslateFirst()) {
          SwcTreeNode::translate(*iter, dx, dy, dz);
        }

        SwcTreeNode::setPos(
              *iter,
              SwcTreeNode::x(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              SwcTreeNode::y(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              SwcTreeNode::z(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));

        if (!dlg.isTranslateFirst()) {
          SwcTreeNode::translate(*iter, dx, dy, dz);
        }
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::connectSelectedSwcNode()
{
  m_doc->executeConnectSwcNodeCommand();
  /*
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (SwcTreeNode::connect(*nodeSet)) {
    m_doc->removeEmptySwcTree();
    m_doc->notifySwcModified();
  }
  */
}

void Z3DWindow::changeSelectedSwcType()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (!treeSet->empty()) {
    SwcTypeDialog dlg(ZSwcTree::WHOLE_TREE, NULL);

    if (dlg.exec()) {
      switch (dlg.pickingMode()) {
      case SwcTypeDialog::INDIVIDUAL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          (*iter)->setType(dlg.type());
        }
        break;
      case SwcTypeDialog::MAIN_TRUNK:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(15.0, 1.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::LONGEST_LEAF:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(0.0, 1.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::FURTHEST_LEAF:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(1.0, 0.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::TRAFFIC:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          ZSwcDistTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.labelTraffic(*iter, ZSwcTrunkAnalyzer::REACH_ROOT);
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::TRUNK_LEVEL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          //ZSwcDistTrunkAnalyzer trunkAnalyzer;
          ZSwcWeightTrunkAnalyzer trunkAnalyzer;
          (*iter)->setBranchSizeWeight();
          (*iter)->labelTrunkLevel(&trunkAnalyzer);
          //trunkAnalyzer.labelTrunk(*iter);
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::BRANCH_LEVEL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          //(*iter)->labelBranchLevel(0);
          (*iter)->labelBranchLevelFromLeaf();
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::ROOT:
        for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
             iter != treeSet->end(); ++iter) {
          Swc_Tree_Node *tn = (*iter)->firstRegularRoot();
          while (tn != NULL) {
            SwcTreeNode::setType(tn, dlg.type());
            tn = SwcTreeNode::nextSibling(tn);
          }
        }
        break;
      default:
        break;
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcSize()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (!treeSet->empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
           iter != treeSet->end(); ++iter) {
        (*iter)->changeRadius(dlg.getAddValue(), dlg.getMulValue());
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcNodeSize()
{
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        SwcTreeNode::changeRadius(*iter, dlg.getAddValue(), dlg.getMulValue());
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::transformSelectedPuncta()
{
  std::set<ZPunctum*> *punctaSet = m_doc->selectedPuncta();
  if (!punctaSet->empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    dlg.setWindowTitle("Transform Puncta");
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      if (dlg.isTranslateFirst()) {
        for (std::set<ZPunctum*>::iterator iter = punctaSet->begin();
             iter != punctaSet->end(); ++iter) {
          (*iter)->setX((*iter)->x() + dx);
          (*iter)->setY((*iter)->y() + dy);
          (*iter)->setZ((*iter)->z() + dz);
        }
      }

      for (std::set<ZPunctum*>::iterator iter = punctaSet->begin();
           iter != punctaSet->end(); ++iter) {
        (*iter)->setX((*iter)->x() * dlg.getScaleValue(SwcSkeletonTransformDialog::X));
        (*iter)->setY((*iter)->y() * dlg.getScaleValue(SwcSkeletonTransformDialog::Y));
        (*iter)->setZ((*iter)->z() * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));
      }

      if (!dlg.isTranslateFirst()) {
        for (std::set<ZPunctum*>::iterator iter = punctaSet->begin();
             iter != punctaSet->end(); ++iter) {
          (*iter)->setX((*iter)->x() + dx);
          (*iter)->setY((*iter)->y() + dy);
          (*iter)->setZ((*iter)->z() + dz);
        }
      }
    }
    m_doc->notifyPunctumModified();
  }
}

void Z3DWindow::transformAllPuncta()
{
  QList<ZPunctum*> *punctaSet = m_doc->punctaList();
  if (!punctaSet->empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    dlg.setWindowTitle("Transform Puncta");
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      if (dlg.isTranslateFirst()) {
        for (QList<ZPunctum*>::iterator iter = punctaSet->begin();
             iter != punctaSet->end(); ++iter) {
          (*iter)->setX((*iter)->x() + dx);
          (*iter)->setY((*iter)->y() + dy);
          (*iter)->setZ((*iter)->z() + dz);
        }
      }

      for (QList<ZPunctum*>::iterator iter = punctaSet->begin();
           iter != punctaSet->end(); ++iter) {
        (*iter)->setX((*iter)->x() * dlg.getScaleValue(SwcSkeletonTransformDialog::X));
        (*iter)->setY((*iter)->y() * dlg.getScaleValue(SwcSkeletonTransformDialog::Y));
        (*iter)->setZ((*iter)->z() * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));
      }

      if (!dlg.isTranslateFirst()) {
        for (QList<ZPunctum*>::iterator iter = punctaSet->begin();
             iter != punctaSet->end(); ++iter) {
          (*iter)->setX((*iter)->x() + dx);
          (*iter)->setY((*iter)->y() + dy);
          (*iter)->setZ((*iter)->z() + dz);
        }
      }
      m_doc->notifyPunctumModified();
    }
  }
}

void Z3DWindow::convertPunctaToSwc()
{
  QList<ZPunctum*> *punctaSet = m_doc->punctaList();
  if (!punctaSet->empty()) {
    ZSwcTree *tree = new ZSwcTree();
    for (QList<ZPunctum*>::iterator iter = punctaSet->begin();
         iter != punctaSet->end(); ++iter) {
      ZPoint pos((*iter)->x(), (*iter)->y(), (*iter)->z());
      Swc_Tree_Node *tn = SwcTreeNode::makePointer(pos, (*iter)->radius());
      tree->addRegularRoot(tn);
    }

    m_doc->addSwcTree(tree, false);
    m_doc->removeSelectedPuncta();

    m_doc->notifyPunctumModified();
    m_doc->notifySwcModified();
  }
}

void Z3DWindow::transformSelectedSwc()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (!treeSet->empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::pos(node + 1) - SwcTreeNode::pos(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    }
    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
           iter != treeSet->end(); ++iter) {
        if (dlg.isTranslateFirst()) {
          (*iter)->translate(dlg.getTranslateValue(SwcSkeletonTransformDialog::X),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Y),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Z));
        }

        (*iter)->scale(dlg.getScaleValue(SwcSkeletonTransformDialog::X),
                       dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
                       dlg.getScaleValue(SwcSkeletonTransformDialog::Z));

        if (!dlg.isTranslateFirst()) {
          (*iter)->translate(dlg.getTranslateValue(SwcSkeletonTransformDialog::X),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Y),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Z));
        }
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::groupSelectedSwc()
{
  m_doc->executeGroupSwcCommand();
  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (treeSet->size() > 1) {
    std::set<ZSwcTree*>::iterator iter = treeSet->begin();
    Swc_Tree_Node *root = (*iter)->root();

    for (++iter; iter != treeSet->end(); ++iter) {
      Swc_Tree_Node *subroot = (*iter)->firstRegularRoot();
      SwcTreeNode::setParent(subroot, root);
    }
    m_doc->removeEmptySwcTree();
    m_doc->notifySwcModified();
  }
  */
}

void Z3DWindow::showSeletedSwcNodeLength()
{
  double length = SwcTreeNode::segmentLength(*(m_doc->selectedSwcTreeNodes()));

  InformationDialog dlg;

  std::ostringstream textStream;

  textStream << "<p>Overall length of selected branches: " << length << "</p>";

  if (m_doc->selectedSwcTreeNodes()->size() == 2) {
    std::set<Swc_Tree_Node*>::const_iterator iter =
        m_doc->selectedSwcTreeNodes()->begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    textStream << "<p>Straight line distance between the two selected nodes: "
               << SwcTreeNode::distance(tn1, tn2) << "</p>";
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void Z3DWindow::showSelectedSwcInfo()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  InformationDialog dlg;

  std::ostringstream textStream;

  if (!treeSet->empty()) {
    for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
         iter != treeSet->end(); ++iter) {
      textStream << "<p><font color=\"blue\">" + (*iter)->source() + "</font></p>";
      textStream << "<p>Overall length: " << (*iter)->length() << "</p>";
      std::set<int> typeList = (*iter)->typeSet();
      if (typeList.size() > 1) {
        textStream << "<p>Typed branch length: ";
        textStream << "<ul>";
        for (std::set<int>::const_iterator typeIter = typeList.begin();
             typeIter != typeList.end(); ++typeIter) {
          textStream << "<li>Type " << *typeIter << ": " << (*iter)->length(*typeIter)
                    << "</li>";
        }
        textStream << "</ul>";
        textStream << "</p>";
      }
      textStream << "<p>Lateral-vertical ratio: "
                 << ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*(*iter))
                 << "</p>";
    }
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void Z3DWindow::refreshTraceMask()
{
  getDocument()->refreshTraceMask();
}

void Z3DWindow::changeSelectedSwcColor()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  if (!treeSet->empty()) {
    QColorDialog dlg;
    dlg.setCurrentColor((*treeSet->begin())->getColor());

    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
           iter != treeSet->end(); ++iter) {
        (*iter)->setColor(dlg.currentColor().red(),
                          dlg.currentColor().green(),
                          dlg.currentColor().blue());
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcAlpha()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  if (!treeSet->empty()) {
    ZAlphaDialog dlg;
    if (dlg.exec()) {
      int alpha = dlg.getAlpha();
      for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
           iter != treeSet->end(); ++iter) {
        (*iter)->setAlpha(alpha);
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::test()
{
  const NeutubeConfig &config = NeutubeConfig::getInstance();

  UNUSED_PARAMETER(&config);

  ZMovieMaker director;
  ZMovieScript script;


  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  treeSet->clear();
  m_doc->requestRedrawSwc();

  QList<ZSwcTree*> *treeList =  m_doc->swcList();
  std::vector<ZMovieActor*> cast(treeList->size());
  std::vector<ZSwcMovieActor> swcCast(treeList->size());

  int index = 0;
  for (QList<ZSwcTree*>::iterator iter = treeList->begin();
       iter != treeList->end(); ++iter, ++index) {
    swcCast[index].setActor(*iter);
    swcCast[index].setMovingOffset((index + 1) * 10, 0, 0);
    cast[index] = &(swcCast[index]);
  }
*/
/*
  ZMovieScene scene(m_doc.get(), this);

  for (index = 0; index < 5; ++index) {
    std::ostringstream stream;
    stream << config.getPath(NeutubeConfig::DATA) + "/test/";
    stream << std::setw(3) << std::setfill('0') << index << ".tif";

    for (std::vector<ZMovieActor*>::iterator iter = cast.begin();
         iter != cast.end(); ++iter) {
      (*iter)->perform();
    }
    scene.saveToImage(stream.str(), 1024, 1024);
  }
*/
  /*
  for (QList<ZSwcTree*>::iterator iter = treeList->begin();
       iter != treeList->end(); ++iter, ++index) {

    scene.saveToImage(stream.str(), 1024, 1024);
    //takeScreenShot(stream.str().c_str(),
    //               1024, 1024, MonoView);
    (*iter)->setVisible(false);
    getInteractionHandler()->getTrackball()->rotate(glm::vec3(0,1,0), 0.5);
    getInteractionHandler()->getTrackball()->zoom(1.1);
  }
*/

  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
       iter != treeSet->end(); ++iter) {
    (*iter)->setVisible(false);
  }
  treeSet->clear();
  */


}

void Z3DWindow::breakSelectedSwc()
{
  m_doc->executeBreakForestCommand();
  //Need to change to m_doc->executeBreakSwcCommand();
#if 0
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (!treeSet->empty()) {
    for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
         iter != treeSet->end(); ++iter) {
      Swc_Tree_Node *root = (*iter)->firstRegularRoot();
      if (root != NULL) {
        root = SwcTreeNode::nextSibling(root);
        while (root != NULL) {
          Swc_Tree_Node *sibling = SwcTreeNode::nextSibling(root);
          SwcTreeNode::detachParent(root);
          ZSwcTree *tree = new ZSwcTree;
          tree->setDataFromNode(root);
          /*
          std::ostringstream stream;
          stream << (*iter)->source().c_str() << "-" << m_doc->swcList()->size() + 1;
          tree->setSource(stream.str());
          */
          m_doc->addSwcTree(tree, false);
          root = sibling;
        }
      }
    }
    m_doc->notifySwcModified();
  }
#endif

}

void Z3DWindow::saveSelectedSwc()
{
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  QString fileName = "";

  if (!treeSet->empty()) {
    if (!(*treeSet->begin())->source().empty()) {
      if ((*treeSet->begin())->source()[0] != '#') {
        fileName = QString((*treeSet->begin())->source().c_str());
      }
    }
  }

  if (fileName.isEmpty()) {
    ZString stackSource = m_doc->stackSourcePath().toStdString();
    if (!stackSource.empty()) {
      fileName = stackSource.changeExt("Edit.swc").c_str();
    }
  }

  if (fileName.isEmpty()) {
    fileName = "untitled.swc";
  }

  fileName =
      QFileDialog::getSaveFileName(this, tr("Save SWC"), fileName,
                                   tr("SWC File"), 0);

  if (!fileName.isEmpty()) {
    if (!fileName.endsWith(".swc", Qt::CaseInsensitive)) {
      fileName += ".swc";
    }

    if (treeSet->size() > 1) {
      ZSwcTree tree;

      for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
           iter != treeSet->end(); ++iter) {
        tree.merge((*iter)->cloneData(), true);
      }

      tree.resortId();
      tree.save(fileName.toStdString().c_str());
    } else {
      ZSwcTree *tree = *(treeSet->begin());
      tree->resortId();
      tree->save(fileName.toStdString().c_str());
      tree->setSource(fileName.toStdString().c_str());
      getDocument()->notifySwcModified();
    }
  }
}

void Z3DWindow::convertSelectedChainToSwc()
{
  std::set<ZLocsegChain*> *chainSet = m_doc->selectedChains();
  for (std::set<ZLocsegChain*>::iterator iter = chainSet->begin();
       iter != chainSet->end(); ++iter) {
    Swc_Tree_Node *tn = TubeModel::createSwc((*iter)->data());
    if (tn != NULL) {
      ZSwcTree *tree = new ZSwcTree;
      tree->setDataFromNode(tn);
      m_doc->addSwcTree(tree, false);
    }
  }
  chainSet->clear();

  m_doc->executeRemoveTubeCommand();

  m_doc->notifySwcModified();
  m_doc->notifyChainModified();
}

bool Z3DWindow::hasSwc() const
{
  return m_doc->hasSwc();
}

bool Z3DWindow::hasSelectedSwc() const
{
  return !m_doc->selectedSwcs()->empty();
}

bool Z3DWindow::hasSelectedSwcNode() const
{
  return !m_doc->selectedSwcTreeNodes()->empty();
}

bool Z3DWindow::hasMultipleSelectedSwcNode() const
{
  return m_doc->hasMultipleSelectedSwcNode();
}
