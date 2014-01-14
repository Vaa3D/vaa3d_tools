#include "z3dgl.h"
#include <iostream>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include <QTextStream>
#include <QtDebug>
#include <iterator>
#include <QSet>
#include <vector>
#include <QTimer>
#include <QInputDialog>
#include <QApplication>

#include "QsLog.h"

#include "informationdialog.h"
#include "tz_image_io.h"
#include "tz_math.h"
#include "zstackdoc.h"
#include "tz_stack_lib.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "zlocalneuroseg.h"
#include "zlocsegchain.h"
#include "tz_vrml_io.h"
#include "tz_vrml_material.h"
#include "tz_color.h"
#include "zstackframe.h"
#include "zellipse.h"
#include "tz_workspace.h"
#include "tz_string.h"
#include "zlocsegchainconn.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_objdetect.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_sampling.h"
#include "tz_stack_utils.h"
#include "tz_darray.h"
#include "tz_stack_lib.h"
#include "tz_stack_math.h"
#include "tz_local_rpi_neuroseg.h"
#include "zlocalrect.h"
#include "tz_geo3d_ball.h"
#include "tz_workspace.h"
#include "zdirectionaltemplatechain.h"
#include "tz_r2_ellipse.h"
#include "zstack.hxx"
#include "tz_stack_stat.h"
#include "mainwindow.h"
#include "tz_error.h"
#include "zswcnetwork.h"
#include "zstring.h"
#include "zcolormap.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zneuronnetwork.h"
#include "zfiletype.h"
#include "zstackfile.h"
#include "zstackprocessor.h"
#include "zswcobjsmodel.h"
#include "zswcnodeobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "swctreenode.h"
#include "zstackgraph.h"
#include "zgraphcompressor.h"
#include "zgraph.h"
#include "neutubeconfig.h"
#include "tz_stack_bwmorph.h"
#include "zstackdoccommand.h"
#include "zstroke2d.h"
#include "zneurontracer.h"
#include "zqtmessagereporter.h"
#include "zswcconnector.h"
#include "biocytin/biocytin.h"
#include "zpunctumio.h"
#include "biocytin/zbiocytinfilenameparser.h"
#include "swcskeletontransformdialog.h"
#include "swcsizedialog.h"

using namespace std;

ZStackDoc::ZStackDoc(ZStack *stack, QObject *parent) : QObject(parent)
{
  m_stack = stack;
  m_parentFrame = NULL;
  //m_parent = parent;
  m_masterChain = NULL;
  //m_previewSwc = NULL;
  m_traceWorkspace = NULL;
  m_isTraceMaskObsolete = true;
  m_connectionTestWorkspace = NULL;
  m_swcNetwork = NULL;
  initTraceWorkspace();
  initConnectionTestWorkspace();
  m_swcObjsModel = new ZSwcObjsModel(this, this);
  m_swcNodeObjsModel = new ZSwcNodeObjsModel(this, this);
  m_punctaObjsModel = new ZPunctaObjsModel(this, this);
  m_undoStack = new QUndoStack(this);
  //m_progressReporter = &m_nullProgressReporter;
  m_undoAction = m_undoStack->createUndoAction(this, tr("&Undo"));
  m_undoAction->setIcon(QIcon(":/images/undo.png"));
  m_undoAction->setShortcuts(QKeySequence::Undo);

  m_redoAction = m_undoStack->createRedoAction(this, tr("&Redo"));
  m_redoAction->setIcon(QIcon(":/images/redo.png"));
  m_redoAction->setShortcuts(QKeySequence::Redo);

  //m_swcNodeContextMenu = NULL;

  connectSignalSlot();

  setReporter(new ZQtMessageReporter());

  if (NeutubeConfig::getInstance().isAutoSaveEnabled()) {
    QTimer *timer = new QTimer(this);
    timer->start(NeutubeConfig::getInstance().getAutoSaveInterval());
    connect(timer, SIGNAL(timeout()), this, SLOT(autoSave()));
  }

  createActions();
  //createContextMenu();

  setTag(NeuTube::Document::NORMAL);
  setStackBackground(NeuTube::IMAGE_BACKGROUND_DARK);
}

ZStackDoc::~ZStackDoc()
{
  deprecate(STACK);

  if (m_objs.size() != 0) {
    for (int i = 0; i < m_objs.size(); ++i) {
      delete m_objs.at(i);
    }
    m_objs.clear();
  }

  if (m_swcNetwork != NULL) {
    delete m_swcNetwork;
  }

  if (m_traceWorkspace != NULL) {
    if (m_traceWorkspace->fit_workspace != NULL) {
      Locseg_Fit_Workspace *fw =
          (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
      fw->sws->mask = NULL;
      Kill_Locseg_Fit_Workspace(fw);
      m_traceWorkspace->fit_workspace = NULL;
    }
    Kill_Trace_Workspace(m_traceWorkspace);
  }

  if (m_connectionTestWorkspace != NULL) {
    Kill_Connection_Test_Workspace(m_connectionTestWorkspace);
  }

  /*
  if (m_previewSwc != NULL) {
    delete m_previewSwc;
  }
  */

  delete m_undoStack;
  //delete m_swcNodeContextMenu;

  destroyReporter();
  //delete m_stackMask;
}

void ZStackDoc::setParentFrame(ZStackFrame *parent)
{
  m_parentFrame = parent;
}

ZStack* ZStackDoc::stack() const
{
  return m_stack;
}

ZStack* ZStackDoc::stackMask() const
{
  return NULL;
}

void ZStackDoc::emptySlot()
{
  QMessageBox::information(NULL, "empty slot", "To be implemented");
}

void ZStackDoc::connectSignalSlot()
{
  connect(this, SIGNAL(swcModified()), m_swcObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(swcModified()), m_swcNodeObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(punctaModified()), m_punctaObjsModel, SLOT(updateModelData()));
  connect(m_undoStack, SIGNAL(cleanChanged(bool)),
          this, SIGNAL(cleanChanged(bool)));
  connect(&m_reader, SIGNAL(finished()), this, SIGNAL(stackReadDone()));
  connect(this, SIGNAL(stackReadDone()), this, SLOT(loadReaderResult()));
}

void ZStackDoc::createActions()
{
  QAction *action = new QAction("Downstream", this);
  connect(action, SIGNAL(triggered()), this, SLOT(selectDownstreamNode()));
  m_actionMap[ACTION_SELECT_DOWNSTREAM] = action;

  action = new QAction("Upstream", this);
  connect(action, SIGNAL(triggered()), this, SLOT(selectUpstreamNode()));
  m_actionMap[ACTION_SELECT_UPSTREAM] = action;

  action = new QAction("Neighbors", this);
  connect(action, SIGNAL(triggered()), this, SLOT(selectNeighborSwcNode()));
  m_actionMap[ACTION_SELECT_NEIGHBOR_SWC_NODE] = action;

  action = new QAction("Host branch", this);
  connect(action, SIGNAL(triggered()), this, SLOT(selectBranchNode()));
  m_actionMap[ACTION_SELECT_SWC_BRANCH] = action;

  action = new QAction("All connected nodes", this);
  connect(action, SIGNAL(triggered()), this, SLOT(selectConnectedNode()));
  m_actionMap[ACTION_SELECT_CONNECTED_SWC_NODE] = action;

  action = new QAction("All nodes", this);
  connect(action, SIGNAL(triggered()), this, SLOT(selectAllSwcTreeNode()));
  m_actionMap[ACTION_SELECT_ALL_SWC_NODE] = action;

  action = new QAction("Resolve crossover", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeResolveCrossoverCommand()));
  m_actionMap[ACTION_RESOLVE_CROSSOVER] = action;

  action = new QAction("Remove turn", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeRemoveTurnCommand()));
  m_actionMap[ACTION_REMOVE_TURN] = action;

  action = new QAction("Path length", this);
  connect(action, SIGNAL(triggered()), this, SLOT(showSeletedSwcNodeLength()));
  m_actionMap[ACTION_MEASURE_SWC_NODE_LENGTH] = action;

  action = new QAction("Delete", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeDeleteSwcNodeCommand()));
  m_actionMap[ACTION_DELETE_SWC_NODE] = action;

  action = new QAction("Insert", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeInsertSwcNode()));
  m_actionMap[ACTION_INSERT_SWC_NODE] = action;

  action = new QAction("Break", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeBreakSwcConnectionCommand()));
  m_actionMap[ACTION_BREAK_SWC_NODE] = action;

  action = new QAction("Connect", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeConnectSwcNodeCommand()));
  m_actionMap[ACTION_CONNECT_SWC_NODE] = action;

  action = new QAction("Merge", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeMergeSwcNodeCommand()));
  m_actionMap[ACTION_MERGE_SWC_NODE] = action;

  action = new QAction("Translate", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeTranslateSelectedSwcNode()));
  m_actionMap[ACTION_TRANSLATE_SWC_NODE] = action;

  action = new QAction("Change size", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeChangeSelectedSwcNodeSize()));
  m_actionMap[ACTION_CHANGE_SWC_SIZE] = action;

  action = new QAction("Set as root", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeSetRootCommand()));
  m_actionMap[ACTION_SET_SWC_ROOT] = action;

  action = new QAction("Join isolated branch", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeSetBranchPoint()));
  m_actionMap[ACTION_SET_BRANCH_POINT] = action;

  action = new QAction("Join isolated brach (across trees)", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeConnectIsolatedSwc()));
  m_actionMap[ACTION_CONNECTED_ISOLATED_SWC] = action;

  action = new QAction("Reset branch point", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeResetBranchPoint()));
  m_actionMap[ACTION_RESET_BRANCH_POINT] = action;

  action = new QAction("Z Interpolation", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeInterpolateSwcZCommand()));
  m_actionMap[ACTION_SWC_Z_INTERPOLATION] = action;

  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_MEASURE_SWC_NODE_LENGTH], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_BREAK_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_CONNECT_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_MERGE_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_SET_SWC_ROOT], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_INSERT_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_SET_BRANCH_POINT], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_RESET_BRANCH_POINT], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_CONNECTED_ISOLATED_SWC], true);
}

void ZStackDoc::updateSwcNodeAction()
{
  m_singleSwcNodeActionActivator.update(this);
}
/*
void ZStackDoc::createContextMenu()
{
  m_swcNodeContextMenu = new QMenu(NULL);

  m_swcNodeContextMenu->addAction(getAction(ACTION_DELETE_SWC_NODE));
  m_swcNodeContextMenu->addAction(getAction(ACTION_BREAK_SWC_NODE));
  m_swcNodeContextMenu->addAction(getAction(ACTION_CONNECT_SWC_NODE));
  m_swcNodeContextMenu->addAction(getAction(ACTION_MERGE_SWC_NODE));

  QMenu *submenu = new QMenu("Select", m_swcNodeContextMenu);
  submenu->addAction(getAction(ACTION_SELECT_DOWNSTREAM));
  submenu->addAction(getAction(ACTION_SELECT_UPSTREAM));
  submenu->addAction(getAction(ACTION_SELECT_SWC_BRANCH));
  submenu->addAction(getAction(ACTION_SELECT_CONNECTED_SWC_NODE));
  submenu->addAction(getAction(ACTION_SELECT_ALL_SWC_NODE));
  m_swcNodeContextMenu->addMenu(submenu);

  submenu = new QMenu("Advanced Editing", m_swcNodeContextMenu);
  submenu->addAction(getAction(ACTION_REMOVE_TURN));
  submenu->addAction(getAction(ACTION_RESOLVE_CROSSOVER));
  m_swcNodeContextMenu->addMenu(submenu);

  submenu = new QMenu("Information", m_swcNodeContextMenu);
  submenu->addAction(getAction(ACTION_SWC_SUMMARIZE));
  submenu->addAction(getAction(ACTION_MEASURE_SWC_NODE_LENGTH));
  m_swcNodeContextMenu->addMenu(submenu);
}
*/

void ZStackDoc::autoSave()
{
  qDebug() << "Auto save triggered in " << this;
  if (!swcList()->empty()) {
    std::string autoSaveDir = NeutubeConfig::getInstance().getPath(
          NeutubeConfig::AUTO_SAVE);
    QDir dir(autoSaveDir.c_str());
    if (dir.exists()) {
      ostringstream stream;
      stream << this;
      std::string autoSavePath =
          autoSaveDir + ZString::FileSeparator;
      if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
        autoSavePath +=
            ZBiocytinFileNameParser::getCoreName(stack()->sourcePath()) +
            ".autosave.swc";
      } else {
        autoSavePath += "~" + stream.str() + ".swc";
      }

      FILE *fp = fopen(autoSavePath.c_str(), "w");
      if (fp != NULL) {
        fclose(fp);
        ZSwcTree *tree = new ZSwcTree;
        foreach (ZSwcTree *treeItem, m_swcList) {
          tree->merge(Copy_Swc_Tree(treeItem->data()), true);
        }
        tree->resortId();
        tree->save(autoSavePath.c_str());
        qDebug() << autoSavePath.c_str();

        delete tree;
      }
    }
  }
}

string ZStackDoc::getSwcSource() const
{
  string swcSource;
  if (m_swcList.size() == 1) {
    swcSource = m_swcList.first()->source();
  }

  return swcSource;
}

bool ZStackDoc::saveSwc(const string &filePath)
{
  if (!swcList()->empty()) {
    ZSwcTree *tree = NULL;
    if (swcList()->size() > 1) {
      tree = new ZSwcTree;
      foreach (ZSwcTree *treeItem, m_swcList) {
        tree->merge(Copy_Swc_Tree(treeItem->data()), true);
      }
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      foreach (ZSwcTree* oldTree, m_swcList) {
        new ZStackDocCommand::SwcEdit::RemoveSwc(this, oldTree, command);
      }
      new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);

      pushUndoCommand(command);
    } else {
      tree = swcList()->front();
    }
    tree->resortId();
    tree->save(filePath.c_str());
    tree->setSource(filePath);
    qDebug() << filePath.c_str();

    return true;
  }

  return false;
}

void ZStackDoc::initTraceWorkspace()
{
  if (m_stack == NULL || m_stack->channelNumber() != 1) {
    m_traceWorkspace =
        Locseg_Chain_Default_Trace_Workspace(m_traceWorkspace, NULL);
  } else {
    m_traceWorkspace =
        Locseg_Chain_Default_Trace_Workspace(m_traceWorkspace,
                                             m_stack->c_stack());
  }
  if (m_traceWorkspace->fit_workspace == NULL) {
    m_traceWorkspace->fit_workspace = New_Locseg_Fit_Workspace();
  }

  m_traceWorkspace->tune_end = TRUE;
  m_traceWorkspace->add_hit = TRUE;


  if (m_stack != NULL) {
    if (m_stack->depth() == 1) {
      m_traceWorkspace->min_score = 0.5;
      Receptor_Fit_Workspace *rfw =
          (Receptor_Fit_Workspace*) m_traceWorkspace->fit_workspace;
      Default_R2_Rect_Fit_Workspace(rfw);
      rfw->sws->fs.n = 2;
      rfw->sws->fs.options[1] = STACK_FIT_CORRCOEF;
    }
  }
}

void ZStackDoc::initConnectionTestWorkspace()
{
  if (m_connectionTestWorkspace == NULL) {
    m_connectionTestWorkspace = New_Connection_Test_Workspace();
  }
}

void ZStackDoc::updateTraceWorkspace(int traceEffort, bool traceMasked,
                                     double xRes, double yRes, double zRes)
{
  if (traceEffort > 0) {
    m_traceWorkspace->refit = FALSE;
  } else {
    m_traceWorkspace->refit = TRUE;
  }

  m_traceWorkspace->resolution[0] = xRes;
  m_traceWorkspace->resolution[1] = yRes;
  m_traceWorkspace->resolution[2] = zRes;

  loadTraceMask(traceMasked);
}

void ZStackDoc::updateConnectionTestWorkspace(
    double xRes, double yRes, double zRes,
    char unit, double distThre, bool spTest, bool crossoverTest)
{
  m_connectionTestWorkspace->resolution[0] = xRes;
  m_connectionTestWorkspace->resolution[1] = yRes;
  m_connectionTestWorkspace->resolution[2] = zRes;
  m_connectionTestWorkspace->unit = unit;
  m_connectionTestWorkspace->dist_thre = distThre;
  m_connectionTestWorkspace->sp_test = spTest;
  m_connectionTestWorkspace->crossover_test = crossoverTest;
}

bool ZStackDoc::isEmpty()
{
  return (!hasStackData()) && (!hasObject());
}

bool ZStackDoc::hasObject()
{
  return !m_objs.isEmpty();
}

bool ZStackDoc::hasSwc()
{
  return !m_swcList.isEmpty();
  //return !m_swcObjects.isEmpty();
}

bool ZStackDoc::hasSwcList()
{
  return !m_swcList.isEmpty();
}

ZResolution ZStackDoc::stackResolution() const
{
  if (hasStackData())
    return m_stack->resolution();
  else
    return ZResolution();
}

QString ZStackDoc::stackSourcePath() const
{
  if (hasStackData())
    return m_stack->sourcePath();
  else
    return QString();
}

bool ZStackDoc::hasChainList()
{
  return !m_chainList.isEmpty();
}

bool ZStackDoc::isUndoClean()
{
  return m_undoStack->isClean();
}

void ZStackDoc::swcTreeTranslateRootTo(double x, double y, double z)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->translateRootTo(x, y, z);
  }
  if (!m_swcList.empty()) {
    emit swcModified();
  }
}

void ZStackDoc::swcTreeRescale(double scaleX, double scaleY, double scaleZ)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->rescale(scaleX, scaleY, scaleZ);
  }
}

void ZStackDoc::swcTreeRescale(double srcPixelPerUmXY, double srcPixelPerUmZ,
                               double dstPixelPerUmXY, double dstPixelPerUmZ)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->rescale(srcPixelPerUmXY, srcPixelPerUmZ,
                          dstPixelPerUmXY, dstPixelPerUmZ);
  }
}

void ZStackDoc::swcTreeRescaleRadius(double scale, int startdepth, int enddepth)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->rescaleRadius(scale, startdepth, enddepth);
  }
}

void ZStackDoc::swcTreeReduceNodeNumber(double lengthThre)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->reduceNodeNumber(lengthThre);
  }
}

void ZStackDoc::deleteSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      SwcTreeNode::detachParent(*iter);
      Swc_Tree_Node *child = (*iter)->first_child;
      while (child != NULL) {
        Swc_Tree_Node *nextChild = child->next_sibling;
        Swc_Tree_Node_Detach_Parent(child);
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(child);
        addSwcTree(tree, false);
        child = nextChild;
      }
#ifdef _DEBUG_
      std::cout << "Node deleted: " << SwcTreeNode::toString(*iter) << std::endl;
#endif
      Kill_Swc_Tree_Node(*iter);
    }
    nodeSet->clear();

    removeEmptySwcTree();

    notifySwcModified();
  }
}

void ZStackDoc::addSizeForSelectedSwcNode(double dr)
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    static const double minRadius = 0.5;
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      double newRadius = SwcTreeNode::radius(*iter) + dr;
      if (newRadius < minRadius) {
        newRadius = minRadius;
      }
      SwcTreeNode::setRadius(*iter, newRadius);
    }

    emit swcModified();
  }
}

void ZStackDoc::selectSwcNodeConnection()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  Swc_Tree_Node *ancestor = *(nodeSet->begin());

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    ancestor = SwcTreeNode::commonAncestor(ancestor, *iter);
  }

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    while (tn != NULL) {
      if (SwcTreeNode::isRegular(tn)) {
        setSwcTreeNodeSelected(tn, true);
      }
      if (tn == ancestor) {
        break;
      }
      tn = SwcTreeNode::parent(tn);
    }
  }
}

void ZStackDoc::selectUpstreamNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> upstreamNodes;

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node* tn = *iter;
    while (tn != NULL && !Swc_Tree_Node_Is_Root(tn) && Swc_Tree_Node_Type(tn) != 1) {
      upstreamNodes.insert(tn);
      tn = tn->parent;
    }
  }
  setSwcTreeNodeSelected(upstreamNodes.begin(), upstreamNodes.end(), true);
}

void ZStackDoc::selectBranchNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> branchNodes;

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node* tn = *iter;
    while (SwcTreeNode::isRegular(tn) && !Swc_Tree_Node_Is_Branch_Point_S(tn)) {
      branchNodes.insert(tn);
      tn = tn->parent;
    }
    tn = *iter;
    while (SwcTreeNode::isRegular(tn) && !Swc_Tree_Node_Is_Branch_Point_S(tn)) {
      branchNodes.insert(tn);
      tn = tn->first_child;
    }
  }
  setSwcTreeNodeSelected(branchNodes.begin(), branchNodes.end(), true);
}

void ZStackDoc::selectTreeNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<ZSwcTree*> trees;
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    trees.insert(nodeToSwcTree(*iter));
  }
  std::vector<Swc_Tree_Node*> treeNodes;
  for (std::set<ZSwcTree*>::iterator iter = trees.begin(); iter != trees.end(); ++iter) {
    ZSwcTree* tree = *iter;
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end(); tn = tn->next)
      treeNodes.push_back(tn);
  }
  setSwcTreeNodeSelected(treeNodes.begin(), treeNodes.end(), true);
}

void ZStackDoc::selectConnectedNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> regularRoots;
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node* tn = *iter;
    while (tn != NULL && !Swc_Tree_Node_Is_Regular_Root(tn))
      tn = tn->parent;
    if (tn)
      regularRoots.insert(tn);
  }
  std::vector<Swc_Tree_Node*> treeNodes;
  for (std::set<Swc_Tree_Node*>::iterator iter = regularRoots.begin(); iter != regularRoots.end(); ++iter) {
    ZSwcTree* tree = nodeToSwcTree(*iter);
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, *iter, FALSE);
    for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end(); tn = tn->next)
      treeNodes.push_back(tn);
  }
  setSwcTreeNodeSelected(treeNodes.begin(), treeNodes.end(), true);
}

void ZStackDoc::selectNeighborSwcNode()
{
  QList<Swc_Tree_Node*> selected;
  //QList<Swc_Tree_Node*> deselected;
  for (std::set<Swc_Tree_Node*>::const_iterator
       iter = selectedSwcTreeNodes()->begin();
       iter != selectedSwcTreeNodes()->end(); ++iter) {
    const Swc_Tree_Node *tn = *iter;
    std::vector<Swc_Tree_Node*> neighborArray = SwcTreeNode::neighborArray(tn);
    for (std::vector<Swc_Tree_Node*>::iterator nbrIter = neighborArray.begin();
         nbrIter != neighborArray.end(); ++nbrIter) {
      selected.append(*nbrIter);
    }
  }

#ifdef _DEBUG_
  qDebug() << selected.size() << "Neighbor selected";
#endif

  setSwcTreeNodeSelected(selected.begin(), selected.end(), true);
  //emit swcTreeNodeSelectionChanged(selected, deselected);
}

void ZStackDoc::hideSelectedPuncta()
{
  for (std::set<ZPunctum*>::iterator it = selectedPuncta()->begin();
       it != selectedPuncta()->end(); ++it) {
    setPunctumVisible(*it, false);
  }
}

void ZStackDoc::showSelectedPuncta()
{
  for (std::set<ZPunctum*>::iterator it = selectedPuncta()->begin();
       it != selectedPuncta()->end(); ++it) {
    setPunctumVisible(*it, true);
  }
}

void ZStackDoc::selectSwcNodeNeighbor()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    setSwcTreeNodeSelected(SwcTreeNode::parent(tn), true);
    tn = tn->first_child;
    while (tn != NULL) {
      setSwcTreeNodeSelected(tn, true);
      tn = SwcTreeNode::nextSibling(tn);
    }
  }
}

void ZStackDoc::updateVirtualStackSize()
{
  if (!hasStackData()) {
    Stack *stack = new Stack;
    double corner[6] = {.0, .0, .0, .0, .0, .0};
    for (int i = 0; i < m_swcList.size(); i++) {
      double tmpcorner[6];
      m_swcList[i]->boundBox(tmpcorner);
      corner[3] = std::max(corner[3], tmpcorner[3]);
      corner[4] = std::max(corner[4], tmpcorner[4]);
      corner[5] = std::max(corner[5], tmpcorner[5]);
    }
    static const double Lateral_Margin = 10.0;
    static const double Axial_Margin = 1.0;
    Stack_Set_Attribute(stack, round(corner[3] + Lateral_Margin),
        round(corner[4] + Lateral_Margin),
        round(corner[5] + Axial_Margin),
        GREY);
    stack->array = NULL;
    loadStack(stack, true);
  }
}

bool ZStackDoc::hasDrawable()
{
  return !m_drawableList.isEmpty();
}

int ZStackDoc::stackWidth() const
{
  if (stack() == NULL) {
    return 0;
  }

  return stack()->width();
}

int ZStackDoc::stackHeight() const
{
  if (stack() == NULL) {
    return 0;
  }

  return stack()->height();
}

int ZStackDoc::stackChannelNumber() const
{
  if (hasStackData())
    return m_stack->channelNumber();
  else
    return 0;
}

ZStack*& ZStackDoc::stackRef()
{
  return m_stack;
}

void ZStackDoc::loadStack(Stack *stack, bool isOwner)
{
  if (stack == NULL)
    return;

  deprecate(STACK);
  ZStack* &mainStack = stackRef();
  mainStack = new ZStack;

  if (mainStack != NULL) {
    mainStack->load(stack, isOwner);
    initTraceWorkspace();
    initConnectionTestWorkspace();
    emit stackModified();
  }
}

void ZStackDoc::loadStack(ZStack *zstack)
{
  if (zstack == NULL)
    return;

  // load it only when the pointer is different
  ZStack* &mainStack = stackRef();

  if (zstack != mainStack) {
    deprecate(STACK);
    mainStack = zstack;
    initTraceWorkspace();
    initConnectionTestWorkspace();
    emit stackModified();
  }
}

void ZStackDoc::loadReaderResult()
{
  deprecate(STACK);

  ZStack*& mainStack = stackRef();
  mainStack = m_reader.getStack();

  if (mainStack != NULL) {
    if (mainStack->data() != NULL) {
      initTraceWorkspace();
      initConnectionTestWorkspace();
      setStackSource(m_reader.getStackFile()->firstUrl().c_str());
    }
  }

#ifdef _DEBUG_2
  std::cout << "emit stackLoaded()" << std::endl;
#endif

  emit stackLoaded();
}

void ZStackDoc::selectDownstreamNode()
{
#ifdef _DEBUG_
  std::cout << "Select downstream" << std::endl;
#endif

  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node_Build_Downstream_List(*iter);
    Swc_Tree_Node *tn = *iter;
    while (tn != NULL) {
      setSwcTreeNodeSelected(tn, true);
      tn = tn->next;
    }
  }
}

void ZStackDoc::readStack(const char *filePath)
{
  m_stackSource.import(filePath);

  m_reader.setStackFile(&m_stackSource);
  m_reader.start();

  /*
  deprecate(STACK);

  ZStack*& mainStack = stackRef();
  ZStackFile file;
  file.import(filePath);
  mainStack = file.readStack();

  emit stackLoaded();
*/
}

bool ZStackDoc::importImageSequence(const char *filePath)
{
  ZStackFile file;
  file.importImageSeries(filePath);

  deprecate(STACK);

  ZStack*& mainStack = stackRef();
  mainStack = file.readStack();

  if (mainStack == NULL) {
    return false;
  }

  if (mainStack->data() == NULL) {
    delete mainStack;
    mainStack = NULL;

    return false;
  } else {
    initTraceWorkspace();
    initConnectionTestWorkspace();
    setStackSource(filePath);
  }

  return true;
}

void ZStackDoc::readSwc(const char *filePath)
{
  removeAllObject(true);
  ZSwcTree *tree = new ZSwcTree;
  tree->load(filePath);
  if (!tree->hasData())
    return;
  Stack stack;
  double corner[6];
  tree->boundBox(corner);
  static const double Lateral_Margin = 10.0;
  static const double Axial_Margin = 1.0;
  Stack_Set_Attribute(&stack, round(corner[3] + Lateral_Margin - corner[0] + 1),
                      round(corner[4] + Lateral_Margin - corner[1] + 1),
                      round(corner[5] + Axial_Margin - corner[2] + 1),
                      GREY);
  /*
  stack->width = round(corner[3] + Lateral_Margin);
  stack->height = round(corner[4] + Lateral_Margin);
  stack->depth = round(corner[5] + Axial_Margin);
  stack->kind = GREY;
  */
  stack.array = NULL;
  loadStack(&stack, false);
  setStackSource(filePath);
  addSwcTree(tree);
  emit swcModified();
}

void ZStackDoc::loadSwcNetwork(const QString &filePath)
{
  loadSwcNetwork(filePath.toStdString().c_str());
}

void ZStackDoc::loadSwcNetwork(const char *filePath)
{
  if (m_swcNetwork == NULL) {
    m_swcNetwork = new ZSwcNetwork;
  }

  m_swcNetwork->importTxtFile(filePath);

  for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
    addSwcTree(m_swcNetwork->getTree(i));
  }
}

void ZStackDoc::importFlyEmNetwork(const char *filePath)
{
  if (m_swcNetwork != NULL) {
    delete m_swcNetwork;
  }

  FlyEm::ZNeuronNetwork flyemNetwork;
  flyemNetwork.import(filePath);
  flyemNetwork.layoutSwc();
  m_swcNetwork = flyemNetwork.toSwcNetwork();

  for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
    addSwcTree(m_swcNetwork->getTree(i));
  }
}

void ZStackDoc::setStackSource(const char *filePath)
{
  if (m_stack != NULL) {
    m_stack->setSource(filePath);
  }
}

bool ZStackDoc::hasStackData() const
{
  if (stack() != NULL) {
    if (stack()->data() != NULL) {
      if (!stack()->isVirtual()) {
        return true;
      }
    }
  }

  return false;
}

bool ZStackDoc::hasStackMask()
{
  bool maskAvailable = false;

  if(stackMask() != NULL) {
    if (stackMask()->channelNumber() > 0) {
      maskAvailable = true;
    }
  }

  return maskAvailable;
}

bool ZStackDoc::hasTracable()
{
  if (hasStackData()) {
    return stack()->isTracable();
  }

  return false;
}

ZPunctum* ZStackDoc::markPunctum(int x, int y, int z, double r)
{
  if (m_stack != NULL) {
    ZPunctum *zpunctum = new ZPunctum(x, y, z, r);
    zpunctum->setMaxIntensity(m_stack->value(x, y, z));
    zpunctum->setMeanIntensity(m_stack->value(x, y, z));
    zpunctum->updateVolSize();
    zpunctum->updateMass();
    zpunctum->setSource("manually marked");
    addPunctum(zpunctum);
    emit punctaModified();
    return zpunctum;
  }
  return NULL;
}

ZLocsegChain* ZStackDoc::fitseg(int x, int y, int z, double r)
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    Locseg_Fit_Workspace *ws =
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;

    if (ws->sws->field_func == Neurofield_Rpi) {
      return fitRpiseg(x, y, z, r);
    }

    Geo3d_Ball ball;
    ball.r = 3.0;
    ball.center[0] = x;
    ball.center[1] = y;
    ball.center[2] = z;
    //Geo3d_Ball_Mean_Shift(&ball, m_stack->data(), 1.0, 0.5);

    double pos[3];
    pos[0] = ball.center[0];
    pos[1] = ball.center[1];
    pos[2] = ball.center[2];

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, NEUROSEG_DEFAULT_H, TZ_PI_4,
                 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Local_Neuroseg_Optimize_W(locseg, mainStack->c_stack(),
                              mainStack->preferredZScale(), 1, ws);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::fitRpiseg(int x, int y, int z, double r)
{
  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    Geo3d_Ball ball;
    ball.r = 3.0;
    ball.center[0] = x;
    ball.center[1] = y;
    ball.center[2] = z;
    //Geo3d_Ball_Mean_Shift(&ball, m_stack->data(), 1.0, 0.5);

    double pos[3];
    pos[0] = ball.center[0];
    pos[1] = ball.center[1];
    pos[2] = ball.center[2];

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, NEUROSEG_DEFAULT_H, TZ_PI_4,
                 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_Locseg_Fit_Workspace(ws);

    Fit_Local_Neuroseg_W(locseg, mainStack->c_stack(),
                         mainStack->preferredZScale(), ws);

    Kill_Receptor_Fit_Workspace(ws);

    Local_Rpi_Neuroseg rpiseg;
    Local_Rpi_Neuroseg_From_Local_Neuroseg(&rpiseg, locseg);

    ws = New_Receptor_Fit_Workspace();
    Default_Rpi_Locseg_Fit_Workspace(ws);

    Fit_Local_Rpi_Neuroseg_W(&rpiseg, mainStack->c_stack(),
                             mainStack->preferredZScale(), ws);
//    Local_Rpi_Neuroseg_Optimize_W(&rpiseg, mainStack->data(),
//                              mainStack->preferredZScale(), 1, ws);

    Kill_Receptor_Fit_Workspace(ws);
    Local_Rpi_Neuroseg_To_Local_Neuroseg(&rpiseg, locseg);

    /*
    Local_Rpi_Neuroseg_Optimize_W(&rpiseg, mainStack->data(),
                                  mainStack->preferredZScale(), 1, ws);
                                  */

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::fitRect(int x, int y, int z, double r)
{
  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    ZLocalRect rect(x, y, z, 0.0, r);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_R2_Rect_Fit_Workspace(ws);
    rect.fitStack(mainStack->c_stack(), ws);

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    rect.toLocalNeuroseg(locseg);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

#ifdef _DEBUG_2
    ZLocalRect *new_rect = (ZLocalRect*) rect.extend();
    new_rect->fitStack(mainStack->data(), ws);
    locseg = new_rect->toLocalNeuroseg();
    Locseg_Chain_Add(locseg_chain, locseg, New_Trace_Record(), DL_TAIL);

    ZLocalRect *cur_rect = new_rect;
    for (int i = 0; i < 7; i++) {
      new_rect = (ZLocalRect*) cur_rect->extend();
      new_rect->fitStack(mainStack->data(), ws);
      locseg = new_rect->toLocalNeuroseg();
      Locseg_Chain_Add(locseg_chain, locseg, New_Trace_Record(), DL_TAIL);
      delete cur_rect;
      cur_rect = new_rect;
    }
    delete new_rect;
#endif

    Kill_Receptor_Fit_Workspace(ws);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::fitEllipse(int x, int y, int z, double r)
{
  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    Local_R2_Ellipse ellipse;
    Default_Local_R2_Ellipse(&ellipse);

    double center[3];
    center[0] = x;
    center[1] = y;
    center[2] = z;
    Local_R2_Ellipse_Set_Center(&ellipse, center);
    r = 3.0;
    Local_R2_Ellipse_Set_Size(&ellipse, r, r);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_R2_Ellipse_Fit_Workspace(ws);
    ws->pos_adjust = 0;

    Local_R2_Ellipse_Optimize_W(&ellipse, mainStack->c_stack(), 1.0, 1, ws);
    Kill_Receptor_Fit_Workspace(ws);

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Local_R2_Ellipse_To_Local_Neuroseg(&ellipse, locseg);

    Locseg_Chain *locsegChain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locsegChain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locsegChain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::dropseg(int x, int y, int z, double r)
{
  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());
    obj->setIgnorable(true);

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

void ZStackDoc::loadTraceMask(bool traceMasked)
{
  if (traceMasked) {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, m_traceWorkspace->trace_mask);
  } else {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, NULL);
  }
}

void ZStackDoc::refreshTraceMask()
{
  if (m_isTraceMaskObsolete) {
    if (m_traceWorkspace->trace_mask == NULL) {
      m_traceWorkspace->trace_mask =
          C_Stack::make(GREY, stack()->width(), stack()->height(),
                        stack()->depth());
    }
    Zero_Stack(m_traceWorkspace->trace_mask);

    foreach (ZSwcTree *tree, m_swcList) {
      tree->labelStack(m_traceWorkspace->trace_mask);
    }
    m_isTraceMaskObsolete = false;
  }
}

ZLocsegChain* ZStackDoc::traceTube(int x, int y, int z, double r, int c)
{
  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    //updateTraceWorkspace();

    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Set_Neuroseg(&(locseg->seg), r, 0.0, 11.0, TZ_PI_4, 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Locseg_Fit_Workspace *ws =
    	(Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
    Local_Neuroseg_Optimize_W(locseg, mainStack->c_stack(c),
                              mainStack->preferredZScale(), 1, ws);

    Trace_Record *tr = New_Trace_Record();
    tr->mask = ZERO_BIT_MASK;
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Node *p = Make_Locseg_Node(locseg, tr);
    Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

    Trace_Workspace_Set_Trace_Status(m_traceWorkspace, TRACE_NORMAL,
    		TRACE_NORMAL);
    Trace_Locseg(mainStack->c_stack(c), mainStack->preferredZScale(), locseg_chain,
    		m_traceWorkspace);
    Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
    Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    if (!obj->isEmpty()) {
      obj->setZScale(mainStack->preferredZScale());
      addLocsegChain(obj);
      emit chainModified();
      /*
      m_parent->setLocsegChainInfo(obj, "Traced: ",
                                   QString(" Confidence: %1")
                                   .arg(obj->confidence(mainStack->data(), 1.0)));
                                   */
      return obj;
    } else {
      /*
      m_parent->setLocsegChainInfo(NULL,
                                   "Tracing failed: no tube-like structure found nearby.");
                                   */
      delete obj;
    }
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::traceRect(int x, int y, int z, double r, int c)
{
  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    //updateTraceWorkspace();

    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Trace_Record *tr = New_Trace_Record();
    tr->mask = ZERO_BIT_MASK;
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);

    ZLocalRect *rect = new ZLocalRect(x, y, z, 0.0, r);

    Receptor_Fit_Workspace *rfw =
        (Receptor_Fit_Workspace*) m_traceWorkspace->fit_workspace;
    rect->fitStack(mainStack->c_stack(c), rfw);

    ZDirectionalTemplateChain chain;
    chain.append(rect);

    Trace_Workspace_Set_Trace_Status(m_traceWorkspace, TRACE_NORMAL,
                TRACE_NORMAL);
    chain.trace(mainStack, m_traceWorkspace);

    ZLocsegChain *obj = chain.toLocsegChain();
    if (!obj->isEmpty()) {
      obj->setZScale(mainStack->preferredZScale());
      addLocsegChain(obj);
      emit chainModified();
      /*
      m_parent->setLocsegChainInfo(obj, "Traced: ",
                                   QString(" Confidence: %1")
                                   .arg(obj->confidence(mainStack->data(), 1.0)));
                                   */
      return obj;
    } else {
      /*
      m_parent->setLocsegChainInfo(NULL,
                                   "Tracing failed: no tube-like structure found nearby.");
                                   */
      delete obj;
    }
  }

  return NULL;
}

void ZStackDoc::cutLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain *> *pResult)
{
  //ZLocsegChain *chain = obj;
  if (pResult) {
    pResult->clear();
  }

  if (obj->heldNode() >= 0) {
    removeObject((ZInterface*) obj, false);
    ZLocsegChain *chain = new ZLocsegChain(*obj);
    ZLocsegChain *new_chain = chain->cutHeldNode();
    if (new_chain != NULL) {
      addLocsegChain(new_chain);
      emit chainModified();
      if (pResult) {
        pResult->append(new_chain);
      }
    }
    if (chain->isEmpty() == false) {
      addLocsegChain(chain);
      emit chainModified();
      if (pResult) {
        pResult->append(chain);
      }
    } else {
      delete chain;
    }
  }
}

void ZStackDoc::breakLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain *> *pResult)
{
  //ZLocsegChain *chain = obj;
  if (pResult) {
    pResult->clear();
  }

  removeObject((ZInterface*) obj, false);

  ZLocsegChain *chain = new ZLocsegChain(*obj);
  ZLocsegChain *newChain = chain->breakBetween(0, chain->length() - 1);

  if (newChain->isEmpty() == false) {
    addLocsegChain(newChain);
    emit chainModified();
    if (pResult) {
      pResult->append(newChain);
    }
  } else {
    delete newChain;
  }

  if (chain->isEmpty() == false) {
    addLocsegChain(chain);
    emit chainModified();
    if (pResult) {
      pResult->append(chain);
    }
  } else {
    delete chain;
  }
}

void ZStackDoc::cutSelectedLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if (m_chainList.at(i)->isSelected() == true) {
      cutLocsegChain(m_chainList.at(i));
      break;
    }
  }
}

void ZStackDoc::breakSelectedLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if (m_chainList.at(i)->isSelected() == true) {
      breakLocsegChain(m_chainList.at(i));
      break;
    }
  }
}

int ZStackDoc::autoThreshold(Stack *stack)
{
  int thre = 0;
  if (stack->array != NULL) {
    int conn = 18;
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }

    int *hist = Stack_Hist_M(stack, locmax);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);

    thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);

    free(hist);
  }
  return thre;
}

int ZStackDoc::autoThreshold()
{
  int thre = 0;
  ZStack *mainStack = stack();

  if (!mainStack->isVirtual()) {
    m_progressReporter->start();
    Stack *stack = mainStack->c_stack();
    double scale = 1.0*stack->width * stack->height * stack->depth * stack->kind /
        (2.0*1024*1024*1024);
    if (scale >= 1.0) {
      scale = std::ceil(std::sqrt(scale + 0.1));
      stack = C_Stack::resize(stack, stack->width/scale, stack->height/scale, stack->depth);
    }

    int conn = 18;
    m_progressReporter->advance(0.1);
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    m_progressReporter->advance(0.1);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    m_progressReporter->advance(0.2);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }
    m_progressReporter->advance(0.1);

    int *hist = Stack_Hist_M(stack, locmax);
    m_progressReporter->advance(0.1);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);
    m_progressReporter->advance(0.1);

    thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);
    m_progressReporter->advance(0.1);
    free(hist);

    if (stack != mainStack->c_stack())
      C_Stack::kill(stack);
    m_progressReporter->end();
  }
  return thre;
}

void ZStackDoc::addSwcTree(ZSwcTree *obj, bool uniqueSource)
{
  if (obj == NULL) {
    return;
  }

  if (uniqueSource) {
    if (!obj->source().empty()) {
      QList<ZSwcTree*> treesToRemove;
      for (int i=0; i<m_swcList.size(); i++) {
        if (m_swcList.at(i)->source() == obj->source()) {
          treesToRemove.push_back(m_swcList.at(i));
        }
      }
      for (int i=0; i<treesToRemove.size(); i++) {
        removeObject(treesToRemove.at(i), true);
      }
    }
  }

  obj->forceVirtualRoot();
  m_objs.append(obj);
  m_swcList.append(obj);
  m_drawableList.append(obj);

  if (obj->isSelected()) {
    setSwcSelected(obj, true);
  }

  notifySwcModified();
}

void ZStackDoc::addSwcTree(const QList<ZSwcTree *> &swcList, bool uniqueSource)
{
  blockSignals(true);
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    addSwcTree(*iter, uniqueSource);
  }
  blockSignals(false);
}

void ZStackDoc::addPunctum(ZPunctum *obj)
{
  if (obj == NULL) {
    return;
  }

  m_objs.append(obj);
  m_punctaList.append(obj);
  m_drawableList.append(obj);

  if (obj->isSelected()) {
    setPunctumSelected(obj, true);
  }
}

void ZStackDoc::addObj3d(ZObject3d *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackDrawable::OBJECT_CANVAS);
  m_objs.append(obj);
  m_obj3dList.append(obj);
  m_drawableList.append(obj);
}

void ZStackDoc::addStroke(ZStroke2d *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackDrawable::OBJECT_CANVAS);
  m_objs.prepend(obj);
  m_strokeList.prepend(obj);
  m_drawableList.prepend(obj);
}

void ZStackDoc::addLocsegChainConn(ZLocsegChainConn *obj)
{
  if (obj == NULL) {
    return;
  }

  m_objs.append(obj);
  m_connList.append(obj);
  m_drawableList.append(obj);
}

void ZStackDoc::addLocsegChain(ZLocsegChain *obj)
{
  if (obj == NULL) {
    return;
  }

  ZStack *mainStack = stack();

  if (mainStack != NULL) {
    if (m_traceWorkspace->trace_mask == NULL) {
      m_traceWorkspace->trace_mask =
          Make_Stack(GREY16, mainStack->width(), mainStack->height(),
                     mainStack->depth());
      Zero_Stack(m_traceWorkspace->trace_mask);
    }
  }

  obj->setId(m_traceWorkspace->chain_id);
  obj->labelTraceMask(m_traceWorkspace->trace_mask);

  m_objs.append(obj);
  //m_swcObjects.append(obj);
  //m_vrmlObjects.append(obj);
  m_chainList.append(obj);
  m_drawableList.append(obj);

  m_traceWorkspace->chain_id++;

  if (obj->isSelected()) {
    setChainSelected(obj, true);
  }
}

void ZStackDoc::updateLocsegChain(ZLocsegChain *obj)
{
  if (obj != NULL) {
    obj->labelTraceMask(m_traceWorkspace->trace_mask);
  }
}

void ZStackDoc::exportSwc(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
  TZ_ERROR(ERROR_OBSOLETE_FUNC);
#if 0
  double z_scale = m_parent->xResolution() / m_parent->zResolution();
  if ((z_scale > 0.99) && (z_scale < 1.01)) {
    z_scale = 1.0;
  }

  int start_id = 0;
  if (m_swcObjects.size() > 0) {
    FILE *fp = fopen(filePath, "w");
    if (fp != NULL) {
      for (int i = 0; i < m_swcObjects.size(); i++) {
        start_id = m_swcObjects.at(i)->swcFprint(fp, start_id + 1, -1,
                                                 z_scale);
      }
      fclose(fp);
    }
  }
#endif
}

void ZStackDoc::exportPuncta(const char *filePath)
{
  ZPunctumIO::save(filePath, m_punctaList);
}

Swc_Tree* ZStackDoc::swcReconstruction(int rootOption, bool singleTree,
                                       bool removingOvershoot)
{
  Swc_Tree *tree = NULL;

 if (m_chainList.size() > 0) {
    int chain_number = m_chainList.size();

//    LINFO() << "";
//    for (int i=0; i<chain_number;++i) {
//      double minDist = 1e10;
//      int minDistTreeIdx = -1;
//      double headPos[3];
//      double tailPos[3];
//      m_chainList[i]->headPosition(headPos);
//      m_chainList[i]->tailPosition(tailPos);
//      for (int j=0; j<m_swcList.size(); ++j) {
//        double dist = m_swcList[j]->distanceTo(headPos[0], headPos[1], headPos[2], 5);
//        if (dist < minDist) {
//          minDist = dist;
//          minDistTreeIdx = j;
//        }
//        dist = m_swcList[j]->distanceTo(tailPos[0], tailPos[1], tailPos[2], 5);
//        if (dist < minDist) {
//          minDist = dist;
//          minDistTreeIdx = j;
//        }
//      }
//      if (minDist <= 30) {
//        LINFO() << i << minDistTreeIdx << minDist;
//      }
//    }
//    LINFO() << "";
//    return NULL;

    /* alloc <chain_array> */
    Neuron_Component *chain_array =
      Make_Neuron_Component_Array(chain_number);

    int chain_number2 = 0;

    for (int i = 0; i < chain_number; i++) {
      if ((m_chainList.at(i)->length() > 0) &&
          !(m_chainList.at(i)->isIgnorable())) {
        Set_Neuron_Component(chain_array + chain_number2,
                             NEUROCOMP_TYPE_LOCSEG_CHAIN,
                             Copy_Locseg_Chain(m_chainList.at(i)->data()));
        chain_number2++;
      }
    }

    /* reconstruct neuron */
    Connection_Test_Workspace *ctw = m_connectionTestWorkspace;

    double zscale = m_chainList.at(0)->zScale();

    /* alloc <ns> */
    ZStack *mainStack = stack();
    Neuron_Structure *ns = NULL;
    if (mainStack != NULL) {
      ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,
                                         mainStack->c_stack(), zscale, ctw);
    } else {
      ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,
                                         NULL, zscale, ctw);
    }

    Process_Neuron_Structure(ns);

    if (m_connectionTestWorkspace->crossover_test == TRUE) {
      Neuron_Structure_Crossover_Test(ns, zscale);
    }

#ifdef _DEBUG_2
    Neuron_Structure_To_Tree(ns);
    Neuron_Structure_Remove_Conn(ns, 1, 3);
    Neuron_Structure_Remove_Conn(ns, 3, 1);
#endif

    /* alloc <ns2> */
    Neuron_Structure* ns2=
      Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
    /*
                                                m_parent->xReconstructScale(),
                                                m_parent->zReconstructScale());
                                                */

    Neuron_Structure_To_Tree(ns2);

    /*
    Neuron_Structure_To_Swc_File_Circle_Z(ns2, filePath, m_stack->zscale(),
                                          NULL);
     */

    tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);
    Swc_Tree_Merge_Close_Node(tree, 0.01 * ctw->resolution[0]);

    Swc_Tree_Node *tn = NULL;
    Swc_Tree_Node *tmp_tn = NULL;

    switch (rootOption) {
    case 0:
      Swc_Tree_Iterator_Start(tree, 2, FALSE);
      tn = Swc_Tree_Next(tree);
      while ((tmp_tn = Swc_Tree_Next(tree)) != NULL) {
        if (Swc_Tree_Node_Data(tmp_tn)->d > Swc_Tree_Node_Data(tn)->d) {
          tn = tmp_tn;
        }
      }
      break;
    case 1:
      updateMasterLocsegChain();
      if (m_masterChain != NULL) {
        Local_Neuroseg *locseg = m_masterChain->heldNeuroseg();
        double pos[3];
        Local_Neuroseg_Center(locseg, pos);
        tn = Swc_Tree_Closest_Node(tree, pos);
      }
      break;
    default:
      break;
    }

    if (tn != NULL) {
      Swc_Tree_Node_Set_Root(tn);
      if (singleTree) {
        if (tn != tree->root) {
          Swc_Tree_Node_Detach_Parent(tn);
          Kill_Swc_Tree(tree);
          tree = New_Swc_Tree();
          tree->root = tn;
        }
      }
    }

    Swc_Tree_Remove_Zigzag(tree);
    Swc_Tree_Tune_Branch(tree);
    Swc_Tree_Remove_Spur(tree);
    Swc_Tree_Merge_Close_Node(tree, 0.01);
    if (removingOvershoot) {
      Swc_Tree_Remove_Overshoot(tree);
    }

    Swc_Tree_Resort_Id(tree);

    /* free <ns2> */
    Kill_Neuron_Structure(ns2);
    /* free <ns> */
    ns->comp = NULL;
    Kill_Neuron_Structure(ns);

    /* free <chain_array> */
    Clean_Neuron_Component_Array(chain_array, chain_number);
    free(chain_array);
  }

 return tree;
}

ZSwcTree *ZStackDoc::nodeToSwcTree(Swc_Tree_Node *node) const
{
  for (int i=0; i<m_swcList.size(); ++i) {
    if (m_swcList[i]->contains(node))
      return m_swcList[i];
  }
  assert(false);
  return NULL;
}

void ZStackDoc::exportSwcTree(const char *filePath)
{
  //Need modification
  Swc_Tree *tree = swcReconstruction(0, false, true);
  if (tree != NULL) {
    Write_Swc_Tree(filePath, tree);
    Kill_Swc_Tree(tree);
  }
}

int ZStackDoc::exportMultipleSwcTree(const QString &filepath)
{
  int total = 0;
  //Need modification
  Swc_Tree *tree = swcReconstruction(0, false, true);
  if (tree != NULL) {
    Swc_Tree_Node *tn = tree->root;
    if (Swc_Tree_Node_Is_Virtual(tn)) {
      Swc_Tree_Node *child = tn->first_child;
      tn = child;
      while (tn != NULL) {
        total++;
        QString filename = QString("%1_%2.swc").arg(filepath).arg(total, 4, 10, QChar('0'));
        Swc_Tree tmp_tree;
        tmp_tree.root = tn;
        Write_Swc_Tree(filename.toLocal8Bit().constData(), &tmp_tree);
        child = tn->next_sibling;
        tn = child;
      }
    }
    Kill_Swc_Tree(tree);
  }
  return total;
}

void ZStackDoc::exportVrml(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
  TZ_ERROR(ERROR_OBSOLETE_FUNC);
  /*
  if (m_vrmlObjects.size() > 0) {
    FILE *fp = fopen(filePath, "w");
    if (fp != NULL) {
      Vrml_Head_Fprint(fp, "V2.0", "utf8");

      Vrml_Material *material = New_Vrml_Material();
      Vrml_SFVec3f_Set(material->diffuse_color, 0.0, 0.8, 0.0);

      Rgb_Color color;

      double hue = 0.0;
      Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material->default_mask));

      double step = 1.0 / m_vrmlObjects.size();

      for (int i = 0; i < m_vrmlObjects.size(); i++) {
        hue = i % 6;
        hue += step * i;
        while (hue > 6.0) {
          hue -= 6.0;
        }
        Set_Color_Hsv(&color, hue, 1.0, 1.0);
        Vrml_SFVec3f_Set(material->diffuse_color, (double)color.r / 255.0,
                         (double)color.g / 255.0, (double)color.b / 255.0);
        m_vrmlObjects.at(i)->vrmlFprint(fp, material);
      }

      Delete_Vrml_Material(material);
      fclose(fp);
    }
  }
  */
}

void ZStackDoc::exportSvg(const char *filePath)
{
  if (!m_swcList.isEmpty()) {
    m_swcList.at(0)->toSvgFile(filePath);
  }
}

void ZStackDoc::exportBinary(const char *prefix)
{
  if (m_objs.size() > 0) {
    char *filePath = new char[strlen(prefix) + 10];
    for (int i = 0; i < m_chainList.size(); i++) {
      sprintf(filePath, "%s%d.tb", prefix, i);
      m_chainList.at(i)->save(filePath);
    }
    ////   ?
    int startNum = m_chainList.size();
    for (int i = 0; i < m_swcList.size(); i++) {
      startNum = m_swcList.at(i)->saveAsLocsegChains(prefix, startNum);
    }
    delete []filePath;
    emit chainModified();  // chain source is modified after saving
  }
}

void ZStackDoc::exportChainFileList(const char *filepath)
{
  QFile file(filepath);
  file.open(QIODevice::WriteOnly);
  QTextStream stream(&file);
  for (int i = 0; i < m_chainList.size(); i++) {
    stream << m_chainList.at(i)->source() << '\n';
  }
  file.close();
}

void ZStackDoc::exportLocsegChainConnFeat(const char *filePath)
{
  double feat[9];
  double res[3];
  res[0] = m_traceWorkspace->resolution[0];// m_parent->xResolution();
  res[1] = m_traceWorkspace->resolution[1];//m_parent->yResolution();
  res[2] = m_traceWorkspace->resolution[2];//m_parent->zResolution();

  FILE *fp = fopen(filePath, "w");

  for (int i = 0; i < m_chainList.size(); i++) {
    for (int j = 0; j < m_chainList.size(); j++) {
      if (i != j) {
        ZLocsegChain *hook = m_chainList.at(i);
        ZLocsegChain *loop = m_chainList.at(j);
        bool found = false;
        for (int k = 0; k < m_connList.size(); k++) {
          if (m_connList.at(k)->has(hook, loop)) {
            found = true;
            break;
          }
        }

        int label;
        if (found) {
          label = 1;
        } else {
          label = 0;
        }

        int n;
        Locseg_Chain_Conn_Feature(hook->data(), loop->data(), NULL, res, feat, &n);
        fprintf(fp, "%d ", label);
        for (int u = 0; u < n; u++) {
          fprintf(fp, "%g ", feat[u]);
        }
        fprintf(fp, "\n");
      }
    }
  }

  fclose(fp);
}

int ZStackDoc::xmlConnNode(QXmlStreamReader *xml,
                           QString *filePath, int *spot)
{
  int succ = 0;
  *spot = -1;
  while (!xml->atEnd()) {
    QXmlStreamReader::TokenType token = xml->readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (xml->name() == "filePath") {
        if (xml->readNext() == QXmlStreamReader::Characters) {
          *filePath = xml->text().toString();
          succ = 1;
        }
      } else if (xml->name() == "spot") {
        if (xml->readNext() == QXmlStreamReader::Characters) {
          *spot = xml->text().toString().toInt();
          succ = 2;
        }
      }
    } else if (token == QXmlStreamReader::EndElement) {
      if ((xml->name() == "loop") || (xml->name() == "hook")){
        break;
      }
    }
  }

  return succ;
}

int ZStackDoc::xmlConnMode(QXmlStreamReader *xml)
{
  int mode = NEUROCOMP_CONN_HL;

  while (!xml->atEnd()) {
    QXmlStreamReader::TokenType token = xml->readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (xml->name() == "mode") {
        if (xml->readNext() == QXmlStreamReader::Characters) {
          mode = xml->text().toString().toInt();
        }
      }
    } else if (token == QXmlStreamReader::EndElement) {
      break;
    }
  }

  return mode;
}

void ZStackDoc::importLocsegChainConn(const char *filePath)
{
  QFile file(filePath);
  file.open(QIODevice::ReadOnly);
  QXmlStreamReader xml(&file);

  while (!xml.atEnd()) {
    QXmlStreamReader::TokenType token = xml.readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (xml.name() == "connection") {
        QString hookFile, loopFile;
        int hook_spot, loop_spot;
        xmlConnNode(&xml, &hookFile, &hook_spot);
        xmlConnNode(&xml, &loopFile, &loop_spot);
        int mode = xmlConnMode(&xml);

        int loopIndex, hookIndex;

        loopIndex = -1;
        hookIndex = -1;

        for (int i = 0; i < m_chainList.size(); i++) {
          if (hookIndex < 0) {
            if (m_chainList.at(i)->source() == hookFile) {
              hookIndex = i;
            }
          }
          if (loopIndex < 0) {
            if (m_chainList.at(i)->source() == loopFile) {
              loopIndex = i;
            }
          }
        }

        if ((hookIndex >= 0) && (loopIndex >= 0)) {
          addLocsegChainConn(m_chainList.at(hookIndex),
                             m_chainList.at(loopIndex),
                             hook_spot, loop_spot, mode);
        }
      }
    }
  }
  file.close();
}

void ZStackDoc::exportLocsegChainConn(const char *filePath)
{
  QFile file(filePath);
  file.open(QIODevice::WriteOnly);
  QXmlStreamWriter xml(&file);
  xml.setAutoFormatting(true);
  xml.setAutoFormattingIndent(2);

  xml.writeStartDocument("1.0");
  xml.writeStartElement("tubeConn");
  for (int i = 0; i < m_connList.size(); i++) {
    m_connList.at(i)->translateMode();
    m_connList.at(i)->writeXml(xml);
  }
  xml.writeEndElement();
  xml.writeEndDocument();
  file.close();
}

void ZStackDoc::importLocsegChain(const QStringList &fileList,
                                  TubeImportOption option,
                                  LoadObjectOption objopt)
{
  if (fileList.empty())
    return;
  if (objopt == REPLACE_OBJECT) {
    removeAllObject(true);
  }

  QString file;
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZLocsegChain*> chainsToRemove;
      for (int i=0; i<m_chainList.size(); i++) {
        if (m_chainList.at(i)->source() == file) {
          chainsToRemove.push_back(m_chainList.at(i));
        }
      }
      for (int i=0; i<chainsToRemove.size(); i++) {
        removeObject(chainsToRemove.at(i), true);
      }
    }
    ZLocsegChain *chain = new ZLocsegChain();

    qDebug() << file.toLocal8Bit().constData() << "\n";

    chain->load(file.toLocal8Bit().constData());

    if (!chain->isEmpty()) {
      bool loadIt = true;

      if (option != ALL_TUBE) {
        double conf = chain->confidence(stack()->c_stack());
        qDebug() << conf << "\n";
        if (option == GOOD_TUBE) {
          if (conf < 0.5) {
            loadIt = false;
          }
        } else if (option == BAD_TUBE) {
          if (conf >= 0.5) {
            loadIt = false;
          }
        }
      }

      if (loadIt == true) {
        addLocsegChain(chain);
      } else {
        delete chain;
      }
    }
  }
  emit chainModified();
}

void ZStackDoc::importGoodTube(const char *dirpath, const char *prefix,
                               QProgressBar *pb)
{
  UNUSED_PARAMETER(pb);

  char file_path[MAX_PATH_LENGTH];

  QStringList fileList;

  sprintf(file_path, "^%s.*\\.tb", m_traceWorkspace->save_prefix);

  if (dirpath == NULL) {
    dirpath = m_traceWorkspace->save_path;
  }

  if (prefix == NULL) {
    prefix = m_traceWorkspace->save_prefix;
  }

  int n = dir_fnum_s(dirpath, file_path);

  int *mask = (int *) Guarded_Calloc(n + 1, sizeof(int), "main");
  int i;
  for (i = 0; i <= n; i++) {
    sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
    if (fexist(file_path)) {
      mask[i] = 1;
    }
  }


  TubeImportOption importOption = ALL_TUBE;

  if (!m_badChainScreen.isEmpty()) {
    if (m_badChainScreen != "auto") {
      sprintf(file_path, "%s/%s", dirpath, m_badChainScreen.toLocal8Bit().constData());

      if (QFile(QString(file_path)).exists()) {
        FILE *fp = fopen(file_path, "r");
        String_Workspace *sw = New_String_Workspace();
        char *line;
        while ((line = Read_Line(fp, sw)) != NULL) {
          int id = String_Last_Integer(line);
          if (id >= 0) {
            mask[id] = 2; //bad tubes
          }
        }
        Kill_String_Workspace(sw);
        fclose(fp);
      }
    } else {
      importOption = GOOD_TUBE;
    }
  }

  for (i = 0; i <= n; i++) {
    if (mask[i] == 1) {
      /* Read the tube file */
      sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
      fileList.append(QString(file_path));
    }
  }

  free(mask);

  importLocsegChain(fileList, importOption);
}

void ZStackDoc::importBadTube(const char *dirpath, const char *prefix)
{
  char file_path[100];
  sprintf(file_path, "%s/badtube.txt", dirpath);

  if (QFile(QString(file_path)).exists() == false) {
    QMessageBox::warning(NULL, tr("Operation Failed"),
                         tr("There is no good/bad tube information."));
    return;
  }

  if (prefix == NULL) {
    prefix = m_traceWorkspace->save_prefix;
  }

  QStringList fileList;

  int n = dir_fnum_s(dirpath, "^chain.*\\.tb");
  int *mask = (int *) Guarded_Calloc(n + 1, sizeof(int), "main");
  int i;
  for (i = 0; i <= n; i++) {
    sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
    if (fexist(file_path)) {
      mask[i] = 1;
    }
  }

  sprintf(file_path, "%s/badtube.txt", dirpath);

  FILE *fp = fopen(file_path, "r");
  String_Workspace *sw = New_String_Workspace();
  char *line;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int id = String_Last_Integer(line);
    if (id >= 0) {
      mask[id] = 2; //bad tubes
    }
  }
  Kill_String_Workspace(sw);
  fclose(fp);

  for (i = 0; i <= n; i++) {
    if (mask[i] == 2) {
      /* Read the tube file */
      sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
      fileList.append(QString(file_path));
    }
  }

  free(mask);

  importLocsegChain(fileList);
}

void ZStackDoc::loadSwc(const QString &filePath)
{
  ZSwcTree *tree = new ZSwcTree();
  tree->load(filePath.toLocal8Bit().constData());
  addSwcTree(tree);
}

void ZStackDoc::loadLocsegChain(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    QList<ZLocsegChain*> chainsToRemove;
    for (int i=0; i<m_chainList.size(); i++) {
      if (m_chainList.at(i)->source() == filePath) {
        chainsToRemove.push_back(m_chainList.at(i));
      }
    }
    for (QList<ZLocsegChain*>::iterator iter = chainsToRemove.begin();
         iter != chainsToRemove.end(); ++iter) {
      removeObject(*iter, true);
    }
  }

  ZLocsegChain *chain = new ZLocsegChain();

  chain->load(filePath.toLocal8Bit().constData());

  addLocsegChain(chain);
}

void ZStackDoc::importSwc(QStringList fileList, LoadObjectOption objopt)
{
  if (fileList.empty())
    return;
  if (objopt == REPLACE_OBJECT) {
    removeAllObject(true);
  }

  QString file;
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZSwcTree*> treesToRemove;
      for (int i=0; i<m_swcList.size(); i++) {
        if (m_swcList.at(i)->source() == file.toStdString()) {
          treesToRemove.push_back(m_swcList.at(i));
        }
      }
      for (int i=0; i<treesToRemove.size(); i++) {
        removeObject(treesToRemove.at(i), true);
      }
    }

    if (file.endsWith(".swc", Qt::CaseInsensitive)) {
      ZSwcTree *tree = new ZSwcTree();
      tree->load(file.toLocal8Bit().constData());
      addSwcTree(tree);
    } else if (file.endsWith(".json", Qt::CaseInsensitive))  {
      importSynapseAnnotation(file.toStdString());
    }
  }
  emit swcModified();
}

bool ZStackDoc::importPuncta(const char *filePath)
{
  QStringList fileList;
  fileList.append(filePath);

  importPuncta(fileList);

  return true;
}

void ZStackDoc::importPuncta(const QStringList &fileList, LoadObjectOption objopt)
{
  if (fileList.empty())
    return;
  if (objopt == REPLACE_OBJECT) {
    removeAllObject();
  }

  QString file;
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZPunctum*> punctaToRemove;
      for (int i=0; i<m_punctaList.size(); i++) {
        if (m_punctaList.at(i)->source() == file) {
          punctaToRemove.push_back(m_punctaList.at(i));
        }
      }
      for (int i=0; i<punctaToRemove.size(); i++) {
        removeObject(punctaToRemove.at(i), true);
      }
    }
    QList<ZPunctum*> plist = ZPunctumIO::load(file);
    for (int i=0; i<plist.size(); i++) {
      addPunctum(plist[i]);
    }
  }
  emit punctaModified();
}

void ZStackDoc::clearLocsegChainConn()
{
  while (!m_connList.isEmpty()) {
    ZLocsegChainConn *obj = m_connList.takeLast();
    removeObject(obj, true);
  }
}

void ZStackDoc::buildLocsegChainConn()
{
  clearLocsegChainConn();

  if (m_chainList.size() > 0) {
    int chain_number = m_chainList.size();

    /* alloc <chain_array> */
    Neuron_Component *chain_array =
        Make_Neuron_Component_Array(chain_number);

    for (int i = 0; i < chain_number; i++) {
      Set_Neuron_Component(chain_array + i,
                           NEUROCOMP_TYPE_LOCSEG_CHAIN,
                           m_chainList.at(i)->data());
    }

    double zscale = m_chainList.at(0)->zScale();

    /* alloc <ns> */
    BOOL old_interpolate = m_connectionTestWorkspace->interpolate;
    m_connectionTestWorkspace->interpolate = FALSE;
    Neuron_Structure *ns = NULL;
    ZStack *mainStack = stack();
    if (mainStack != NULL) {
      Locseg_Chain_Comp_Neurostruct(
            chain_array, chain_number, mainStack->c_stack(), zscale,
            m_connectionTestWorkspace);
    } else {
      Locseg_Chain_Comp_Neurostruct(
            chain_array, chain_number, NULL, zscale,
            m_connectionTestWorkspace);
    }
    m_connectionTestWorkspace->interpolate = old_interpolate;

    Process_Neuron_Structure(ns);

    if (m_connectionTestWorkspace->crossover_test) {
      Neuron_Structure_Crossover_Test(ns, zscale);
    }

    Neuron_Structure_To_Tree(ns);

    for (int i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
      ZLocsegChain *hook = m_chainList.at(ns->graph->edges[i][0]);
      ZLocsegChain *loop = m_chainList.at(ns->graph->edges[i][1]);
      int hook_spot = ns->conn[i].info[0];
      int loop_spot = ns->conn[i].info[1];


      ZLocsegChainConn *conn =
          new ZLocsegChainConn(hook, loop,
                               hook_spot, loop_spot, ns->conn[i].mode);

      addLocsegChainConn(conn);
    }

    /* free <ns>, including <chain_array> */
    free(chain_array);
    ns->comp = NULL;
    Kill_Neuron_Structure(ns);
  }
}

void ZStackDoc::selectNeighbor()
{
  if (m_masterChain != NULL) {
    for (int i = 0; i < m_connList.size(); i++) {
      if (m_connList.at(i)->isHook(m_masterChain)) {
        //m_connList.at(i)->loopChain()->setSelected(true);
        setChainSelected(m_connList.at(i)->loopChain(), true);
      } else if (m_connList.at(i)->isLoop(m_masterChain)) {
        //m_connList.at(i)->hookChain()->setSelected(true);
        setChainSelected(m_connList.at(i)->hookChain(), true);
      }
    }
  }
}

void ZStackDoc::selectConnectedChain()
{
  if (m_masterChain != NULL) {
    QQueue<ZLocsegChain*> queue;
    queue.enqueue(m_masterChain);
    while (!queue.isEmpty()) {
      ZLocsegChain *cur = queue.dequeue();
      for (int i = 0; i < m_connList.size(); i++) {
        if (m_connList.at(i)->isHook(cur)) {
          if (!m_connList.at(i)->loopChain()->isSelected()) {
            //m_connList.at(i)->loopChain()->setSelected(true);
            setChainSelected(m_connList.at(i)->loopChain(), true);
            queue.enqueue(m_connList.at(i)->loopChain());
          }
        } else if (m_connList.at(i)->isLoop(cur)) {
          if (!m_connList.at(i)->hookChain()->isSelected()) {
            //m_connList.at(i)->hookChain()->setSelected(true);
            setChainSelected(m_connList.at(i)->hookChain(), true);
            queue.enqueue(m_connList.at(i)->hookChain());
          }
        }
      }
    }
  }
}

int ZStackDoc::pickLocsegChainId(int x, int y, int z) const
{
  if (m_traceWorkspace == NULL) {
    return -1;
  }

  if (m_traceWorkspace->trace_mask == NULL) {
    return -1;
  }

  int id = -1;

  if (IS_IN_CLOSE_RANGE(x, 0, m_traceWorkspace->trace_mask->width - 1) &&
      IS_IN_CLOSE_RANGE(y, 0, m_traceWorkspace->trace_mask->height - 1)) {
    if (z >= 0) {
      id = ((int) Get_Stack_Pixel(m_traceWorkspace->trace_mask, x, y, z, 0)) - 1;
    } else {
      id = ((int) Stack_Hittest_Z(m_traceWorkspace->trace_mask, x, y)) - 1;
    }
  }

  return id;
}

int ZStackDoc::pickPunctaIndex(int x, int y, int z) const
{
  int index = -1;
  for (int i=0; i<m_punctaList.size(); i++) {
    if (z >= 0) {
      if (IS_IN_CLOSE_RANGE3(x, y, z,
                             m_punctaList[i]->x() - m_punctaList[i]->radius(),
                             m_punctaList[i]->x() + m_punctaList[i]->radius(),
                             m_punctaList[i]->y() - m_punctaList[i]->radius(),
                             m_punctaList[i]->y() + m_punctaList[i]->radius(),
                             iround(m_punctaList[i]->z()),
                             iround(m_punctaList[i]->z()))) {
        index = i;
        break;
      }
    } else {
      if (IS_IN_CLOSE_RANGE3(x, y, z,
                             m_punctaList[i]->x() - m_punctaList[i]->radius(),
                             m_punctaList[i]->x() + m_punctaList[i]->radius(),
                             m_punctaList[i]->y() - m_punctaList[i]->radius(),
                             m_punctaList[i]->y() + m_punctaList[i]->radius(),
                             -1, -1)) {
        index = i;
        break;
      }
    }
  }
  return index;
}

bool ZStackDoc::selectPuncta(int index)
{
  if (index < m_punctaList.size() && index >= 0) {
    setPunctumSelected(m_punctaList[index], true);
    return true;
  }
  return false;
}

bool ZStackDoc::deleteAllPuncta()
{
  int objstartpos = m_objs.size()-1;
  bool result = true;
  int drawablestartpos = m_drawableList.size()-1;
  while (!m_punctaList.isEmpty()) {
    bool match = false;
    ZDocumentable *obj = m_punctaList.takeLast();
    for (int i=objstartpos; i>=0; i--) {
      if ((ZInterface*) m_objs.at(i) == (ZInterface*) obj) {
        m_objs.removeAt(i);
        match = true;
        objstartpos = i-1;
        break;
      }
    }
    if (!match) {
      std::cout << "error remove puncta objs!" << std::endl;
      result = false;
    }
    match = false;
    for (int i=drawablestartpos; i>=0; i--) {
      if ((ZInterface*) m_drawableList.at(i) == (ZInterface*) obj) {
        m_drawableList.removeAt(i);
        match = true;
        drawablestartpos = i-1;
        break;
      }
    }
    if (!match) {
      std::cout << "error remove puncta drawable!" << std::endl;
      result = false;
    }
  }
  return result;
}

bool ZStackDoc::expandSelectedPuncta()
{
  QMutableListIterator<ZPunctum*> iter(m_punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      iter.value()->setRadius(iter.value()->radius() + 1);
    }
  }
  return true;
}

bool ZStackDoc::shrinkSelectedPuncta()
{
  QMutableListIterator<ZPunctum*> iter(m_punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      if (iter.value()->radius() > 1) {
        iter.value()->setRadius(iter.value()->radius() - 1);
      }
    }
  }
  return true;
}

bool ZStackDoc::meanshiftSelectedPuncta()
{
  if (stack()->isVirtual()) {
    return false;
  }
  QMutableListIterator<ZPunctum*> iter(m_punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      Geo3d_Ball *gb = New_Geo3d_Ball();
      gb->center[0] = iter.value()->x();
      gb->center[1] = iter.value()->y();
      gb->center[2] = iter.value()->z();
      gb->r = iter.value()->radius();
      Geo3d_Ball_Mean_Shift(gb, stack()->c_stack(), 1, 0.5);
      iter.value()->setX(gb->center[0]);
      iter.value()->setY(gb->center[1]);
      iter.value()->setZ(gb->center[2]);
      Delete_Geo3d_Ball(gb);
    }
  }
  return true;
}

bool ZStackDoc::meanshiftAllPuncta()
{
  if (stack()->isVirtual()) {
    return false;
  }
  for (int i=0; i<m_punctaList.size(); i++) {
    Geo3d_Ball *gb = New_Geo3d_Ball();
    gb->center[0] = m_punctaList[i]->x();
    gb->center[1] = m_punctaList[i]->y();
    gb->center[2] = m_punctaList[i]->z();
    gb->r = m_punctaList[i]->radius();
    Geo3d_Ball_Mean_Shift(gb, stack()->c_stack(), 1, 0.5);
    m_punctaList[i]->setX(gb->center[0]);
    m_punctaList[i]->setY(gb->center[1]);
    m_punctaList[i]->setZ(gb->center[2]);
    Delete_Geo3d_Ball(gb);
  }
  return true;
}

void ZStackDoc::holdClosestSeg(int id, int x, int y, int z)
{
  ZLocsegChain *chain;
  foreach (chain, m_chainList) {
    if (chain->id() == id) {
      chain->holdClosestSeg(x, y, z);
      break;
    }
  }
}

int ZStackDoc::selectLocsegChain(int id, int x, int y, int z, bool showProfile)
{
  int found = 0;

  ZLocsegChain *chain;
  foreach (chain, m_chainList) {
    if (chain->id() == id) {
      //chain->setSelected(true);

      m_masterChain = chain;
      found = -1;

      if (x > 0) {
        found = iround(chain->holdClosestSeg(x, y, z)) + 1;

#if defined _ADVANCED_2
        Local_Neuroseg *locseg = chain->heldNeuroseg();

        if (showProfile == true) {
          if (locseg != NULL) {
            Stack *profile_stack = Local_Neuroseg_Stack(locseg, stack()->c_stack());
            ZStackFrame *frame = new ZStackFrame(NULL);
            ZEllipse *ellipse =
                new ZEllipse(QPointF(profile_stack->width / 2,
                                     profile_stack->height / 2),
                             Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM),
                             Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM));

            frame->addDecoration(ellipse);
            frame->loadStack(profile_stack, true);

            emit frameDelivered(frame);
          }
        }
#else
        UNUSED_PARAMETER(showProfile);
#endif
      }

      emit locsegChainSelected(chain);
      setChainSelected(chain, true);
    } else {
      //chain->setSelected(false);
      setChainSelected(chain, false);
    }
  }

  return found;
}

bool ZStackDoc::selectSwcTreeBranch(int x, int y, int z)
{
  if (!m_swcList.isEmpty()) {
    return m_swcList.at(0)->labelBranch(x, y, z, 5.0);
  }

  return false;
}

void ZStackDoc::removeLastObject(bool deleteObject)
{
  if (!m_objs.isEmpty()) {
    ZDocumentable *obj = m_objs.takeLast();

    if (!m_chainList.isEmpty()) {
      if ((ZInterface*) m_chainList.last() == (ZInterface*) obj) {
        removeLocsegChain((ZInterface*) obj);
      }
    }

    if (!m_swcList.isEmpty()) {
      if ((ZInterface*) m_swcList.last() == (ZInterface*) obj) {
        m_swcList.removeLast();
      }
    }

    if (!m_punctaList.isEmpty()) {
      if ((ZInterface*) m_punctaList.last() == (ZInterface*) obj) {
        m_punctaList.removeLast();
      }
    }

    if (!m_obj3dList.isEmpty()) {
      if ((ZInterface*) m_obj3dList.last() == (ZInterface*) obj) {
        m_obj3dList.removeLast();
      }
    }

    if (!m_connList.isEmpty()) {
      if ((ZInterface*) m_connList.last() == (ZInterface*) obj) {
        m_connList.removeLast();
      }
    }

    if (!m_strokeList.isEmpty()) {
      if ((ZInterface*) m_strokeList.last() == (ZInterface*) obj) {
        m_strokeList.removeLast();
      }
    }

    if (!m_drawableList.isEmpty()) {
      if ((ZInterface*) m_drawableList.last() == (ZInterface*) obj) {
        m_drawableList.removeLast();
      }
    }

    if (deleteObject == true) {
      delete obj;
    }
  }
}

void ZStackDoc::removeAllObject(bool deleteObject)
{
  while (!m_objs.isEmpty()) {
    removeLastObject(deleteObject);
  }
}

#define REMOVE_OBJECT(list, obj)				\
  for (int i = 0; i < list.size(); i++) {			\
    if ((ZInterface*) list.at(i) == obj) {		\
      list.removeAt(i);					\
      break;					\
    }						\
  }

void ZStackDoc::removeLocsegChain(ZInterface *obj)
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if ((ZInterface*) m_chainList.at(i) == obj) {
      ZLocsegChain *chain = m_chainList.takeAt(i);
      chain->eraseTraceMask(m_traceWorkspace->trace_mask);
      if (m_masterChain == chain) {
        m_masterChain = NULL;
      }

      QMutableListIterator<ZLocsegChainConn*> connIter(m_connList);
      connIter.toFront();
      while (connIter.hasNext()) {
        ZLocsegChainConn *conn = connIter.next();
        if (conn->isLoop(chain) || conn->isHook(chain)) {
          //REMOVE_OBJECT(m_drawableList, ((ZInterface*) conn))
          connIter.remove();
          removeObject(conn, true);
          //delete conn;
        }
      }

      notifyChainModified();
      break;
    }
  }
}

void ZStackDoc::removeSmallLocsegChain(double thre)
{
  QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *chain = chainIter.next();
    if (chain->geoLength() < thre) {
      removeObject(chain, true);
    }
  }

  notifyChainModified();
}

void ZStackDoc::removeAllLocsegChain()
{
  QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *chain = chainIter.next();
    removeObject(chain, true);
  }

  notifyChainModified();
}

void ZStackDoc::removeObject(ZInterface *obj, bool deleteObject)
{
  REMOVE_OBJECT(m_objs, obj);
  //REMOVE_OBJECT(m_swcObjects, obj);
  //REMOVE_OBJECT(m_vrmlObjects, obj);
  REMOVE_OBJECT(m_swcList, obj);
  REMOVE_OBJECT(m_connList, obj);
  REMOVE_OBJECT(m_obj3dList, obj);
  REMOVE_OBJECT(m_drawableList, obj);
  REMOVE_OBJECT(m_punctaList, obj);
  REMOVE_OBJECT(m_strokeList, obj);

  removeLocsegChain(obj);

  if (deleteObject == true) {
    delete obj;
  }
}

std::set<ZSwcTree *> ZStackDoc::removeEmptySwcTree(bool deleteObject)
{
  std::set<ZSwcTree *> emptyTreeSet;
  QMutableListIterator<ZSwcTree*> swcIter(m_swcList);
  while (swcIter.hasNext()) {
    ZSwcTree *tree = swcIter.next();
    if (!tree->hasRegularNode()) {
      swcIter.remove();
      removeObject(tree, deleteObject);
      if (!deleteObject) {
        emptyTreeSet.insert(tree);
      }
    }
  }
  notifySwcModified();

  return emptyTreeSet;
}

void ZStackDoc::removeAllSwcTree(bool deleteObject)
{
  QMutableListIterator<ZSwcTree*> swcIter(m_swcList);
  while (swcIter.hasNext()) {
    ZSwcTree *tree = swcIter.next();
    removeObject(tree, deleteObject);
  }
  notifySwcModified();
}

#define REMOVE_SELECTED_OBJECT(objtype, list, iter)	\
  QMutableListIterator<objtype*> iter(list);	\
  while (iter.hasNext()) {	\
    if (iter.next()->isSelected()) {	\
      iter.remove();	\
    }	\
  }

void ZStackDoc::removeSelectedObject(bool deleteObject)
{
  //REMOVE_SELECTED_OBJECT(ZSwcExportable, m_swcObjects, swceIter);
  //REMOVE_SELECTED_OBJECT(ZVrmlExportable, m_vrmlObjects, vrmlIter);
  REMOVE_SELECTED_OBJECT(ZSwcTree, m_swcList, swcIter);
  REMOVE_SELECTED_OBJECT(ZObject3d, m_obj3dList, obj3dIter);
  REMOVE_SELECTED_OBJECT(ZLocsegChainConn, m_connList, connIter);
  REMOVE_SELECTED_OBJECT(ZStackDrawable, m_drawableList, drawableIter);
  REMOVE_SELECTED_OBJECT(ZPunctum, m_punctaList, punctaIter);
  REMOVE_SELECTED_OBJECT(ZStroke2d, m_strokeList, strokeIter);

  QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *obj = chainIter.next();
    if (obj->isSelected()) {
      if (obj == m_masterChain) {
        m_masterChain = NULL;
      }
      obj->eraseTraceMask(m_traceWorkspace->trace_mask);
      connIter.toFront();
      while (connIter.hasNext()) {
        ZLocsegChainConn *conn = connIter.next();
        if (conn->isLoop(obj) || conn->isHook(obj)) {
          REMOVE_OBJECT(m_drawableList, ((ZInterface*) conn))
          connIter.remove();
        }
      }
      chainIter.remove();
    }
  }


  QMutableListIterator<ZDocumentable*> docIter(m_objs);
  while (docIter.hasNext()) {
    ZDocumentable *obj = docIter.next();
    if (obj->isSelected()) {
      docIter.remove();
      if (deleteObject == true) {
        delete obj;
      }
    }
  }
}

void ZStackDoc::removeSelectedPuncta(bool deleteObject)
{
  REMOVE_SELECTED_OBJECT(ZPunctum, m_punctaList, punctaIter);

  QMutableListIterator<ZStackDrawable*> drawableIter(m_drawableList);
  while (drawableIter.hasNext()) {
    ZStackDrawable *obj = drawableIter.next();

    if (obj->isSelected()) {
      if (obj->className() == "ZPunctum") {
        drawableIter.remove();
      }
    }
  }

  QMutableListIterator<ZDocumentable*> docIter(m_objs);
  while (docIter.hasNext()) {
    ZDocumentable *obj = docIter.next();

    if (obj->isSelected()) {
      if (obj->className() == "ZPunctum") {
        docIter.remove();
        if (deleteObject == true) {
          delete obj;
        }
      }
    }
  }

  notifyPunctumModified();
}

bool ZStackDoc::pushLocsegChain(ZInterface *obj)
{
  bool found =false;

  ZLocsegChain *chain = NULL;
  foreach (chain, m_chainList) {
    if ((ZInterface*) chain == obj) {
      found = true;
      break;
    }
  }

  if (found) {
    ZLocsegChain *newChain = NULL;
    if (m_traceWorkspace == NULL) {
      newChain = chain->pushHeldNode(stack()->c_stack());
    } else {
      chain->eraseTraceMask(m_traceWorkspace->trace_mask);
      newChain = chain->pushHeldNode(stack()->c_stack(),
				    m_traceWorkspace->trace_mask);
    }
    if (newChain != NULL) {
      chain->merge(newChain);
      if (m_traceWorkspace != NULL) {
        chain->labelTraceMask(m_traceWorkspace->trace_mask);
      }
      delete newChain;
    }
  }

  return found;
}

void ZStackDoc::pushSelectedLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if (m_chainList.at(i)->isSelected()) {
      pushLocsegChain((ZInterface*) (m_chainList.at(i)));
    }
  }
}

bool ZStackDoc::fixLocsegChainTerminal(ZInterface *obj)
{
  bool found =false;

  ZLocsegChain *chain = NULL;
  foreach (chain, m_chainList) {
    if ((ZInterface*) chain == obj) {
      found = true;
      break;
    }
  }

  if (found) {
    if (m_traceWorkspace != NULL) {
      chain->eraseTraceMask(m_traceWorkspace->trace_mask);
      chain->fixTerminal(stack()->c_stack(), m_traceWorkspace);
      chain->labelTraceMask(m_traceWorkspace->trace_mask);
    }

    updateLocsegChain(chain);
  }

  return found;
}

void ZStackDoc::setPunctumSelected(ZPunctum *punctum, bool select)
{
  if (punctum->isSelected() != select) {
    punctum->setSelected(select);
    QList<ZPunctum*> selected;
    QList<ZPunctum*> deselected;
    if (select) {
      m_selectedPuncta.insert(punctum);
      selected.push_back(punctum);
    } else {
      m_selectedPuncta.erase(punctum);
      deselected.push_back(punctum);
    }
    emit punctaSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::deselectAllPuncta()
{
  QList<ZPunctum*> selected;
  QList<ZPunctum*> deselected;
  m_selectedPuncta.clear();
  for (int i=0; i<m_punctaList.size(); i++) {
    if (m_punctaList[i]->isSelected()) {
      m_punctaList[i]->setSelected(false);
      deselected.push_back(m_punctaList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit punctaSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::setChainSelected(ZLocsegChain *chain, bool select)
{
  if (chain->isSelected() != select) {
    QList<ZLocsegChain*> selected;
    QList<ZLocsegChain*> deselected;
    chain->setSelected(select);
    if (select) {
      m_selectedChains.insert(chain);
      selected.push_back(chain);
    } else {
      m_selectedChains.erase(chain);
      deselected.push_back(chain);
    }
    emit chainSelectionChanged(selected, deselected);
  } else {
    emit holdSegChanged();
  }

}

void ZStackDoc::setChainSelected(const std::vector<ZLocsegChain *> &chains, bool select)
{
  QList<ZLocsegChain*> selected;
  QList<ZLocsegChain*> deselected;
  for (size_t i=0; i<chains.size(); ++i) {
    ZLocsegChain *chain = chains[i];
    if (chain->isSelected() != select) {
      chain->setSelected(select);
      if (select) {
        m_selectedChains.insert(chain);
        selected.push_back(chain);
      } else {
        m_selectedChains.erase(chain);
        deselected.push_back(chain);
      }
    }
  }
  if (!selected.empty() || !deselected.empty())
    emit chainSelectionChanged(selected, deselected);
}

void ZStackDoc::deselectAllChains()
{
  QList<ZLocsegChain*> selected;
  QList<ZLocsegChain*> deselected;
  m_selectedChains.clear();
  for (int i=0; i<m_chainList.size(); i++) {
    if (m_chainList[i]->isSelected()) {
      m_chainList[i]->setSelected(false);
      deselected.push_back(m_chainList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit chainSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::setSwcSelected(ZSwcTree *tree, bool select)
{
  if (tree->isSelected() != select) {
    tree->setSelected(select);
    QList<ZSwcTree*> selected;
    QList<ZSwcTree*> deselected;
    if (select) {
      m_selectedSwcs.insert(tree);
      selected.push_back(tree);
      // deselect its nodes
      std::vector<Swc_Tree_Node *> tns;
      for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin();
           it != m_selectedSwcTreeNodes.end(); ++it) {
        if (tree == nodeToSwcTree(*it))
          tns.push_back(*it);
      }
      setSwcTreeNodeSelected(tns.begin(), tns.end(), false);
    } else {
      m_selectedSwcs.erase(tree);
      deselected.push_back(tree);
    }
    emit swcSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::deselectAllSwcs()
{
  QList<ZSwcTree*> selected;
  QList<ZSwcTree*> deselected;
  m_selectedSwcs.clear();
  for (int i=0; i<m_swcList.size(); i++) {
    if (m_swcList[i]->isSelected()) {
      m_swcList[i]->setSelected(false);
      deselected.push_back(m_swcList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit swcSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::setSwcTreeNodeSelected(Swc_Tree_Node *tn, bool select)
{
  if (SwcTreeNode::isRegular(tn)) {
    QList<Swc_Tree_Node*> selected;
    QList<Swc_Tree_Node*> deselected;
    if (select) {
      if ((m_selectedSwcTreeNodes.insert(tn)).second) {
        selected.push_back(tn);
        // deselect its tree
        setSwcSelected(nodeToSwcTree(tn), false);
      }
    } else {
      if (m_selectedSwcTreeNodes.erase(tn) > 0) {
        deselected.push_back(tn);
      }
    }

    if (selected.size() > 0 || deselected.size() > 0) {
      emit swcTreeNodeSelectionChanged(selected, deselected);
    }
  }
}

void ZStackDoc::deselectAllSwcTreeNodes()
{
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;
  for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin(); it != m_selectedSwcTreeNodes.end(); it++) {
    deselected.push_back(*it);
  }
  m_selectedSwcTreeNodes.clear();
  if (deselected.size() > 0)
    emit swcTreeNodeSelectionChanged(selected, deselected);
}

void ZStackDoc::deselectAllObject()
{
  deselectAllPuncta();
  deselectAllChains();
  deselectAllSwcs();
  deselectAllSwcTreeNodes();
  QMutableListIterator<ZDocumentable*> iter0(m_objs);
  while (iter0.hasNext()) {
    ZDocumentable *obj = iter0.next();
    if (obj->isSelected()) {
      obj->setSelected(false);
    }
  }
}

void ZStackDoc::setPunctumVisible(ZPunctum *punctum, bool visible)
{
  if (punctum->isVisible() != visible) {
    punctum->setVisible(visible);
    emit punctumVisibleStateChanged(punctum, visible);
  }
}

void ZStackDoc::setChainVisible(ZLocsegChain *chain, bool visible)
{
  if (chain->isVisible() != visible) {
    chain->setVisible(visible);
    emit chainVisibleStateChanged(chain, visible);
  }
}

void ZStackDoc::setSwcVisible(ZSwcTree *tree, bool visible)
{
  if (tree->isVisible() != visible) {
    tree->setVisible(visible);
    emit swcVisibleStateChanged(tree, visible);
  }
}

QString ZStackDoc::toString()
{
  return QString("Number of chains: %1").arg(m_chainList.size());
}

QStringList ZStackDoc::toStringList() const
{
  ZStack *mainStack = stack();

  QStringList list;
  list.append(QString("Number of objects: %1").arg(m_objs.size()));
  list.append(QString("Number of chains: %1").arg(m_chainList.size()));
  if (mainStack != NULL) {
    list.append(QString("Stack size: %1 x %2 x %3").arg(mainStack->width())
                .arg(mainStack->height()).arg(mainStack->depth()));
    list.append(QString("Stack offset: ") +
                mainStack->getOffset().toString().c_str());
  }

  if (m_chainList.size() > 0) {
    for (int i = 0; i < m_chainList.size(); i++) {
      ZLocsegChain *chain = m_chainList.at(i);
      if (chain->isSelected()) {
        ZStack *mainStack = stack();
        if (mainStack != NULL) {
          list += chain->toStringList(mainStack->c_stack());
        }
        break;
      }
    }
  }

  return list;
}

ZCurve ZStackDoc::locsegProfileCurve(int option) const
{
  ZCurve curve;

  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    if (Stack_Channel_Number(mainStack->c_stack()) == 1) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->isSelected()) {
          if (chain->heldNode() >= 0) {
            int nsample = 11;
            double *profile = Local_Neuroseg_Height_Profile(
                  chain->heldNeuroseg(), mainStack->c_stack(), chain->zScale(),
                  nsample, option, NULL, NULL);
            curve.loadArray(profile, nsample);
            free(profile);
          }
          break;
        }
      }
    }
  }

  return curve;
}

void ZStackDoc::addDocumentable(ZDocumentable *obj)
{
  m_objs.append(obj);
}

void ZStackDoc::appendSwcNetwork(ZSwcNetwork &network)
{
  if (m_swcNetwork == NULL) {
    m_swcNetwork = new ZSwcNetwork;
  }
  for (size_t i = 0; i < network.treeNumber(); i++) {
    addSwcTree(network.getTree(i));
  }

  m_swcNetwork->merge(network);

  ZStack *mainStack = stack();
  if (mainStack == NULL) {
    Stack *stack = new Stack;
    double corner[6];
    m_swcNetwork->boundBox(corner);
    static const double Lateral_Margin = 10.0;
    static const double Axial_Margin = 1.0;
    Stack_Set_Attribute(stack, round(corner[3] + Lateral_Margin - corner[0] + 1),
        round(corner[4] + Lateral_Margin - corner[1] + 1),
        round(corner[5] + Axial_Margin - corner[2] + 1),
        GREY);

    stack->array = NULL;
    loadStack(stack, true);
    setStackSource("swc network");
  }

  emit swcNetworkModified();
}

void ZStackDoc::setTraceMinScore(double score)
{
  m_traceWorkspace->min_score = score;
}

void ZStackDoc::setReceptor(int option, bool cone)
{
  ((Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace)->sws->field_func =
      Neuroseg_Slice_Field_Func(option);

  if (cone == TRUE) {
    Locseg_Fit_Workspace_Enable_Cone(
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace);
  } else {
    Locseg_Fit_Workspace_Disable_Cone(
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace);
  }
}

void ZStackDoc::updateMasterLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    ZLocsegChain *chain = m_chainList.at(i);
    if (chain->isSelected()) {
      m_masterChain = chain;
      break;
    }
  }
}

ZInterface* ZStackDoc::bringChainToFront()
{
  ZLocsegChain *chain = NULL;

  for (int i = 0; i < m_chainList.size(); i++) {
    chain = m_chainList.at(i);
    if (chain->isSelected()) {
      if (i > 0) {
        m_chainList.move(i, 0);
        chain->labelTraceMask(m_traceWorkspace->trace_mask, -1);
        int j;
        for (j = 0; j < m_objs.size(); j++) {
          if ((ZInterface*) m_objs.at(j) == (ZInterface*) chain) {
            m_objs.move(j, 0);
            break;
          }
        }

        /*
        for (j = 0; j < m_swcObjects.size(); j++) {
          if ((ZInterface*) m_swcObjects.at(j) == (ZInterface*) chain) {
            m_swcObjects.move(j, 0);
            break;
          }
        }

        for (j = 0; j < m_vrmlObjects.size(); j++) {
          if ((ZInterface*) m_vrmlObjects.at(j) == (ZInterface*) chain) {
            m_vrmlObjects.move(j, 0);
            break;
          }
        }
*/

        for (j = 0; j < m_drawableList.size(); j++) {
          if ((ZInterface*) m_drawableList.at(j) == (ZInterface*) chain) {
            m_drawableList.move(j, 0);
            break;
          }
        }
      }
      break;
    }
  }

  return (ZInterface*) chain;
}

ZInterface* ZStackDoc::sendChainToBack()
{
  ZLocsegChain *chain = NULL;

  for (int i = 0; i < m_chainList.size(); i++) {
    chain = m_chainList.at(i);
    if (chain->isSelected()) {
      if (i < m_chainList.size() - 1) {
        int j;
        for (j = i + 1; j < m_chainList.size(); j++) {
          m_chainList.at(j)->labelTraceMask(m_traceWorkspace->trace_mask,
                                            chain->id() + 1);
        }

        m_chainList.move(i, m_chainList.size() - 1);

        for (j = 0; j < m_objs.size(); j++) {
          if ((ZInterface*) m_objs.at(j) == (ZInterface*) chain) {
            m_objs.move(j, m_objs.size() - 1);
            break;
          }
        }

        /*
        for (j = 0; j < m_swcObjects.size(); j++) {
          if ((ZInterface*) m_swcObjects.at(j) == (ZInterface*) chain) {
            m_swcObjects.move(j, m_swcObjects.size() - 1);
            break;
          }
        }

        for (j = 0; j < m_vrmlObjects.size(); j++) {
          if ((ZInterface*) m_vrmlObjects.at(j) == (ZInterface*) chain) {
            m_vrmlObjects.move(j, m_vrmlObjects.size() - 1);
            break;
          }
        }
*/

        for (j = 0; j < m_drawableList.size(); j++) {
          if ((ZInterface*) m_drawableList.at(j) == (ZInterface*) chain) {
            m_drawableList.move(j, m_drawableList.size() - 1);
            break;
          }
        }
      }
      break;
    }
  }

  return chain;
}

bool ZStackDoc::linkChain(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          //if (chain->isSelected()) {
          addLocsegChain(m_masterChain->bridge(chain));
          if (m_masterChain->isIgnorable()) {
            removeObject(m_masterChain, true);
          }
          if (chain->isIgnorable()) {
            removeObject(chain, true);
          }
          emit chainModified();
          return true;
          //}
        }
      }
    }
  }

  return false;
}

bool ZStackDoc::hookChain(int id, int option)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          //if (chain->isSelected()) {
          ZLocsegChain *newchain = NULL;
          switch (option) {
          case 0:
            newchain = m_masterChain->bridge(chain, false);
            break;
          default:
            {
              Locseg_Fit_Workspace *ws =
                  (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
              newchain = m_masterChain->spBridge(chain, stack()->c_stack(), ws);
            }
          }
          if (m_masterChain->isIgnorable()) {
            removeObject(m_masterChain, true);
          }

          if (chain->isIgnorable()) {
            removeObject(chain, true);
          }

          if (newchain != NULL) {
            addLocsegChain(newchain);
          } else {
            return false;
          }
          emit chainModified();
          return true;
        }
      }
    }
  }

  return false;
}

bool ZStackDoc::mergeChain(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);

        if (chain->id() == id) {
          removeObject((ZInterface*) chain, false);
          m_masterChain->merge(chain);
          chain->detachData();
          delete chain;
          updateLocsegChain(m_masterChain);

          emit chainModified();
          return true;
        }
      }
    }
  }

  return false;
}

bool ZStackDoc::connectChain(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          addLocsegChainConn(m_masterChain, chain);
          return true;
        }
      }
    }
  }

  return false;
}

bool ZStackDoc::disconnectChain(int id)
{
  if (m_masterChain != NULL) {
    ZLocsegChain *slaveChain = NULL;

    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          slaveChain = chain;
          break;
        }
      }
      if (slaveChain != NULL) {
        for (int i = 0; i < m_connList.size(); i++) {
          ZLocsegChainConn *conn = m_connList.at(i);
          if (conn->has(m_masterChain, slaveChain)) {
            removeObject(conn, true);
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool ZStackDoc::chainShortestPath(int id)
{
  bool succ = false;

  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          //addLocsegChainConn(m_masterChain, chain);
          Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
          sgw->conn = 26;
          sgw->wf = Stack_Voxel_Weight_S;
          sgw->resolution[0] = m_traceWorkspace->resolution[0];// m_parent->xResolution();
          sgw->resolution[1] = m_traceWorkspace->resolution[1];//m_parent->yResolution();
          sgw->resolution[2] = m_traceWorkspace->resolution[2];//m_parent->zResolution();
          /*
          double inner = Locseg_Chain_Average_Score(m_masterChain->data(),
                                                    stack()->stack(), 1.0,
                                                    STACK_FIT_MEAN_SIGNAL);
          double outer = Locseg_Chain_Average_Score(m_masterChain->data(),
                                                    stack()->stack(), 1.0,
                                                    STACK_FIT_OUTER_SIGNAL);
*/
          //sgw->argv[3] = inner * 0.1 + outer * 0.9;
          //sgw->argv[4] = (inner - outer) / 4.6 * 1.8;
          //sgw->argv[4] = 2.0;
          Int_Arraylist *path =
              Locseg_Chain_Shortest_Path(m_masterChain->data(), chain->data(),
                                         stack()->c_stack(), 1.0, sgw);

          if (path != NULL) {
            Object_3d *obj = Make_Object_3d(path->length, 0);
            for (int i = 0; i < path->length; i++) {
              Stack_Util_Coord(path->array[i], stack()->width(),
                               stack()->height(),
                               obj->voxels[i], obj->voxels[i] + 1,
                               obj->voxels[i] + 2);
            }
            addObj3d(new ZObject3d(obj));
            Kill_Int_Arraylist(path);
            succ = true;
          }

          Kill_Stack_Graph_Workspace(sgw);
        }
      }
    }
  }

  return succ;
}

void ZStackDoc::chainConnInfo(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      double feats[100];
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          double res[3];
          res[0] = m_traceWorkspace->resolution[0];//m_parent->xResolution();
          res[1] = m_traceWorkspace->resolution[1];//m_parent->yResolution();
          res[2] = m_traceWorkspace->resolution[2];//m_parent->zResolution();
          int n;
          Locseg_Chain_Conn_Feature(m_masterChain->data(),
                                                    chain->data(),
                                                    stack()->c_stack(),
                                                    res, feats, &n);
          QString info;
          info.append(QString("Planar distance: %1\n").arg(feats[1]));
          info.append(QString("Euclidean distance: %1\n").arg(feats[8]));
          info.append(QString("Geodesic distance: %1\n").arg(feats[9]));
          info.append(QString("Maxmin distance: %1\n").arg(feats[10]));

          QMessageBox::information(NULL, QString("test"), info);
        }
      }
    }
  }
}

void ZStackDoc::extendChain(double x, double y, double z)
{
  if (m_masterChain != NULL) {
    m_masterChain->extendHeldEnd(x, y, z);
    updateLocsegChain(m_masterChain);
  }
}

void ZStackDoc::addLocsegChainConn(ZLocsegChain *hook, ZLocsegChain *loop)
{
  Connection_Test_Workspace *ctw = m_connectionTestWorkspace;

  Neurocomp_Conn nc_conn;
  Locseg_Chain_Connection_Test(hook->data(), loop->data(), NULL,
  		1.0, &nc_conn, ctw);
  Neurocomp_Conn_Translate_Mode(loop->length(), &nc_conn);

  ZLocsegChainConn *conn = new ZLocsegChainConn(hook, loop,
                                                nc_conn.info[0],
                                                nc_conn.info[1], nc_conn.mode);

  addLocsegChainConn(conn);

  if (nc_conn.mode == NEUROCOMP_CONN_LINK) {
    int hook_spot, loop_spot;
    hook_spot = nc_conn.info[1];
    loop_spot = nc_conn.info[0];
    conn = new ZLocsegChainConn(loop, hook, hook_spot, loop_spot,
                                nc_conn.mode);

    addLocsegChainConn(conn);
  }
}

void ZStackDoc::addLocsegChainConn(ZLocsegChain *hook, ZLocsegChain *loop,
                                   int hookSpot, int loopSpot, int mode)
{
  ZLocsegChainConn *conn = NULL;

  if ((hookSpot >= 0) && (loopSpot >= 0)) {
    conn = new ZLocsegChainConn(hook, loop, hookSpot, loopSpot, mode);
  } else {
    Connection_Test_Workspace *ctw = m_connectionTestWorkspace;
    int old_value = ctw->hook_spot;
    ctw->hook_spot = hookSpot;
    Neurocomp_Conn nc_conn;
    Locseg_Chain_Connection_Test(hook->data(), loop->data(), NULL,
                                 1.0, &nc_conn, ctw);

    conn = new ZLocsegChainConn(hook, loop, nc_conn.info[0], nc_conn.info[1],
                                nc_conn.mode);
    ctw->hook_spot = old_value;
  }

  addLocsegChainConn(conn);
}

bool ZStackDoc::isMasterChainId(int id)
{
  if (m_masterChain->id() == id) {
    return true;
  } else {
    return false;
  }
}

void ZStackDoc::refineSelectedChainEnd()
{
  Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, m_traceWorkspace->trace_mask);
  foreach(ZLocsegChain *chain, m_chainList) {
    if (chain->isSelected()) {
      ZStack *mainStack = stack();
      if (mainStack != NULL) {
        chain->refineHeldEnd(mainStack->c_stack(), m_traceWorkspace);
        updateLocsegChain(chain);

        QMutableListIterator<ZLocsegChainConn*> connIter(m_connList);
        connIter.toFront();
        while (connIter.hasNext()) {
          ZLocsegChainConn *conn = connIter.next();
          if (conn->isLoop(chain) || conn->isHook(chain)) {
            connIter.remove();
            removeObject(conn, true);
          }
        }
      }
    }
  }
}

void ZStackDoc::refineLocsegChainEnd()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, m_traceWorkspace->trace_mask);
    foreach(ZLocsegChain *chain, m_chainList) {
      chain->refineEnd(DL_HEAD, mainStack->c_stack(), m_traceWorkspace);
      chain->refineEnd(DL_TAIL, mainStack->c_stack(), m_traceWorkspace);
      updateLocsegChain(chain);

      QMutableListIterator<ZLocsegChainConn*> connIter(m_connList);
      connIter.toFront();
      while (connIter.hasNext()) {
        ZLocsegChainConn *conn = connIter.next();
        if (conn->isLoop(chain) || conn->isHook(chain)) {
          connIter.remove();
          removeObject(conn, true);
        }
      }
    }
  }
}

void ZStackDoc::mergeAllChain()
{
  foreach(ZLocsegChain *chain, m_chainList) {
    chain->eraseTraceMask(m_traceWorkspace->trace_mask);
  }

  if (m_chainList.size() > 0) {
     int chain_number = m_chainList.size();

     /* alloc <chain_array> */
     Neuron_Component *chain_array =
       Make_Neuron_Component_Array(chain_number);

     int chain_number2 = 0;

     for (int i = 0; i < chain_number; i++) {
       if ((m_chainList.at(i)->length() > 0) &&
           !(m_chainList.at(i)->isIgnorable())) {
         Set_Neuron_Component(chain_array + chain_number2,
                              NEUROCOMP_TYPE_LOCSEG_CHAIN,
                              m_chainList.at(i)->data());
         chain_number2++;
       }
     }

     /* reconstruct neuron */
     Connection_Test_Workspace ctw = *m_connectionTestWorkspace;
     ctw.sp_test = FALSE;

     double zscale = m_chainList.at(0)->zScale();

     /* alloc <ns> */
     ZStack *mainStack = stack();
     Neuron_Structure *ns = NULL;
     if (mainStack != NULL) {
       ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,
                                          mainStack->c_stack(), zscale, &ctw);
     } else {
       ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,
                                          NULL, zscale, &ctw);
     }

     Process_Neuron_Structure(ns);

     if (m_connectionTestWorkspace->crossover_test) {
       Neuron_Structure_Crossover_Test(ns, zscale);
     }

     Neuron_Structure_To_Tree(ns);
     Neuron_Structure_Merge_Locseg_Chain(ns);
     free(ns->comp);
     ns->comp = NULL;
     /* free <ns> */
     Kill_Neuron_Structure(ns);

     QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
     while (chainIter.hasNext()) {
       ZLocsegChain *obj = chainIter.next();
       if (obj->isEmpty()) {
         //obj->setSelected(true);
         ////removeLocsegChain(obj);
         setChainSelected(obj, true);
       } else {
         obj->updateBufferChain();
         obj->labelTraceMask(m_traceWorkspace->trace_mask);
         //obj->setSelected(false);
         setChainSelected(obj, false);
       }
     }
     removeSelectedObject(true);
   }
}

QString ZStackDoc::dataInfo(int x, int y, int z) const
{
  QString info = QString("%1, %2").arg(x).arg(y);

  if (z < 0) {
    info += " (MIP): ";
  } else {
    info += QString(", %3: ").arg(z);
  }

  if (stack() != NULL) {
    if (!stack()->isVirtual()) {
      if (stack()->channelNumber() == 1) {
        info += QString("%4").arg(stack()->value(x, y, z));
      } else {
        info += QString("(");
        for (int i=0; i<stack()->channelNumber(); i++) {
          if (i==0) {
            info += QString("%1").arg(stack()->value(x, y, z, i));
          } else {
            info += QString(", %1").arg(stack()->value(x, y, z, i));
          }
        }
        info += QString(")");
      }
    }

    if (stackMask() != NULL) {
      info += " | Mask: ";
      if (stackMask()->channelNumber() == 1) {
        info += QString("%4").arg(stackMask()->value(x, y, z));
      } else {
        info += QString("(");
        for (int i=0; i<stackMask()->channelNumber(); i++) {
          if (i==0) {
            info += QString("%1").arg(stackMask()->value(x, y, z, i));
          } else {
            info += QString(", %1").arg(stackMask()->value(x, y, z, i));
          }
        }
        info += QString(")");
      }
    }
  }

  if (m_traceWorkspace != NULL) {
    if (m_traceWorkspace->trace_mask != NULL) {
      int id = pickLocsegChainId(x, y, z);

      if (id >= 0) {
        info += QString("; Chain ID: %1").arg(id);
      }
    }
  }

  return info;
}

void ZStackDoc::setWorkdir(const QString &filePath)
{
  setWorkdir(filePath.toLocal8Bit().constData());
}

void ZStackDoc::setWorkdir(const char *filePath)
{
  strcpy(m_traceWorkspace->save_path, filePath);
}

void ZStackDoc::setTubePrefix(const char *prefix)
{
  strcpy(m_traceWorkspace->save_prefix, prefix);
}

void ZStackDoc::setBadChainScreen(const char *screen)
{
  if (screen == NULL) {
    m_badChainScreen.clear();
  } else {
    m_badChainScreen = screen;
  }
}

void ZStackDoc::autoTrace()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    QDir workDir(m_traceWorkspace->save_path);

    if (!workDir.exists()) {
      workDir.mkdir(workDir.absolutePath());
    }

    m_progressReporter->start();

    m_progressReporter->advance(0.05);

    //removeAllObject(true);

    m_progressReporter->startSubprogress(0.1);
    int thre = autoThreshold();
    m_progressReporter->endSubprogress(0.1);

    /* alloc <mask> */
    Stack *mask = Copy_Stack(mainStack->c_stack());
    Stack_Threshold_Binarize(mask, thre);
    m_progressReporter->advance(0.05);

    Translate_Stack(mask, GREY, 1);
    m_progressReporter->advance(0.05);

    /* alloc <mask2> */
    Stack *mask2 = Stack_Majority_Filter_R(mask, NULL, 26, 4);
    m_progressReporter->advance(0.05);

    /* free <mask>, <mask2> => <mask> */
    Kill_Stack(mask);
    mask = mask2;

    double z_scale = 1.0;
    if (m_traceWorkspace->resolution[0] != m_traceWorkspace->resolution[2]) {
      z_scale = m_traceWorkspace->resolution[0] / m_traceWorkspace->resolution[2];
    }

    mask2 = mask;
    /* resample the stack for dist calc if z is different */
    if (z_scale != 1.0) {
      mask2 = Resample_Stack_Depth(mask, NULL, z_scale);
      Stack_Binarize(mask2);
    }

    /* alloc <dist> */
    Stack *dist = Stack_Bwdist_L_U16(mask2, NULL, 0);
    m_progressReporter->advance(0.05);

    if (mask != mask2) {
      Kill_Stack(mask2);
    }


    /* alloc <seeds> */
    Stack *seeds = Stack_Locmax_Region(dist, 26);
    m_progressReporter->advance(0.05);

    /* alloc <objs> */
    Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);
    m_progressReporter->advance(0.05);

    Zero_Stack(seeds);

    int objnum = 0;
    while (objs != NULL) {
      Object_3d *obj = objs->data;
      Voxel_t center;
      Object_3d_Central_Voxel(obj, center);
      Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
      objs = objs->next;
      objnum++;
    }
    m_progressReporter->advance(0.05);

    /* free <objs> */
    Kill_Object_3d_List(objs);

    /* alloc <list> */
    Voxel_List *list = Stack_To_Voxel_List(seeds);

    /* free <seeds> */
    Kill_Stack(seeds);

    /* alloc <pa> */
    Pixel_Array *pa = Voxel_List_Sampling(dist, list);

    /* free <dist> */
    Kill_Stack(dist);

    /* alloc <voxel_array> */
    Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);

    uint16 *pa_array = (uint16 *) pa->array;

    /* alloc <seed_field> */
    Geo3d_Scalar_Field *seed_field = Make_Geo3d_Scalar_Field(pa->size);
    seed_field->size = 0;
    int i;
    for (i = 0; i < pa->size; i++) {
      seed_field->points[seed_field->size][0] = voxel_array[i]->x;
      seed_field->points[seed_field->size][1] = voxel_array[i]->y;
      seed_field->points[seed_field->size][2] = z_scale * voxel_array[i]->z;
      seed_field->values[seed_field->size] = sqrt((double)pa_array[i]);
      seed_field->size++;
    }

    /* free <list> */
    Kill_Voxel_List(list);

    /* free <voxel_array> */
    free(voxel_array);

    /* free <pa> */
    Kill_Pixel_Array(pa);

    /* now the seeds are in <field> */
    /* <mask> => <seed_mask> */
    Stack *seed_mask = mask;
    Zero_Stack(seed_mask);
    Locseg_Fit_Workspace *fws =
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
    Stack_Fit_Score old_fs = fws->sws->fs;

    fws->sws->fs.n = 2;
    fws->sws->fs.options[0] = STACK_FIT_DOT;
    fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;

    /* alloc <locseg> */
    Local_Neuroseg *locseg = (Local_Neuroseg *)
        malloc(seed_field->size * sizeof(Local_Neuroseg));

    /* alloc <values> */
    double *values = darray_malloc(seed_field->size);

    m_progressReporter->advance(0.05);

    m_progressReporter->startSubprogress(0.3);
    /* fit segment on each seed */
    for (i = 0; i < seed_field->size; i++) {
      qDebug("-----------------------------> seed: %d / %d\n",
             i, seed_field->size);
      m_progressReporter->advance(1.0 / seed_field->size);

      int index = i;
      int x = iround(seed_field->points[index][0]);
      int y = iround(seed_field->points[index][1]);
      int z = iround(seed_field->points[index][2]);

      double width = seed_field->values[index];

      ssize_t seed_offset =
          Stack_Util_Offset(x, y, z, stack()->width(), stack()->height(),
                            stack()->depth());

      if (seed_offset < 0) {
        continue;
      }

      if (width < 3.0) {
        width += 0.5;
      }

      Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_DEFAULT_H,
                   TZ_PI_2, 0.0, 0.0, 0.0, 1.0 / z_scale);
      /*
    Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_HEIGHT,
                 0.0, 0.0, 0.0, 0.0, 1.0 / z_scale);
*/
      double cpos[3];
      cpos[0] = x;
      cpos[1] = y;
      cpos[2] = z;

      Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);

      if (seed_mask->array[seed_offset] > 0) {
        qDebug("labeled\n");
        values[i] = 0.0;
        continue;
      }

      Local_Neuroseg_Optimize_W(locseg + i, stack()->c_stack(), 1.0, 0, fws);

      values[i] = fws->sws->fs.scores[1];

      qDebug("%g\n", values[i]);

      /* label seed_mask */
      if (values[i] > LOCAL_NEUROSEG_MIN_CORRCOEF) {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 2, 1.0);
      } else {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 1, 1.0);
      }

      //addLocsegChain(new ZLocsegChain(Copy_Local_Neuroseg(locseg + i)));
    }
    m_progressReporter->endSubprogress(0.3);

    fws->sws->fs = old_fs;
    Kill_Stack(seed_mask);

    /* make trace mask */
    if (m_traceWorkspace->trace_mask == NULL) {
      m_traceWorkspace->trace_mask =
          Make_Stack(GREY16, mainStack->width(), mainStack->height(),
                     mainStack->depth());
    }
    Zero_Stack(m_traceWorkspace->trace_mask);


    /* trace all seeds */
    int nchain;
    Locseg_Chain **chain =
        Trace_Locseg_S(mainStack->c_stack(), 1.0, locseg, values, seed_field->size,
                       m_traceWorkspace, &nchain);

    m_progressReporter->advance(0.3);

    /* tune ends */
    Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
    ws->signal = stack()->c_stack();
    ws->sratio = 1.0;
    ws->sdiff = 0.0;
    ws->option = 6;

    /*
    for (i = 0; i < nchain; i++) {
      if(chain[i] != NULL) {
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);
      }
    }
    */

    Stack_Binarize(m_traceWorkspace->trace_mask);

    double old_step = m_traceWorkspace->trace_step;
    BOOL old_refit = m_traceWorkspace->refit;
    BOOL traceMasked = Trace_Workspace_Is_Masked(m_traceWorkspace);

    loadTraceMask(true);
    m_traceWorkspace->trace_step = 0.1;
    m_traceWorkspace->refit = FALSE;

    for (i = 0; i < nchain; i++) {
      if (chain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        Trace_Locseg(stack()->c_stack(), 1.0, chain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(chain[i]);

        Locseg_Chain_Tune_End(chain[i], stack()->c_stack(), 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(chain[i], stack()->c_stack(), 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(chain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(chain[i]) - 1, ws);
        }
      }
    }

    ws->signal = NULL;
    Kill_Locseg_Label_Workspace(ws);

    m_traceWorkspace->trace_step = old_step;
    m_traceWorkspace->refit = old_refit;
    loadTraceMask(traceMasked);

    Zero_Stack(m_traceWorkspace->trace_mask);
    m_traceWorkspace->chain_id = 0;

    /* add chains */
    for (i = 0; i < nchain; i++) {
      if (chain[i] != NULL) {
        if (Locseg_Chain_Length(chain[i]) > 0) {
          ZLocsegChain *zchain = new ZLocsegChain(chain[i]);
          if (zchain->confidence(stack()->c_stack()) > 0.5) {
            addLocsegChain(zchain);
          } else {
            delete zchain;
          }
        }
      }
    }
    m_progressReporter->advance(0.05);

    free(chain);

    /* free <seed_field> */
    Kill_Geo3d_Scalar_Field(seed_field);

    /* free <values> */
    free(values);

    /* free <locseg> */
    free(locseg);

    m_progressReporter->end();
  }
}

void ZStackDoc::eraseTraceMask(const ZLocsegChain *chain)
{
  chain->eraseTraceMask(m_traceWorkspace->trace_mask);
}

void ZStackDoc::autoTrace(Stack *stack)
{
  int thre = autoThreshold(stack);
  /* alloc <mask> */
  Stack *mask = Copy_Stack(stack);
  Stack_Threshold_Binarize(mask, thre);
  Translate_Stack(mask, GREY, 1);

  /* alloc <mask2> */
  //Stack *mask2 = Stack_Majority_Filter_R(mask, NULL, 26, 4);

  /* free <mask>, <mask2> => <mask> */
  //Kill_Stack(mask);
  //mask = mask2;

  double z_scale = 1.0;
  if (m_traceWorkspace->resolution[0] != m_traceWorkspace->resolution[2]) {
    z_scale = m_traceWorkspace->resolution[0] / m_traceWorkspace->resolution[2];
  }

  Stack *mask2 = mask;
  /* resample the stack for dist calc if z is different */
  if (z_scale != 1.0) {
    mask2 = Resample_Stack_Depth(mask, NULL, z_scale);
    Stack_Binarize(mask2);
  }

  /* alloc <dist> */
  Stack *dist = Stack_Bwdist_L_U16(mask2, NULL, 0);

  if (mask != mask2) {
    Kill_Stack(mask2);
  }

  /* alloc <seeds> */
  Stack *seeds = Stack_Locmax_Region(dist, 26);

  /* alloc <objs> */
  Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);

  Zero_Stack(seeds);

  int objnum = 0;
  while (objs != NULL) {
    Object_3d *obj = objs->data;
    Voxel_t center;
    Object_3d_Central_Voxel(obj, center);
    Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
    objs = objs->next;
    objnum++;
  }

  /* free <objs> */
  Kill_Object_3d_List(objs);

  /* alloc <list> */
  Voxel_List *list = Stack_To_Voxel_List(seeds);

  /* free <seeds> */
  Kill_Stack(seeds);

  /* alloc <pa> */
  Pixel_Array *pa = Voxel_List_Sampling(dist, list);

  /* free <dist> */
  Kill_Stack(dist);

  /* alloc <voxel_array> */
  Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);

  uint16 *pa_array = (uint16 *) pa->array;

  /* alloc <seed_field> */
  Geo3d_Scalar_Field *seed_field = Make_Geo3d_Scalar_Field(pa->size);
  seed_field->size = 0;
  int i;
  for (i = 0; i < pa->size; i++) {
    seed_field->points[seed_field->size][0] = voxel_array[i]->x;
    seed_field->points[seed_field->size][1] = voxel_array[i]->y;
    seed_field->points[seed_field->size][2] = z_scale * voxel_array[i]->z;
    seed_field->values[seed_field->size] = sqrt((double)pa_array[i]);
    seed_field->size++;
  }

  /* free <list> */
  Kill_Voxel_List(list);

  /* free <voxel_array> */
  free(voxel_array);

  /* free <pa> */
  Kill_Pixel_Array(pa);

  /* now the seeds are in <field> */
  /* <mask> => <seed_mask> */
  Stack *seed_mask = mask;
  Zero_Stack(seed_mask);
  Locseg_Fit_Workspace *fws =
      (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
  Stack_Fit_Score old_fs = fws->sws->fs;

  fws->sws->fs.n = 2;
  fws->sws->fs.options[0] = STACK_FIT_DOT;
  fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;

  /* alloc <locseg> */
  Local_Neuroseg *locseg = (Local_Neuroseg *)
                           malloc(seed_field->size * sizeof(Local_Neuroseg));

  /* alloc <values> */
  double *values = darray_malloc(seed_field->size);

  /* fit segment on each seed */
  for (i = 0; i < seed_field->size; i++) {
    qDebug("-----------------------------> seed: %d / %d\n",
           i, seed_field->size);

    int index = i;
    int x = iround(seed_field->points[index][0]);
    int y = iround(seed_field->points[index][1]);
    int z = iround(seed_field->points[index][2]);

    double width = seed_field->values[index];

    ssize_t seed_offset =
        Stack_Util_Offset(x, y, z, stack->width, stack->height,
             stack->depth);

    if (seed_offset < 0) {
      continue;
    }

    if (width < 3.0) {
      width += 0.5;
    }

    Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_DEFAULT_H,
                 TZ_PI_2, 0.0, 0.0, 0.0, 1.0 / z_scale);
/*
    Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_HEIGHT,
                 0.0, 0.0, 0.0, 0.0, 1.0 / z_scale);
*/
    double cpos[3];
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;

    Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);

    if (seed_mask->array[seed_offset] > 0) {
      qDebug("labeled\n");
      values[i] = 0.0;
      continue;
    }

    Local_Neuroseg_Optimize_W(locseg + i, stack, 1.0, 0, fws);

    values[i] = fws->sws->fs.scores[1];

    qDebug("%g\n", values[i]);

    /* label seed_mask */
    if (values[i] > LOCAL_NEUROSEG_MIN_CORRCOEF) {
      Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 2, 1.0);
    } else {
      Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 1, 1.0);
    }

    //addLocsegChain(new ZLocsegChain(Copy_Local_Neuroseg(locseg + i)));
  }

  fws->sws->fs = old_fs;
  Kill_Stack(seed_mask);

  /* make trace mask */
  if (m_traceWorkspace->trace_mask == NULL) {
    m_traceWorkspace->trace_mask =
      Make_Stack(GREY16, stack->width, stack->height,
     stack->depth);
    Zero_Stack(m_traceWorkspace->trace_mask);
  }

  /* trace all seeds */
  int nchain;
  m_traceWorkspace->min_chain_length = 10;
  m_traceWorkspace->min_score = 0.1;
  Locseg_Chain **chain =
    Trace_Locseg_S(stack, 1.0, locseg, values, seed_field->size,
                   m_traceWorkspace, &nchain);

  /* tune ends */
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->signal = stack;
  ws->sratio = 1.0;
  ws->sdiff = 0.0;
  ws->option = 6;

  Zero_Stack(m_traceWorkspace->trace_mask);
  for (i = 0; i < nchain; i++) {
    if(chain[i] != NULL) {
      Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                           0, Locseg_Chain_Length(chain[i]) - 1,
                           ws);
    }
  }

  double old_step = m_traceWorkspace->trace_step;
  BOOL old_refit = m_traceWorkspace->refit;
  BOOL traceMasked = Trace_Workspace_Is_Masked(m_traceWorkspace);

  loadTraceMask(true);
  m_traceWorkspace->trace_step = 0.1;
  m_traceWorkspace->refit = FALSE;

  for (i = 0; i < nchain; i++) {
    if (chain[i] != NULL) {
      /* erase the mask */
      ws->option = 7;
      Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                           0, Locseg_Chain_Length(chain[i]) - 1,
                           ws);

      m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
      m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
      Trace_Locseg(stack, 1.0, chain[i], m_traceWorkspace);
      Locseg_Chain_Down_Sample(chain[i]);

      Locseg_Chain_Tune_End(chain[i], stack, 1.0,
                            m_traceWorkspace->trace_mask, DL_HEAD);
      Locseg_Chain_Tune_End(chain[i], stack, 1.0,
                            m_traceWorkspace->trace_mask, DL_TAIL);

      if (Locseg_Chain_Length(chain[i]) > 0) {
        ws->option = 6;
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1, ws);
      }
    }
  }

  ws->signal = NULL;
  Kill_Locseg_Label_Workspace(ws);

  m_traceWorkspace->trace_step = old_step;
  m_traceWorkspace->refit = old_refit;
  loadTraceMask(traceMasked);

  Zero_Stack(m_traceWorkspace->trace_mask);
  m_traceWorkspace->chain_id = 0;

  /* add chains */
  for (i = 0; i < nchain; i++) {
    if (chain[i] != NULL) {
      if (Locseg_Chain_Length(chain[i]) > 0) {
        ZLocsegChain *zchain = new ZLocsegChain(chain[i]);
        if (zchain->confidence(stack) > 0.03) {
          addLocsegChain(zchain);
        } else {
          delete zchain;
        }
      }
    }
  }

  free(chain);

  /* free <seed_field> */
  Kill_Geo3d_Scalar_Field(seed_field);

  /* free <values> */
  free(values);

  /* free <locseg> */
  free(locseg);
}

#if 0
void ZStackDoc::autoTraceAxon()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    m_progressReporter->start();
    QDir workDir(m_traceWorkspace->save_path);

    if (!workDir.exists()) {
      workDir.mkdir(workDir.absolutePath());
    }

    m_progressReporter->advance(0.05);

    m_progressReporter->startSubprogress(0.1);
    int thre = autoThreshold();
    m_progressReporter->endSubprogress(0.1);

    /* alloc <mask> */
    Stack *mask = Copy_Stack(mainStack->c_stack());
    Stack_Threshold_Binarize(mask, thre);
    Translate_Stack(mask, GREY, 1);

    m_progressReporter->advance(0.05);

    /* alloc <mask2> */
    //Stack *mask2 = Stack_Majority_Filter_R(mask, NULL, 26, 4);

    /* free <mask>, <mask2> => <mask> */
    //Kill_Stack(mask);
    //mask = mask2;

    double z_scale = 1.0;
    if (m_traceWorkspace->resolution[0] != m_traceWorkspace->resolution[2]) {
      z_scale = m_traceWorkspace->resolution[0] / m_traceWorkspace->resolution[2];
    }

    Stack *mask2 = mask;
    /* resample the stack for dist calc if z is different */
    if (z_scale != 1.0) {
      mask2 = Resample_Stack_Depth(mask, NULL, z_scale);
      Stack_Binarize(mask2);
    }

    /* alloc <dist> */
    Stack *dist = Stack_Bwdist_L_U16(mask2, NULL, 0);

    if (mask != mask2) {
      Kill_Stack(mask2);
    }

    m_progressReporter->advance(0.05);

    /* alloc <seeds> */
    Stack *seeds = Stack_Locmax_Region(dist, 26);

    m_progressReporter->advance(0.05);

    /* alloc <objs> */
    Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);

    Zero_Stack(seeds);

    int objnum = 0;
    while (objs != NULL) {
      Object_3d *obj = objs->data;
      Voxel_t center;
      Object_3d_Central_Voxel(obj, center);
      Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
      objs = objs->next;
      objnum++;
    }

    /* free <objs> */
    Kill_Object_3d_List(objs);

    m_progressReporter->advance(0.05);

    /* use eigenvalue of hessian to detect seed*/
    double sigma[] = {1.0, 1.0, 1.0};
    sigma[2] *= z_scale;
    FMatrix *result = NULL;

    if (mainStack->width() * mainStack->height() * mainStack->depth() > 1024 * 1024 * 100) {
      result = El_Stack_L_F(mainStack->c_stack(), sigma, NULL);
    } else {
      result = El_Stack_F(mainStack->c_stack(), sigma, NULL);
    }
    Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
        result->dim[2], GREY16);
    Stack *stack2 = Copy_Stack(out);

    m_progressReporter->advance(0.05);

    Kill_FMatrix(result);
    thre = autoThreshold(out);
    Stack_Threshold_Binarize(out, thre);
    Translate_Stack(out, GREY, 1);
    Stack *dist2 = Stack_Bwdist_L_U16(out, NULL, 0);
    Kill_Stack(out);
    Stack *seeds2 = Stack_Locmax_Region(dist2, 26);
    Kill_Stack(dist2);
    objs = Stack_Find_Object_N(seeds2, NULL, 1, 0, 26);
    Kill_Stack(seeds2);
    int seedId = 0;
    QList<ZPunctum> seedsAsPuncta;
    Zero_Stack(seeds);
    while (objs != NULL) {
      Object_3d *obj = objs->data;
      Voxel_t center;
      Object_3d_Central_Voxel(obj, center);
      Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
      objs = objs->next;
      seedsAsPuncta.push_back(ZPunctum(seedId++, center[0], center[1], center[2]));
    }
#ifdef __APPLE__
    ZPunctumIO::save("../../../../data/axon_test_seeds.apo", seedsAsPuncta);
#else
    ZPunctumIO::writePunctaFile("../data/axon_test_seeds.apo", seedsAsPuncta);
#endif

    Kill_Object_3d_List(objs);

    m_progressReporter->advance(0.05);

    /* alloc <list> */
    Voxel_List *list = Stack_To_Voxel_List(seeds);
    LINFO() << "voxel list size:" << Voxel_List_Length(list);

    /* free <seeds> */
    Kill_Stack(seeds);

    /* alloc <pa> */
    Pixel_Array *pa = Voxel_List_Sampling(dist, list);
    LINFO() << "pa size:" << pa->size;

    /* free <dist> */
    Kill_Stack(dist);

    /* alloc <voxel_array> */
    Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);

    uint16 *pa_array = (uint16 *) pa->array;

    /* alloc <seed_field> */
    Geo3d_Scalar_Field *seed_field = Make_Geo3d_Scalar_Field(pa->size);
    seed_field->size = 0;
    int i;
    for (i = 0; i < pa->size; i++) {
      seed_field->points[seed_field->size][0] = voxel_array[i]->x;
      seed_field->points[seed_field->size][1] = voxel_array[i]->y;
      seed_field->points[seed_field->size][2] = z_scale * voxel_array[i]->z;
      seed_field->values[seed_field->size] = sqrt((double)pa_array[i]);
      seed_field->size++;
    }
    LINFO() << "seed_field size:" << seed_field->size;

    /* free <list> */
    Kill_Voxel_List(list);

    /* free <voxel_array> */
    free(voxel_array);

    /* free <pa> */
    Kill_Pixel_Array(pa);

    /* now the seeds are in <field> */
    /* <mask> => <seed_mask> */
    Stack *seed_mask = mask;
    Zero_Stack(seed_mask);
    Locseg_Fit_Workspace *fws =
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
    Stack_Fit_Score old_fs = fws->sws->fs;

    fws->sws->fs.n = 2;
    fws->sws->fs.options[0] = STACK_FIT_DOT;
    fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;

    /* alloc <locseg> */
    Local_Neuroseg *locseg = (Local_Neuroseg *)
        malloc(seed_field->size * sizeof(Local_Neuroseg));

    /* alloc <values> */
    double *values = darray_malloc(seed_field->size);

    m_progressReporter->advance(0.05);

    seedsAsPuncta.clear();
    ZColorMap cm(0.0, 1.0, glm::col4(0,0,255,255), glm::col4(255,0,0,255));

    /* fit segment on each seed */
    for (i = 0; i < seed_field->size; i++) {
      qDebug("-----------------------------> seed: %d / %d\n",
             i, seed_field->size);

      int index = i;
      int x = iround(seed_field->points[index][0]);
      int y = iround(seed_field->points[index][1]);
      int z = iround(seed_field->points[index][2]);

      double width = seed_field->values[index];

      ssize_t seed_offset =
          Stack_Util_Offset(x, y, z, stack()->width(), stack()->height(),
                            stack()->depth());

      if (seed_offset < 0) {
        continue;
      }

      if (width < 3.0) {
        width += 0.5;
      }

      Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_DEFAULT_H,
                   TZ_PI_2, 0.0, 0.0, 0.0, 1.0 / z_scale);
      /*
    Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_HEIGHT,
                 0.0, 0.0, 0.0, 0.0, 1.0 / z_scale);
*/
      double cpos[3];
      cpos[0] = x;
      cpos[1] = y;
      cpos[2] = z;

      Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);

      if (seed_mask->array[seed_offset] > 0) {
        qDebug("labeled\n");
        values[i] = 0.0;

        ZPunctum tmpPunctum(i, x, y, z);
        tmpPunctum.setColor(cm.getMappedQColor(values[i]));
        seedsAsPuncta.push_back(tmpPunctum);
        continue;
      }

      Local_Neuroseg_Optimize_W(locseg + i, stack()->c_stack(), 1.0, 1, fws);

      values[i] = fws->sws->fs.scores[1];

      ZPunctum tmpPunctum(i, x, y, z);
      tmpPunctum.setColor(cm.getMappedQColor(values[i]));
      seedsAsPuncta.push_back(tmpPunctum);

      qDebug("%g\n", values[i]);

      /* label seed_mask */
      if (values[i] > LOCAL_NEUROSEG_MIN_CORRCOEF) {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 2, 1.0);
      } else {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 1, 1.0);
      }

      //addLocsegChain(new ZLocsegChain(Copy_Local_Neuroseg(locseg + i)));
    }

#ifdef __APPLE__
    ZPunctumIO::save("../../../../data/axon_test_seeds_colormap.apo", seedsAsPuncta);
#else
    ZPunctumIO::writePunctaFile("../data/axon_test_seeds_colormap.apo", seedsAsPuncta);
#endif

    QList<ZPunctum> seedsAsPuncta1;
    for (int index=0; index<seedsAsPuncta.size(); index++) {
      if (Local_Neuroseg_Good_Score(locseg + index, values[index], m_traceWorkspace->min_score) == TRUE)
        seedsAsPuncta1.push_back(seedsAsPuncta[index]);
    }

#ifdef __APPLE__
    ZPunctumIO::save("../../../../data/axon_test_highscore_seeds_colormap.apo", seedsAsPuncta);
#else
    ZPunctumIO::writePunctaFile("../data/axon_test_highscore_seeds_colormap.apo", seedsAsPuncta);
#endif

    LINFO() << "seeds min score:" << *std::min_element(values, values+seed_field->size);
    LINFO() << "seeds max score:" << *std::max_element(values, values+seed_field->size);

    fws->sws->fs = old_fs;
    Kill_Stack(seed_mask);

    /* make trace mask */
    if (m_traceWorkspace->trace_mask == NULL) {
      m_traceWorkspace->trace_mask =
          Make_Stack(GREY16, mainStack->width(), mainStack->height(),
                     mainStack->depth());
      Zero_Stack(m_traceWorkspace->trace_mask);
    }

    m_progressReporter->advance(0.05);

    /* trace all seeds */
    int nchain;
    m_traceWorkspace->min_chain_length = 10;
    m_traceWorkspace->min_score = 0.1;
    int *lowScoreSeedsIndex = new int[seed_field->size];
    int *tooThickSeedsIndex = new int[seed_field->size];
    int *shortChainsSeedsIndex = new int[seed_field->size];
    int nlowScoreSeeds;
    int ntooThickSeeds;
    int nshortChainsSeeds;
    Locseg_Chain **chain =
        ZLocsegTraceExp::Trace_Locseg_S_Exp(
          mainStack->c_stack(), 1.0, locseg, values, seed_field->size,
          m_traceWorkspace, &nchain, lowScoreSeedsIndex, tooThickSeedsIndex, shortChainsSeedsIndex,
          &nlowScoreSeeds, &ntooThickSeeds, &nshortChainsSeeds);

    m_progressReporter->advance(0.05);

    int nlowScoreChain = 0;
    Locseg_Chain **lowScoreChain = NULL;
    if (nlowScoreSeeds > 0) {
      Local_Neuroseg *lowscorelocseg = (Local_Neuroseg *)
          malloc(nlowScoreSeeds * sizeof(Local_Neuroseg));
      for (int i=0; i<nlowScoreSeeds; i++) {
        Local_Neuroseg_Copy(lowscorelocseg+i, locseg+lowScoreSeedsIndex[i]);
      }
      lowScoreChain = Trace_Locseg_S(stack2, 1.0, lowscorelocseg, NULL, nlowScoreSeeds, m_traceWorkspace, &nlowScoreChain);
      free(lowscorelocseg);
    }

    int ntooThickChain = 0;
    Locseg_Chain **tooThickChain = NULL;
    if (ntooThickSeeds > 0) {
      Local_Neuroseg *toothicklocseg = (Local_Neuroseg *)
          malloc(ntooThickSeeds * sizeof(Local_Neuroseg));
      for (int i=0; i<ntooThickSeeds; i++) {
        Local_Neuroseg_Copy(toothicklocseg+i, locseg+tooThickSeedsIndex[i]);
      }
      tooThickChain = Trace_Locseg_S(stack2, 1.0, toothicklocseg, NULL, ntooThickSeeds, m_traceWorkspace, &ntooThickChain);
      free(toothicklocseg);
    }

    int nshortChainsChain = 0;
    Locseg_Chain **shortChainsChain = NULL;
    if (nshortChainsSeeds > 0) {
      Local_Neuroseg *shortchainslocseg = (Local_Neuroseg *)
          malloc(nshortChainsSeeds * sizeof(Local_Neuroseg));
      for (int i=0; i<nshortChainsSeeds; i++) {
        Local_Neuroseg_Copy(shortchainslocseg+i, locseg+shortChainsSeedsIndex[i]);
      }
      shortChainsChain = Trace_Locseg_S(stack2, 1.0, shortchainslocseg, NULL, nshortChainsSeeds, m_traceWorkspace, &nshortChainsChain);
      free(shortchainslocseg);
    }

    m_progressReporter->advance(0.05);

    /* tune ends */
    Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
    ws->signal = stack()->c_stack();
    ws->sratio = 1.0;
    ws->sdiff = 0.0;
    ws->option = 6;

    Zero_Stack(m_traceWorkspace->trace_mask);
    for (i = 0; i < nchain; i++) {
      if(chain[i] != NULL) {
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);
      }
    }

    for (i = 0; i< nlowScoreChain; i++) {
      if (lowScoreChain[i] != NULL) {
        Locseg_Chain_Label_W(lowScoreChain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(lowScoreChain[i]) - 1,
                             ws);
      }
    }

    for (i = 0; i< ntooThickChain; i++) {
      if (tooThickChain[i] != NULL) {
        Locseg_Chain_Label_W(tooThickChain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(tooThickChain[i]) - 1,
                             ws);
      }
    }

    for (i = 0; i< nshortChainsChain; i++) {
      if (shortChainsChain[i] != NULL) {
        Locseg_Chain_Label_W(shortChainsChain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(shortChainsChain[i]) - 1,
                             ws);
      }
    }

    double old_step = m_traceWorkspace->trace_step;
    BOOL old_refit = m_traceWorkspace->refit;
    BOOL traceMasked = Trace_Workspace_Is_Masked(m_traceWorkspace);

    loadTraceMask(true);
    m_traceWorkspace->trace_step = 0.1;
    m_traceWorkspace->refit = FALSE;

    for (i = 0; i < nchain; i++) {
      if (chain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        ZLocsegTraceExp::Trace_Locseg_Exp(
              stack()->c_stack(), 1.0, chain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(chain[i]);

        Locseg_Chain_Tune_End(chain[i], stack()->c_stack(), 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(chain[i], stack()->c_stack(), 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(chain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(chain[i]) - 1, ws);
        }
      }
    }

    m_progressReporter->advance(0.05);

    for (i = 0; i< nlowScoreChain; i++) {
      if (lowScoreChain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(lowScoreChain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(lowScoreChain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        ZLocsegTraceExp::Trace_Locseg_Exp(stack2, 1.0, lowScoreChain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(lowScoreChain[i]);

        Locseg_Chain_Tune_End(lowScoreChain[i], stack2, 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(lowScoreChain[i], stack2, 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(lowScoreChain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(lowScoreChain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(lowScoreChain[i]) - 1, ws);
        }
      }
    }

    m_progressReporter->advance(0.05);

    for (i = 0; i< ntooThickChain; i++) {
      if (tooThickChain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(tooThickChain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(tooThickChain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        ZLocsegTraceExp::Trace_Locseg_Exp(stack2, 1.0, tooThickChain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(tooThickChain[i]);

        Locseg_Chain_Tune_End(tooThickChain[i], stack2, 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(tooThickChain[i], stack2, 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(tooThickChain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(tooThickChain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(tooThickChain[i]) - 1, ws);
        }
      }
    }

    m_progressReporter->advance(0.05);

    for (i = 0; i< nshortChainsChain; i++) {
      if (shortChainsChain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(shortChainsChain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(shortChainsChain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        ZLocsegTraceExp::Trace_Locseg_Exp(stack2, 1.0, shortChainsChain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(shortChainsChain[i]);

        Locseg_Chain_Tune_End(shortChainsChain[i], stack2, 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(shortChainsChain[i], stack2, 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(shortChainsChain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(shortChainsChain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(shortChainsChain[i]) - 1, ws);
        }
      }
    }

    m_progressReporter->advance(0.05);

    ws->signal = NULL;
    ws->option = 1;
    ws->value = Stack_Common_Intensity(stack()->c_stack(), 3, 65535);
    Stack *stack3 = Copy_Stack(stack()->c_stack());

    m_traceWorkspace->trace_step = old_step;
    m_traceWorkspace->refit = old_refit;
    loadTraceMask(traceMasked);

    Zero_Stack(m_traceWorkspace->trace_mask);
    m_traceWorkspace->chain_id = 0;

    int nlowscore = 0;
    /* add chains */
    for (i = 0; i < nchain; i++) {
      if (chain[i] != NULL) {
        if (Locseg_Chain_Length(chain[i]) > 0) {
          ZLocsegChain *zchain = new ZLocsegChain(chain[i]);
          if (zchain->confidence(stack()->c_stack()) > 0.5) {
            addLocsegChain(zchain);
            Locseg_Chain_Label_W(chain[i], stack3, 1.0, 0, Locseg_Chain_Length(chain[i]) - 1, ws);
          } else {
            //qDebug() << zchain->confidence(stack()->data());
            //addLocsegChain(zchain);
            nlowscore++;
            delete zchain;
          }
        }
      }
    }

    m_progressReporter->advance(0.05);

    LINFO() << QString("total num nchain: %1, low score num: %2.").arg(nchain).arg(nlowscore);

    nlowscore = 0;
    for (i = 0; i< nlowScoreChain; i++) {
      if (lowScoreChain[i] != NULL) {
        if (Locseg_Chain_Length(lowScoreChain[i]) > 0) {
          ZLocsegChain *zchain = new ZLocsegChain(lowScoreChain[i]);
          if (zchain->confidence(stack2) > 0.5) {
            addLocsegChain(zchain);
            Locseg_Chain_Label_W(lowScoreChain[i], stack3, 1.0, 0, Locseg_Chain_Length(lowScoreChain[i]) - 1, ws);
          } else {
            //qDebug() << zchain->confidence(stack()->data());
            //addLocsegChain(zchain);
            nlowscore++;
            delete zchain;
          }
        }
      }
    }

    m_progressReporter->advance(0.05);

    LINFO() << QString("total num nlowscorechain: %1, low score num: %2.").arg(nlowScoreChain).arg(nlowscore);

    nlowscore = 0;
    for (i = 0; i< ntooThickChain; i++) {
      if (tooThickChain[i] != NULL) {
        if (Locseg_Chain_Length(tooThickChain[i]) > 0) {
          ZLocsegChain *zchain = new ZLocsegChain(tooThickChain[i]);
          if (zchain->confidence(stack2) > 0.5) {
            addLocsegChain(zchain);
            Locseg_Chain_Label_W(tooThickChain[i], stack3, 1.0, 0, Locseg_Chain_Length(tooThickChain[i]) - 1, ws);
          } else {
            //qDebug() << zchain->confidence(stack()->data());
            //addLocsegChain(zchain);
            nlowscore++;
            delete zchain;
          }
        }
      }
    }

    m_progressReporter->advance(0.05);

    LINFO() << QString("total num ntooThickChain: %1, low score num: %2.").arg(ntooThickChain).arg(nlowscore);

    nlowscore = 0;
    for (i = 0; i< nshortChainsChain; i++) {
      if (shortChainsChain[i] != NULL) {
        if (Locseg_Chain_Length(shortChainsChain[i]) > 0) {
          ZLocsegChain *zchain = new ZLocsegChain(shortChainsChain[i]);
          if (zchain->confidence(stack2) > 0.5) {
            addLocsegChain(zchain);
            Locseg_Chain_Label_W(shortChainsChain[i], stack3, 1.0, 0, Locseg_Chain_Length(shortChainsChain[i]) - 1, ws);
          } else {
            //qDebug() << zchain->confidence(stack()->data());
            //addLocsegChain(zchain);
            nlowscore++;
            delete zchain;
          }
        }
      }
    }

    m_progressReporter->advance(0.05);

    LINFO() << QString("total num nshortChainsChain: %1, low score num: %2.").arg(nshortChainsChain).arg(nlowscore);

    Kill_Locseg_Label_Workspace(ws);
    Kill_Stack(stack2);

    free(chain);
    free(lowScoreChain);
    free(tooThickChain);
    free(shortChainsChain);

    /* free <seed_field> */
    Kill_Geo3d_Scalar_Field(seed_field);

    /* free <values> */
    free(values);

    /* free <locseg> */
    free(locseg);

    m_progressReporter->end();
  }
}
#endif

void ZStackDoc::traceFromSwc(QProgressBar *pb)
{
  if (pb != NULL) {
    pb->setRange(0, m_swcList.length());
  }

  if (!m_swcList.isEmpty()) {
    foreach (ZSwcTree *tree, m_swcList) {
      int n = tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);
      Geo3d_Ball *ball = NULL;
      if (Swc_Tree_Node_Is_Virtual(tree->begin())) {
        n--;
        tree->next();
      }
      GUARDED_MALLOC_ARRAY(ball, n, Geo3d_Ball);
      int length = 0;
      Locseg_Fit_Workspace *ws =
          (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
      Swc_Tree_Node *tn = NULL;
      while ((tn = tree->next()) != NULL) {
        if (Swc_Tree_Node_Is_Leaf(tn)) {
          ball[length].center[0] = Swc_Tree_Node_Data(tn)->x;
          ball[length].center[1] = Swc_Tree_Node_Data(tn)->y;
          ball[length].center[2] = Swc_Tree_Node_Data(tn)->z;
          ball[length].r = Swc_Tree_Node_Data(tn)->d;
          length++;
          if (stack()->isTracable()) {
            ws = (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
          } else {
            ws = NULL;
          }
          Locseg_Chain *chain =
              Locseg_Chain_From_Ball_Array(ball, length, stack()->c_stack(),
                                           stack()->preferredZScale(), ws);

          if (chain != NULL) {
            addLocsegChain(new ZLocsegChain(chain));
          }
          length = 0;
        } else {
          ball[length].center[0] = Swc_Tree_Node_Data(tn)->x;
          ball[length].center[1] = Swc_Tree_Node_Data(tn)->y;
          ball[length].center[2] = Swc_Tree_Node_Data(tn)->z;
          ball[length].r = Swc_Tree_Node_Data(tn)->d;
          length++;
        }
      }
      free(ball);
      if (pb != NULL) {
        pb->setValue(pb->value() + 1);
      }
    }
    emit chainModified();
  }

  if (pb != NULL) {
    pb->hide();
  }
}

bool ZStackDoc::binarize(int threshold)
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    if (threshold < 0) {
      threshold = 0;
    }

    if (mainStack->binarize(threshold)) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::bwsolid()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    if (mainStack->bwsolid()) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::invert()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    ZStackProcessor::invert(mainStack);
    emit stackModified();
    return true;
  }

  return false;
}

bool ZStackDoc::enhanceLine()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    if (mainStack->enhanceLine()) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::importSynapseAnnotation(const std::string &filePath)
{
  FlyEm::ZSynapseAnnotationArray synapseArray;
  if (synapseArray.loadJson(filePath)) {
    ZSwcTree *tree = synapseArray.toSwcTree();
    ZStack *mainStack = stack();
    if (mainStack != NULL) {
      tree->flipY(mainStack->height() - 1);
    }

    addSwcTree(tree);
    return true;
  }

  return false;
}

void ZStackDoc::loadFileList(const QList<QUrl> &urlList)
{
  QStringList fileList;
  for (QList<QUrl>::const_iterator iter = urlList.begin();
       iter != urlList.end(); ++iter) {
    // load files inside if is folder
    QFileInfo dirCheck(iter->toLocalFile());
    if (dirCheck.isDir()) {
      QDir dir = dirCheck.absoluteDir();
      QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
      for (int i=0; i<list.size(); i++) {
        fileList.append(list.at(i).absoluteFilePath());
      }
    } else {
      fileList.append(dirCheck.absoluteFilePath());
    }
  }

  loadFileList(fileList);
}

void ZStackDoc::loadFileList(const QStringList &fileList)
{
  bool swcLoaded = false;
  bool chainLoaded = false;
  bool networkLoaded = false;
  bool punctaLoaded = false;
  //bool apoLoaded = false;

  for (QStringList::const_iterator iter = fileList.begin(); iter != fileList.end();
       ++iter) {
    switch (ZFileType::fileType(iter->toStdString())) {
    case ZFileType::SWC_FILE:
    case ZFileType::SYNAPSE_ANNOTATON_FILE:
      swcLoaded = true;
      break;
    case ZFileType::SWC_NETWORK_FILE:
    case ZFileType::FLYEM_NETWORK_FILE:
      swcLoaded = true;
      networkLoaded = true;
      break;
    case ZFileType::LOCSEG_CHAIN_FILE:
      chainLoaded = true;
      break;
    case ZFileType::V3D_APO_FILE:
    case ZFileType::V3D_MARKER_FILE:
    case ZFileType::RAVELER_BOOKMARK:
      punctaLoaded = true;
      break;
    default:
      break;
    }

    blockSignals(true);
    loadFile(*iter, false);
    blockSignals(false);
  }

  if (swcLoaded) {
    emit swcModified();
  }

  if (chainLoaded) {
    emit chainModified();
  }

  if (networkLoaded) {
    emit swcNetworkModified();
  }

  if (punctaLoaded) {
    emit punctaModified();
  }

#ifdef _FLYEM_2
  emit punctaModified();
#endif
}

void ZStackDoc::loadFile(const QString &filePath, bool emitMessage)
{
  switch (ZFileType::fileType(filePath.toStdString())) {
  case ZFileType::SWC_FILE:
#ifdef _FLYEM_2
    removeAllObject();
#endif
    loadSwc(filePath);
    if (emitMessage) {
      emit swcModified();
    }
    break;
  case ZFileType::LOCSEG_CHAIN_FILE:
    loadLocsegChain(filePath);
    if (emitMessage) {
      emit chainModified();
    }
    break;
  case ZFileType::SWC_NETWORK_FILE:
    loadSwcNetwork(filePath);
    if (emitMessage) {
      emit swcModified();
      emit swcNetworkModified();
    }
    break;
  case ZFileType::TIFF_FILE:
  case ZFileType::LSM_FILE:
  case ZFileType::V3D_RAW_FILE:
    readStack(filePath.toStdString().c_str());
    if (emitMessage) {
      stackModified();
    }
    break;
  case ZFileType::FLYEM_NETWORK_FILE:
    importFlyEmNetwork(filePath.toStdString().c_str());
    if (emitMessage) {
      emit swcModified();
      emit swcNetworkModified();
    }
    break;
  case ZFileType::SYNAPSE_ANNOTATON_FILE:
    if (importSynapseAnnotation(filePath.toStdString())) {
      if (emitMessage) {
        emit swcModified();
      }
    }
    break;
  case ZFileType::V3D_APO_FILE:
  case ZFileType::V3D_MARKER_FILE:
  case ZFileType::RAVELER_BOOKMARK:
    if (importPuncta(filePath.toStdString().c_str())) {
      if (emitMessage) {
        emit punctaModified();
      }
    }
    break;
  default:
    break;
  }
}

void ZStackDoc::deprecateDependent(EComponent component)
{
  switch (component) {
  case STACK:
    break;
  default:
    break;
  }
}

void ZStackDoc::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case STACK:
    delete stackRef();
    stackRef() = NULL;
    break;
  default:
    break;
  }
}

bool ZStackDoc::isDeprecated(EComponent component)
{
  switch (component) {
  case STACK:
    return stackRef() == NULL;
    break;
  default:
    return false;
  }

  return false;
}

Swc_Tree_Node* ZStackDoc::swcHitTest(int x, int y, int z)
{
  Swc_Tree_Node *selected = NULL;
  const double Margin = 0.5;

  for (QList<ZSwcTree*>::iterator iter = m_swcList.begin();
       iter != m_swcList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    if (z < 0) {
      selected = tree->hitTest(x, y);
    } else {
      selected = tree->hitTest(x, y, z, Margin);
    }

    if (selected != NULL) {
      break;
    }
  }

  return selected;
}

void ZStackDoc::selectSwcTreeNode(Swc_Tree_Node *selected, bool append)
{
  if (!append) {
    selectedSwcTreeNodes()->clear();
    if (selected != NULL) {
      selectedSwcTreeNodes()->insert(selected);
    }
  } else {
    if (selected != NULL) {
      if (selectedSwcTreeNodes()->count(selected) > 0) {
        selectedSwcTreeNodes()->erase(selected);
      } else {
        selectedSwcTreeNodes()->insert(selected);
      }
    }
  }
}

Swc_Tree_Node *ZStackDoc::selectSwcTreeNode(int x, int y, int z, bool append)
{
  Swc_Tree_Node *selected = swcHitTest(x, y, z);

  selectSwcTreeNode(selected, append);

  return selected;
}

void ZStackDoc::updateStackFromSource()
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    if (mainStack->isSwc()) {
      readSwc(mainStack->sourcePath());
      emit stackModified();
    } else {
      if (mainStack->updateFromSource()) {
        emit stackModified();
      }
    }
  }
}

int ZStackDoc::maxIntesityDepth(int x, int y)
{
  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    return mainStack->maxIntensityDepth(x, y);
  }

  return 0;
}

void ZStackDoc::test(QProgressBar *pb)
{
#if 0
  importLocsegChainConn("/Users/zhaot/work/neurolabi/data/diadem_e1/conn.xml");
  for (int i = 0; i < m_connList.size(); i++) {
    m_connList.at(i)->print();
  }
#endif
  UNUSED_PARAMETER(pb);

  ZStack *mainStack = stack();
  if (mainStack != NULL) {
    mainStack->enhanceLine();
  }
}

const char* ZStackDoc::tubePrefix() const
{
  if (m_traceWorkspace != NULL) {
    return m_traceWorkspace->save_prefix;
  }

  return NULL;
}

#if 0
void ZStackDoc::updatePreviewSwc()
{
  //Need modification
  Swc_Tree *tree = this->swcReconstruction(0, false, true);
  if (tree != NULL) {
    if (m_previewSwc == NULL) {
      m_previewSwc = new ZSwcTree;
    }
    m_previewSwc->setData(tree);
  }
}
#endif

void ZStackDoc::notifySwcModified()
{
  foreach (ZSwcTree *tree, m_swcList) {
    tree->deprecate(ZSwcTree::ALL_COMPONENT);
  }

  emit swcModified();
}

void ZStackDoc::notifyPunctumModified()
{
  emit punctaModified();
}

void ZStackDoc::notifyChainModified()
{
  emit chainModified();
}

void ZStackDoc::notifyObj3dModified()
{
  emit obj3dModified();
}

void ZStackDoc::notifyStackModified()
{
  emit stackModified();
}

void ZStackDoc::notifyStrokeModified()
{
  emit strokeModified();
}

void ZStackDoc::notifyObjectModified()
{
  emit objectModified();
}

void ZStackDoc::notifyAllObjectModified()
{
  notifySwcModified();
  notifyPunctumModified();
  notifyChainModified();
  notifyObj3dModified();
  notifyStrokeModified();
}

bool ZStackDoc::watershed()
{
  ZStack *mainStack = stack();
  m_progressReporter->start();
  m_progressReporter->advance(0.5);
  if (mainStack != NULL) {
    if (mainStack->watershed()) {
      emit stackModified();
      return true;
    }
  }
  m_progressReporter->end();

  return false;
}

int ZStackDoc::findLoop(int minLoopSize)
{
  int loopNumber = 0;

  if (hasStackData()) {
    m_progressReporter->start();

    Stack *data = C_Stack::clone(stack()->c_stack(0));

    m_progressReporter->advance(0.1);

    Stack_Binarize(data);
    Stack *filled = Stack_Fill_Hole_N(data, NULL, 1, 6, NULL);

    m_progressReporter->advance(0.1);
    Stack *shrinked = Stack_Bwpeel(filled, REMOVE_ARC, NULL);
    C_Stack::kill(filled);

    m_progressReporter->advance(0.2);
#ifdef _DEBUG_2
    const NeutubeConfig &config = NeutubeConfig::getInstance();
    C_Stack::write(config.getPath(NeutubeConfig::DATA) + "/test.tif", shrinked);
#endif
    //Stack_Threshold(shrinked, 100);
    //m_progressReporter->update(40);

    ZStackGraph stackGraph;
    ZGraph *graph = stackGraph.buildForegroundGraph(shrinked);

    graph->setProgressReporter(m_progressReporter);
    m_progressReporter->advance(0.1);

    ZGraphCompressor compressor;
    compressor.setGraph(graph);
    compressor.compress();
    m_progressReporter->advance(0.1);

    m_progressReporter->startSubprogress(0.3);
    std::vector<std::vector<int> > cycleArray = graph->getCycle();
    graph->setProgressReporter(m_progressReporter);
    for (size_t i = 0; i < cycleArray.size(); ++i) {
      vector<int> path = cycleArray[i];
#ifdef _DEBUG_
      cout << "Cycle size: " << path.size() << endl;
#endif
      if ((int) path.size() >= minLoopSize) {
        ZObject3d *obj = new ZObject3d;
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          int x, y, z;
          C_Stack::indexToCoord(compressor.uncompress(*iter), C_Stack::width(data),
                                C_Stack::height(data), &x, &y, &z);
          obj->append(x, y, z);
        }
        addObj3d(obj);
        ++loopNumber;
      }
    }
    m_progressReporter->endSubprogress(0.3);
    /*
    vector<bool> labeled(graph->getVertexNumber(), false);
    //For each loop in the graph, label it
    for (int i = 0; i < graph->getVertexNumber(); ++i) {
      if (!labeled[i]) {
        vector<int> path = graph->getPath(i, i);
        ZObject3d *obj = new ZObject3d;
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;

          if (path.size() > 100) {
            int x, y, z;
            C_Stack::indexToCoord(compressor.uncompress(*iter), C_Stack::width(data),
                                  C_Stack::height(data), &x, &y, &z);
            obj->append(x, y, z);
          }
        }
        if (obj->size() > 0) {
          addObj3d(obj);
        } else {
          delete obj;
        }

        ++loopNumber;
      }
    }
    */
    m_progressReporter->advance(0.1);

    if (loopNumber > 0) {
      emit obj3dModified();
    }

    delete graph;
    Kill_Stack(shrinked);
    Kill_Stack(data);

    m_progressReporter->end();
  }

  return loopNumber;
}

void ZStackDoc::bwthin()
{
  if (hasStackData()) {
    m_progressReporter->start();

    if (C_Stack::kind(stack()->c_stack(0)) == GREY) {
      m_progressReporter->advance(0.1);
      Stack *out = Stack_Bwthin(stack()->c_stack(0), NULL);
      m_progressReporter->advance(0.5);
      C_Stack::copyValue(out, stack()->c_stack(0));
      C_Stack::kill(out);
      m_progressReporter->advance(0.3);
      stack()->deprecateSingleChannelView(0);
      emit stackModified();
    }

    m_progressReporter->end();
  }
}

void ZStackDoc::executeSwcRescaleCommand(const ZRescaleSwcSetting &setting)
{
  QUndoCommand *allcommand = new QUndoCommand();
  if (setting.bTranslateSoma) {
    new ZStackDocCommand::SwcEdit::TranslateRoot(
          this, setting.translateDstX, setting.translateDstY,
          setting.translateDstZ, allcommand);
  }
  if (setting.scaleX != 1 || setting.scaleY != 1 || setting.scaleZ != 1) {
    new ZStackDocCommand::SwcEdit::Rescale(
          this, setting.scaleX, setting.scaleY, setting.scaleZ, allcommand);
  }
  if (setting.bRescaleBranchRadius) {
    new ZStackDocCommand::SwcEdit::RescaleRadius(
          this, setting.rescaleBranchRadiusScale, setting.somaCutLevel+1, -1,
          allcommand);
  }
  if (setting.bRescaleSomaRadius) {
    new ZStackDocCommand::SwcEdit::RescaleRadius(
          this, setting.rescaleSomaRadiusScale, 0, setting.somaCutLevel+1,
          allcommand);
  }
  if (setting.bReduceSwcNodes) {
    new ZStackDocCommand::SwcEdit::ReduceNodeNumber(
          this, setting.reduceSwcNodesLengthThre, allcommand);
  }

  if (allcommand->childCount() > 0) {
    allcommand->setText(QObject::tr("rescale swc"));
    pushUndoCommand(allcommand);
    deprecateTraceMask();
  } else {
    delete allcommand;
  }
}

bool ZStackDoc::executeSwcNodeExtendCommand(const ZPoint &center)
{
  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              center[0], center[1], center[2], SwcTreeNode::radius(prevNode));
        command  = new ZStackDocCommand::SwcEdit::ExtendSwcNode(this, tn, prevNode);
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeSwcNodeExtendCommand(const ZPoint &center, double radius)
{
  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              center[0], center[1], center[2], radius);
        command  = new ZStackDocCommand::SwcEdit::ExtendSwcNode(this, tn, prevNode);
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeSwcNodeSmartExtendCommand(const ZPoint &center)
{
//  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      return executeSwcNodeSmartExtendCommand(
            center, SwcTreeNode::radius(prevNode));
    }
  }
#if 0
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        ZNeuronTracer tracer;
        tracer.setIntensityField(stack()->c_stack());
        tracer.setTraceWorkspace(m_traceWorkspace);
        if (m_traceWorkspace->trace_mask == NULL) {
          m_traceWorkspace->trace_mask =
              C_Stack::make(GREY, stack()->width(), stack()->height(),
                            stack()->depth());
        }

        Swc_Tree *branch = tracer.trace(
              SwcTreeNode::x(prevNode), SwcTreeNode::y(prevNode),
              SwcTreeNode::z(prevNode), SwcTreeNode::radius(prevNode),
              center.x(), center.y(), center.z(),
              SwcTreeNode::radius(prevNode));
        if (branch != NULL) {
          if (Swc_Tree_Has_Branch(branch)) {
            //tracer.updateMask(branch);
            Swc_Tree_Node *root = Swc_Tree_Regular_Root(branch);
            Swc_Tree_Node *begin = SwcTreeNode::firstChild(root);
            SwcTreeNode::detachParent(begin);
            Kill_Swc_Tree(branch);

            Swc_Tree_Node *leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
            }
            ZSwcPath path(begin, leaf);

            command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
            new ZStackDocCommand::SwcEdit::AddSwcNode(this, begin, command);
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, begin, prevNode, command);
            new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
            /*
            new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(
                  this, path, command);
                  */
          }
        }
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }
#endif

  return false;
}


bool ZStackDoc::executeSwcNodeSmartExtendCommand(
    const ZPoint &center, double radius)
{
  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        ZNeuronTracer tracer;
        tracer.setBackgroundType(getStackBackground());
        tracer.setIntensityField(stack()->c_stack());
        tracer.setTraceWorkspace(m_traceWorkspace);
        if (m_traceWorkspace->trace_mask == NULL) {
          m_traceWorkspace->trace_mask =
              C_Stack::make(GREY, stack()->width(), stack()->height(),
                            stack()->depth());
        }
        if (GET_APPLICATION_NAME == "Biocytin") {
          tracer.setResolution(1, 1, 10);
        }

        Swc_Tree *branch = tracer.trace(
              SwcTreeNode::x(prevNode), SwcTreeNode::y(prevNode),
              SwcTreeNode::z(prevNode), SwcTreeNode::radius(prevNode),
              center.x(), center.y(), center.z(), radius);
        if (branch != NULL) {
          if (Swc_Tree_Has_Branch(branch)) {
            //tracer.updateMask(branch);
            Swc_Tree_Node *root = Swc_Tree_Regular_Root(branch);
            Swc_Tree_Node *begin = SwcTreeNode::firstChild(root);
            SwcTreeNode::detachParent(begin);
            Kill_Swc_Tree(branch);

            Swc_Tree_Node *leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
            }
            ZSwcPath path(begin, leaf);

            command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
            new ZStackDocCommand::SwcEdit::AddSwcNode(this, begin, command);
            std::set<Swc_Tree_Node*> nodeSet;
            nodeSet.insert(leaf);
            new ZStackDocCommand::SwcEdit::SetSwcNodeSeletion(
                  this, nodeSet, command);
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, begin, prevNode, command);
            new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
            /*
            new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(
                  this, path, command);
                  */
          }
        }
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeInterpolateSwcZCommand()
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               m_selectedSwcTreeNodes.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               m_selectedSwcTreeNodes.count(downEnd) == 1) { /* continuation and selected*/
          downEnd = SwcTreeNode::firstChild(downEnd);
        }

        double dist1 = SwcTreeNode::planePathLength(*iter, upEnd);
        double dist2 = SwcTreeNode::planePathLength(*iter, downEnd);

        double z = SwcTreeNode::z(*iter);
        if (dist1 == 0.0 && dist2 == 0.0) {
          z = SwcTreeNode::z(upEnd);
        } else {
          double lambda = dist1 / (dist1 + dist2);
          z = SwcTreeNode::z(upEnd) * (1.0 - lambda) +
              SwcTreeNode::z(downEnd) * lambda;
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNodeZ(
              this, *iter, z, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Z Interpolation"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    return true;
  }

  return false;
}

bool ZStackDoc::executeSwcNodeChangeZCommand(double z)
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (*iter != NULL && SwcTreeNode::z(*iter) != z) {
        new ZStackDocCommand::SwcEdit::ChangeSwcNodeZ(
              this, *iter, z, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Change Z of Selected Swc Node"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    return true;
  }

  return false;
}

bool ZStackDoc::executeMoveSwcNodeCommand(double dx, double dy, double dz)
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (*iter != NULL && (dx != 0 || dy != 0 || dz != 0)) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::translate(&newNode, dx, dy, dz);
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Move Selected Swc Node"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    return true;
  }

  return false;
}

bool ZStackDoc::executeTranslateSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

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

      double sx = dlg.getScaleValue(SwcSkeletonTransformDialog::X);
      double sy = dlg.getScaleValue(SwcSkeletonTransformDialog::Y);
      double sz = dlg.getScaleValue(SwcSkeletonTransformDialog::Z);

      ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        Swc_Tree_Node newNode = *(*iter);
        if (dlg.isTranslateFirst()) {
          SwcTreeNode::translate(&newNode, dx, dy, dz);
        }

        SwcTreeNode::setPos(&newNode, SwcTreeNode::x(&newNode) * sx,
                            SwcTreeNode::y(&newNode) * sy,
                            SwcTreeNode::z(&newNode) * sz);
        if (!dlg.isTranslateFirst()) {
          SwcTreeNode::translate(&newNode, dx, dy, dz);
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }
      pushUndoCommand(allCommand);
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeChangeSelectedSwcNodeSize()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::changeRadius(&newNode, dlg.getAddValue(), dlg.getMulValue());
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }

      pushUndoCommand(allCommand);
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeSwcNodeChangeSizeCommand(double dr)
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (dr != 0) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::changeRadius(&newNode, dr, 1.0);
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Swc Node - Change Size"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    return true;
  }

  return false;
}

void ZStackDoc::estimateSwcRadius(ZSwcTree *tree)
{
  if (tree != NULL) {
    startProgress();
    int count = tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    double step = 1.0 / count;
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      if (SwcTreeNode::isRegular(tn)) {
        SwcTreeNode::fitSignal(tn, stack()->c_stack(), getStackBackground());
      }
      advanceProgress(step);
    }
    endProgress();
  }
}

void ZStackDoc::estimateSwcRadius()
{
  foreach (ZSwcTree *tree, m_swcList){
    estimateSwcRadius(tree);
  }
}

bool ZStackDoc::executeSwcNodeEstimateRadiusCommand()
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    startProgress();
    double step = 1.0 / m_selectedSwcTreeNodes.size();
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      Swc_Tree_Node newNode = *(*iter);
      if (SwcTreeNode::fitSignal(&newNode,  stack()->c_stack(),
                                 NeuTube::IMAGE_BACKGROUND_BRIGHT)) {
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
        advanceProgress(step);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Swc Node - Estimate Radius"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    endProgress();

    return true;
  }

  return false;
}

bool ZStackDoc::executeMergeSwcNodeCommand()
{
  if (selectedSwcTreeNodes()->size() > 1) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::MergeSwcNode(this);
    pushUndoCommand(command);
    deprecateTraceMask();

    return true;
  }

  return false;
}

bool ZStackDoc::executeSetRootCommand()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    QUndoCommand *command =
        new ZStackDocCommand::SwcEdit::SetRoot(this, *nodeSet->begin());
    pushUndoCommand(command);
    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveTurnCommand()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    Swc_Tree_Node *tn = *(nodeSet->begin());
    if (SwcTreeNode::isContinuation(tn)) {
      if (SwcTreeNode::isTurn(SwcTreeNode::firstChild(tn), tn,
                              SwcTreeNode::parent(tn))) {
        ZStackDocCommand::SwcEdit::CompositeCommand *command =
                new ZStackDocCommand::SwcEdit::CompositeCommand(this);

        new ZStackDocCommand::SwcEdit::SetParent(
              this, SwcTreeNode::firstChild(tn), SwcTreeNode::parent(tn),
              command);
        new ZStackDocCommand::SwcEdit::DeleteSwcNode(
              this, tn, SwcTreeNode::root(tn), command);
        pushUndoCommand(command);

        deprecateTraceMask();

        return true;
      }
    } else {
      std::vector<Swc_Tree_Node*> neighborArray =
          SwcTreeNode::neighborArray(tn);
      double minDot = 0.0;
      Swc_Tree_Node *tn1 = NULL;
      Swc_Tree_Node *tn2 = NULL;
      for (size_t i = 0; i < neighborArray.size(); ++i) {
        for (size_t j = 0; j < neighborArray.size(); ++j) {
          if (i != j) {
            double dot = Swc_Tree_Node_Dot(neighborArray[i], tn, neighborArray[j]);
            if (dot < minDot) {
              minDot = dot;
              tn1 = neighborArray[i];
              tn2 = neighborArray[j];
            }
          }
        }
      }

      if (tn1 != NULL && tn2 != NULL) {
        double x, y, z, r;
        SwcTreeNode::interpolate(tn1, tn2, 0.5, &x, &y, &z, &r);
        QUndoCommand *command =
            new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
              this, tn, x, y, z, r);
        pushUndoCommand(command);
        deprecateTraceMask();
        return true;
      }
    }
  }

  return false;
}

bool ZStackDoc::executeResolveCrossoverCommand()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    Swc_Tree_Node *center = *(nodeSet->begin());
    std::map<Swc_Tree_Node*, Swc_Tree_Node*> matched =
        SwcTreeNode::crossoverMatch(center, TZ_PI_2);
    if (!matched.empty()) {
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      Swc_Tree_Node *root = SwcTreeNode::root(center);
      for (std::map<Swc_Tree_Node*, Swc_Tree_Node*>::const_iterator
           iter = matched.begin(); iter != matched.end(); ++iter) {
        if (SwcTreeNode::parent(iter->first) == center &&
            SwcTreeNode::parent(iter->second) == center) {
          new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->first, iter->second, command);
          new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->second, root, command);
        } else {
          new ZStackDocCommand::SwcEdit::SetParent(this, center, root, command);
          if (SwcTreeNode::parent(iter->first) == center) {
            new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->first, iter->second, command);
          } else {
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, iter->second, iter->first, command);
          }
        }

        if (matched.size() * 2 == SwcTreeNode::neighborArray(center).size()) {
          new ZStackDocCommand::SwcEdit::DeleteSwcNode(
                this, center, root, command);
        }
      }
      pushUndoCommand(command);
      deprecateTraceMask();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeWatershedCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::StackProcess::Watershed(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeBinarizeCommand(int thre)
{
  if (hasStackData()) {
    QUndoCommand *command =
        new ZStackDocCommand::StackProcess::Binarize(this, thre);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeBwsolidCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::StackProcess::BwSolid(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeEnhanceLineCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::StackProcess::EnhanceLine(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeDeleteSwcNodeCommand()
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (*iter != NULL) {
        new ZStackDocCommand::SwcEdit::DeleteSwcNode(
              this, *iter, SwcTreeNode::root(*iter), allCommand);
      }
    }
    new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, allCommand);

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Delete Selected Swc Node"));
      blockSignals(true);
      pushUndoCommand(allCommand);
#ifdef _DEBUG_2
      m_swcList[0]->print();
#endif
      m_selectedSwcTreeNodes.clear();
      blockSignals(false);
      notifySwcModified();
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    return true;
  }

  return false;
}

bool ZStackDoc::executeConnectSwcNodeCommand()
{
  if (m_selectedSwcTreeNodes.size() > 1) {
    QUndoCommand *command =  new ZStackDocCommand::SwcEdit::ConnectSwcNode(this);
    pushUndoCommand(command);
    deprecateTraceMask();
  }
  /*
  std::set<SwcTreeNode> swcNodeSet;
  for (std::set<SwcTreeNode*>::const_iterator iter = m_selectedSwcTreeNodes.begin();
       iter != m_selectedSwcTreeNodes.end(); ++iter) {
    swcNodeSet.insert(m_selectedSwcTreeNodes.end(), *(*iter));
  }

  if (SwcTreeNode::connect(m_selectedSwcTreeNodes)) {
    QUndoCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if ()
      new ZStackDocCommand::SwcEdit::SetParent()
    }
  }
  */

  return false;
}

bool ZStackDoc::executeConnectSwcNodeCommand(Swc_Tree_Node *tn)
{
  if (!selectedSwcTreeNodes()->empty()) {
    Swc_Tree_Node *target = SwcTreeNode::findClosestNode(
          *selectedSwcTreeNodes(), tn);
    return executeConnectSwcNodeCommand(target, tn);
  }

  return false;
}

bool ZStackDoc::executeConnectSwcNodeCommand(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  if (!SwcTreeNode::isRegular(tn1) || !SwcTreeNode::isRegular(tn2)) {
    return false;
  }

  if (SwcTreeNode::isRegular(SwcTreeNode::commonAncestor(tn1, tn2))) {
    return false;
  }

  QUndoCommand *command =
      new ZStackDocCommand::SwcEdit::CompositeCommand(this);
  new ZStackDocCommand::SwcEdit::SetRoot(this, tn2, command);
  new ZStackDocCommand::SwcEdit::SetParent(this, tn2, tn1, command);
  new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);

  pushUndoCommand(command);
  deprecateTraceMask();

  notifySwcModified();

  return true;
}

bool ZStackDoc::executeBreakSwcConnectionCommand()
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (m_selectedSwcTreeNodes.count(SwcTreeNode::parent(*iter)) > 0) {
        new ZStackDocCommand::SwcEdit::SetParent(
              this, *iter, SwcTreeNode::root(*iter), allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Break Connection"));
      blockSignals(true);
      pushUndoCommand(allCommand);
#ifdef _DEBUG_2
      m_swcList[0]->print();
#endif
      blockSignals(false);
      notifySwcModified();
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    return true;
  }

  return false;
}

bool ZStackDoc::executeBreakForestCommand()
{
  if (!m_selectedSwcs.empty()) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::BreakForest(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeGroupSwcCommand()
{
  if (m_selectedSwcs.size() > 1) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::GroupSwc(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeAddSwcCommand(ZSwcTree *tree)
{
  if (tree != NULL) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::AddSwc(this, tree);
    pushUndoCommand(command);
    deprecateTraceMask();
  }

  return false;
}

bool ZStackDoc::executeAddSwcNodeCommand(const ZPoint &center, double radius)
{
  if (radius > 0) {
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(center, radius);
    ZStackDocCommand::SwcEdit::AddSwcNode *command = new
        ZStackDocCommand::SwcEdit::AddSwcNode(this, tn);
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeAddStrokeCommand(ZStroke2d *stroke)
{
  if (stroke != NULL) {
    QUndoCommand *command =
        new ZStackDocCommand::StrokeEdit::AddStroke(this, stroke);
    if (!stroke->isEmpty()) {
      pushUndoCommand(command);
      return true;
    } else {
      delete command;
    }
  }

  return false;
}

void ZStackDoc::addObject(ZDocumentable *obj, NeuTube::EDocumentableType type)
{
  switch (type) {
  case NeuTube::Documentable_SWC:
    addSwcTree(dynamic_cast<ZSwcTree*>(obj), false);
    break;
  case NeuTube::Documentable_PUNCTUM:
    addPunctum(dynamic_cast<ZPunctum*>(obj));
    break;
  case NeuTube::Documentable_OBJ3D:
    addObj3d(dynamic_cast<ZObject3d*>(obj));
    break;
  case NeuTube::Documentable_LOCSEG_CHAIN:
    addLocsegChain(dynamic_cast<ZLocsegChain*>(obj));
    break;
  case NeuTube::Documentable_CONN:
    addLocsegChainConn(dynamic_cast<ZLocsegChainConn*>(obj));
    break;
  case NeuTube::Documentable_STROKE:
    addStroke(dynamic_cast<ZStroke2d*>(obj));
    break;
  }
}

bool ZStackDoc::hasObjectSelected()
{
  return !(m_selectedPuncta.empty() && m_selectedChains.empty() &&
           m_selectedSwcs.empty());
}

bool ZStackDoc::executeAddObjectCommand(
    ZDocumentable *obj, NeuTube::EDocumentableType type)
{
  if (obj != NULL) {
    ZStackDocCommand::ObjectEdit::AddObject *command =
        new ZStackDocCommand::ObjectEdit::AddObject(this, obj, type);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveObjectCommand()
{
  if (hasObjectSelected()) {
    ZStackDocCommand::ObjectEdit::RemoveSelected *command = new
        ZStackDocCommand::ObjectEdit::RemoveSelected(this);
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}
/*
bool ZStackDoc::executeRemoveUnselectedObjectCommand()
{

  return false;
}
*/
bool ZStackDoc::executeMoveObjectCommand(double x, double y, double z,
    double punctaScaleX, double punctaScaleY, double punctaScaleZ,
    double swcScaleX, double swcScaleY, double swcScaleZ)
{
  if (selectedSwcs()->empty() && selectedPuncta()->empty() &&
      selectedSwcTreeNodes()->empty())
    return false;

  ZStackDocCommand::ObjectEdit::MoveSelected *moveSelectedObjectCommand =
      new ZStackDocCommand::ObjectEdit::MoveSelected(this, x, y, z);
  moveSelectedObjectCommand->setPunctaCoordScale(punctaScaleX,
                                                 punctaScaleY,
                                                 punctaScaleZ);
  moveSelectedObjectCommand->setSwcCoordScale(swcScaleX,
                                              swcScaleY,
                                              swcScaleZ);
  pushUndoCommand(moveSelectedObjectCommand);

  return true;
}

bool ZStackDoc::executeTraceTubeCommand(double x, double y, double z, int c)
{
  QUndoCommand *traceTubeCommand =
      new ZStackDocCommand::TubeEdit::Trace(this, x, y, z, c);
  pushUndoCommand(traceTubeCommand);

  return true;
}

bool ZStackDoc::executeTraceSwcBranchCommand(
    double x, double y, double z, int c)
{
  /*
  QUndoCommand *command =
      new ZStackDocCommand::SwcEdit::TraceSwcBranch(this, x, y, z, c);
  pushUndoCommand(command);
  */

  ZNeuronTracer tracer;
  tracer.setIntensityField(stack()->c_stack(c));
  tracer.setTraceWorkspace(getTraceWorkspace());
  refreshTraceMask();
  ZSwcPath branch = tracer.trace(x, y, z);

  if (branch.size() > 1) {
    ZSwcConnector swcConnector;

    std::pair<Swc_Tree_Node*, Swc_Tree_Node*> conn =
        swcConnector.identifyConnection(branch, getSwcArray());

    Swc_Tree_Node *branchRoot = branch.front();
    bool needAdjust = false;
    if (conn.first != NULL) {
      if (!SwcTreeNode::isRoot(conn.first)) {
        SwcTreeNode::setAsRoot(conn.first);
        branchRoot = conn.first;
      }

      if (SwcTreeNode::hasOverlap(conn.first, conn.second)) {
        needAdjust = true;
      } else {
        if (SwcTreeNode::isTurn(conn.second, conn.first,
                                SwcTreeNode::firstChild(conn.first))) {
          needAdjust = true;
        }
      }
    } else {
      if (SwcTreeNode::isRegular(SwcTreeNode::firstChild(branchRoot))) {
        Swc_Tree_Node *rootNeighbor = SwcTreeNode::firstChild(branchRoot);
        ZPoint rootCenter = SwcTreeNode::pos(branchRoot);
        ZPoint nbrCenter = SwcTreeNode::pos(rootNeighbor);
        double rootIntensity = Stack_Point_Sampling(
              stack()->c_stack(), rootCenter.x(), rootCenter.y(), rootCenter.z());
        if (rootIntensity == 0.0) {
          needAdjust = true;
        } else {
          double nbrIntensity = Stack_Point_Sampling(
                stack()->c_stack(), nbrCenter.x(), nbrCenter.y(), nbrCenter.z());
          if (nbrIntensity / rootIntensity >= 3.0) {
            needAdjust = true;
          }
        }
      }
    }

    if (needAdjust) {
      SwcTreeNode::average(branchRoot, SwcTreeNode::firstChild(branchRoot),
                           branchRoot);
    }

    Swc_Tree_Node *loop = conn.second;
    Swc_Tree_Node *hook = conn.first;

    if (hook != NULL) {
      //Adjust the branch point
      std::vector<Swc_Tree_Node*> neighborArray =
          SwcTreeNode::neighborArray(loop);
      for (std::vector<Swc_Tree_Node*>::iterator iter = neighborArray.begin();
           iter != neighborArray.end(); ++iter) {
        Swc_Tree_Node *tn = *iter;
        if (SwcTreeNode::hasSignificantOverlap(tn, hook)) {
          loop = tn;
          Swc_Tree_Node *newHook = hook;
          newHook = SwcTreeNode::firstChild(hook);
          SwcTreeNode::detachParent(newHook);
          SwcTreeNode::kill(hook);
          hook = newHook;
          branchRoot = hook;
        }
      }
    }


    ZSwcTree *tree = new ZSwcTree();
    tree->setDataFromNode(branchRoot);

    if (SwcTreeNode::isRegular(SwcTreeNode::firstChild(branchRoot))) {
      Swc_Tree_Node *terminal = tree->firstLeaf();
      Swc_Tree_Node *terminalNeighbor = SwcTreeNode::parent(tree->firstLeaf());
      ZPoint terminalCenter = SwcTreeNode::pos(terminal);
      ZPoint nbrCenter = SwcTreeNode::pos(terminalNeighbor);
      double terminalIntensity = Stack_Point_Sampling(
            stack()->c_stack(), terminalCenter.x(), terminalCenter.y(), terminalCenter.z());
      if (terminalIntensity == 0.0) {
        SwcTreeNode::average(terminal, terminalNeighbor, terminal);
      } else {
        double nbrIntensity = Stack_Point_Sampling(
              stack()->c_stack(), nbrCenter.x(), nbrCenter.y(), nbrCenter.z());
        if (nbrIntensity / terminalIntensity >= 3.0) {
          SwcTreeNode::average(terminal, terminalNeighbor, terminal);
        }
      }
    }

    QUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);

    ZSwcPath path(branchRoot, tree->firstLeaf());

    new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);

    if (conn.first != NULL) {
      new ZStackDocCommand::SwcEdit::SetParent(
            this, hook, loop, command);
      new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
    }

    new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(this, path, command);

    pushUndoCommand(command);

    //tracer.updateMask(branch);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveTubeCommand()
{
  if (!selectedChains()->empty()) {
    QUndoCommand *command =
        new ZStackDocCommand::TubeEdit::RemoveSelected(this);
    pushUndoCommand(command);
    return true;
  }

  return false;
}

bool ZStackDoc::executeAutoTraceCommand()
{
#if 0
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::TubeEdit::AutoTrace(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
#endif

  autoTrace();
  Swc_Tree *rawTree = this->swcReconstruction(0, false, true);
  removeAllLocsegChain();
  Zero_Stack(m_traceWorkspace->trace_mask);
  if (rawTree != NULL) {
    ZSwcTree *tree = new ZSwcTree;
    tree->setData(rawTree);
    //QUndoCommand *command = new ZStackDocCommand::SwcEdit::AddSwc(this, tree);
    ZStackDocCommand::SwcEdit::CompositeCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);
    new ZStackDocCommand::SwcEdit::SwcTreeLabeTraceMask(this, rawTree, command);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeAutoTraceAxonCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::TubeEdit::AutoTraceAxon(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

void ZStackDoc::saveSwc(QWidget *parentWidget)
{
  //Assume there is no empty tree
  if (!m_swcList.empty()) {
    if (m_swcList.size() > 1) {
      report("Save failed", "More than one SWC tree exist.",
             ZMessageReporter::Error);
    } else {
      ZSwcTree *tree = m_swcList[0];
      if (tree->hasGoodSourceName()) {
        tree->resortId();
        tree->save(tree->source().c_str());
      } else {
        ZString stackSource = stackSourcePath().toStdString();
        QString fileName;
        if (!stackSource.empty()) {
          fileName = stackSource.changeExt("Edit.swc").c_str();
        }

        if (fileName.isEmpty()) {
          fileName = "untitled.swc";
        }

        fileName = QFileDialog::getSaveFileName(
              parentWidget, tr("Save SWC"), fileName, tr("SWC File"), 0);
        if (!fileName.isEmpty()) {
          tree->resortId();
          tree->save(fileName.toStdString().c_str());
          tree->setSource(fileName.toStdString());
          notifySwcModified();
        }
      }
    }
  }
}

std::vector<ZSwcTree*> ZStackDoc::getSwcArray() const
{
  std::vector<ZSwcTree*> swcArray;
  for (QList<ZSwcTree*>::const_iterator iter = m_swcList.begin();
       iter != m_swcList.end(); ++iter) {
    swcArray.push_back(const_cast<ZSwcTree*>(*iter));
  }

  return swcArray;
}

ZStack* ZStackDoc::projectBiocytinStack(
    Biocytin::ZStackProjector &projector)
{
  projector.setProgressReporter(m_progressReporter);

  ZStack *proj = projector.project(stack(), true);

  if (proj != NULL) {
    if (proj->channelNumber() == 2) {
      proj->initChannelColors();
      proj->setChannelColor(0, 1, 1, 1);
      proj->setChannelColor(1, 0, 0, 0);
    }

   // ZString filePath(stack()->sourcePath());
    proj->setSource(projector.getDefaultResultFilePath(stack()->sourcePath()));
#ifdef _DEBUG2
    const vector<int>& depthArray = projector.getDepthArray();
    ZStack depthImage(GREY16, proj->width(), proj->height(), 1, 1);
    uint16_t *array = depthImage.array16();
    size_t index = 0;
    for (int y = 0; y < proj->height(); ++y) {
      for (int x = 0; x < proj->width(); ++x) {
        array[index] = depthArray[index];
        ++index;
      }
    }
    depthImage.save(NeutubeConfig::getInstance().getPath(NeutubeConfig::DATA) +
                    "/test.tif");
#endif
  }

  return proj;
}

void ZStackDoc::selectAllSwcTreeNode()
{
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;
  foreach (ZSwcTree *tree, m_swcList) {
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      if ((m_selectedSwcTreeNodes.insert(tn)).second) {
        selected.push_back(tn);
        // deselect its tree
        setSwcSelected(nodeToSwcTree(tn), false);
      }
    }
  }
  if (!selected.empty() || !deselected.empty()) {
    emit swcTreeNodeSelectionChanged(selected, deselected);
  }
}

bool ZStackDoc::getLastStrokePoint(int *x, int *y) const
{
  if (!m_strokeList.empty()) {
    if (!m_strokeList[0]->isEmpty()) {
      return m_strokeList[0]->getLastPoint(x, y);
    }
  }

  return false;
}

bool ZStackDoc::hasSelectedSwc() const
{
  return !selectedSwcs()->empty();
}

bool ZStackDoc::hasSelectedSwcNode() const
{
  return !selectedSwcTreeNodes()->empty();
}

bool ZStackDoc::hasMultipleSelectedSwcNode() const
{
  return selectedSwcTreeNodes()->size() > 1;
}

void ZStackDoc::updateModelData(EDocumentDataType type)
{
  switch (type) {
  case SWC_DATA:
    swcObjsModel()->updateModelData();
    break;
  case PUNCTA_DATA:
    punctaObjsModel()->updateModelData();
    break;
  default:
    break;
  }
}

void ZStackDoc::showSeletedSwcNodeLength()
{
  double length = SwcTreeNode::segmentLength(*selectedSwcTreeNodes());

  InformationDialog dlg;

  std::ostringstream textStream;

  textStream << "<p>Overall length of selected branches: " << length << "</p>";

  if (selectedSwcTreeNodes()->size() == 2) {
    std::set<Swc_Tree_Node*>::const_iterator iter =
        selectedSwcTreeNodes()->begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    textStream << "<p>Straight line distance between the two selected nodes: "
               << SwcTreeNode::distance(tn1, tn2) << "</p>";
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

bool ZStackDoc::executeInsertSwcNode()
{
  if (selectedSwcTreeNodes()->size() >= 2) {
    QUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = selectedSwcTreeNodes()->begin();
         iter != selectedSwcTreeNodes()->end(); ++iter) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(*iter);
      if (selectedSwcTreeNodes()->count(parent) > 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer();
        SwcTreeNode::interpolate(*iter, parent, 0.5, tn);
        new ZStackDocCommand::SwcEdit::SetParent(this, tn, parent, command);
        new ZStackDocCommand::SwcEdit::SetParent(this, *iter, tn, command);
      }
    }
    if (command->childCount() > 0) {
      pushUndoCommand(command);
      deprecateTraceMask();
      return true;
    } else {
      delete command;
    }
  }

  return false;
}

bool ZStackDoc::executeSetBranchPoint()
{
  if (selectedSwcTreeNodes()->size() == 1) {
    Swc_Tree_Node *branchPoint = *(selectedSwcTreeNodes()->begin());
    Swc_Tree_Node *hostRoot = SwcTreeNode::regularRoot(branchPoint);
    Swc_Tree_Node *masterRoot = SwcTreeNode::parent(hostRoot);
    if (SwcTreeNode::childNumber(masterRoot) > 1) {
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      ZSwcTree tree;
      tree.setDataFromNode(masterRoot);

      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
      bool isConnected = false;
      double minDist = Infinity;
      Swc_Tree_Node *closestNode = NULL;
      for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
        if (SwcTreeNode::isRegular(tn)) {
          if (SwcTreeNode::isRoot(tn)) {
            if (tn == hostRoot) {
              isConnected = false;
            } else {
              isConnected = true;
            }
          }

          if (isConnected) {
            double dist = SwcTreeNode::distance(
                  tn, branchPoint, SwcTreeNode::EUCLIDEAN_SURFACE);
            if (dist < minDist) {
              minDist = dist;
              closestNode = tn;
            }
          }
        }
      }
      tree.setDataFromNode(NULL, ZSwcTree::LEAVE_ALONE);

      if (!SwcTreeNode::isRoot(closestNode)) {
        new ZStackDocCommand::SwcEdit::SetRoot(this, closestNode, command);
      }
      new ZStackDocCommand::SwcEdit::SetParent(
            this, closestNode, branchPoint, command);

      pushUndoCommand(command);
      deprecateTraceMask();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeConnectIsolatedSwc()
{
  if (selectedSwcTreeNodes()->size() == 1) {
    Swc_Tree_Node *branchPoint = *(selectedSwcTreeNodes()->begin());
    Swc_Tree_Node *hostRoot = SwcTreeNode::regularRoot(branchPoint);
    Swc_Tree_Node *masterRoot = SwcTreeNode::parent(hostRoot);

    if (SwcTreeNode::childNumber(masterRoot) > 1 || swcList()->size() > 1) {
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      ZSwcTree tree;
      tree.setDataFromNode(masterRoot);

      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
      bool isConnected = false;
      double minDist = Infinity;
      Swc_Tree_Node *closestNode = NULL;
      for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
        if (SwcTreeNode::isRegular(tn)) {
          if (SwcTreeNode::isRoot(tn)) {
            if (tn == hostRoot) {
              isConnected = false;
            } else {
              isConnected = true;
            }
          }

          if (isConnected) {
            double dist = SwcTreeNode::distance(
                  tn, branchPoint, SwcTreeNode::EUCLIDEAN_SURFACE);
            if (dist < minDist) {
              minDist = dist;
              closestNode = tn;
            }
          }
        }
      }
      tree.setDataFromNode(NULL, ZSwcTree::LEAVE_ALONE);

      foreach (ZSwcTree *buddyTree, *swcList()) {
        if (buddyTree->root() != masterRoot) {
          Swc_Tree_Node *tn = NULL;
          double dist = buddyTree->distanceTo(branchPoint, &tn);
          if (dist < minDist) {
            minDist = dist;
            closestNode = tn;
          }
        }
      }

      if (closestNode != NULL) {
        if (!SwcTreeNode::isRoot(closestNode)) {
          new ZStackDocCommand::SwcEdit::SetRoot(this, closestNode, command);
        }
        new ZStackDocCommand::SwcEdit::SetParent(
              this, closestNode, branchPoint, command);
        new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
      }

      pushUndoCommand(command);
      deprecateTraceMask();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeResetBranchPoint()
{
  if (selectedSwcTreeNodes()->size() == 1) {
    Swc_Tree_Node *loop = *(selectedSwcTreeNodes()->begin());
    std::vector<Swc_Tree_Node*> neighborArray = SwcTreeNode::neighborArray(loop);
    for (std::vector<Swc_Tree_Node*>::iterator iter = neighborArray.begin();
         iter != neighborArray.end(); ++iter) {
      Swc_Tree_Node *tn = *iter;
      if (SwcTreeNode::isBranchPoint(tn)) {
        std::vector<Swc_Tree_Node*> candidateHookArray =
            SwcTreeNode::neighborArray(tn);
        Swc_Tree_Node *hook = NULL;
        double minDot = Infinity;
        for (std::vector<Swc_Tree_Node*>::iterator iter = candidateHookArray.begin();
             iter != candidateHookArray.end(); ++iter) {
          Swc_Tree_Node *hookCandidate = *iter;
          if (hookCandidate != loop && SwcTreeNode::isRegular(hookCandidate)) {
            double dot = SwcTreeNode::normalizedDot(hookCandidate, tn, loop);
            if (dot < minDot) {
              minDot = dot;
              hook = hookCandidate;
            }
          }
        }

        if (hook != NULL) {
          QUndoCommand *command =
              new ZStackDocCommand::SwcEdit::CompositeCommand(this);
          new ZStackDocCommand::SwcEdit::SetParent(this, hook, NULL, command);
          if (SwcTreeNode::parent(hook) != tn) {
            new ZStackDocCommand::SwcEdit::SetRoot(this, hook, command);
          }

          new ZStackDocCommand::SwcEdit::SetParent(this, hook, loop, command);
          pushUndoCommand(command);
          deprecateTraceMask();
        }
        break;
      }
    }
  }

  return false;
}
