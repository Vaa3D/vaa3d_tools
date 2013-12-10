#include "z3dgl.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

#include "zstackpresenter.h"
#include "zstackframe.h"
#include "zstackview.h"
#include "tz_rastergeom.h"
#include "zlocalneuroseg.h"
#include "zlocsegchain.h"
#include "zimagewidget.h"
#include "zstack.hxx"
#include "zstackdoc.h"
#include "swctreenode.h"
#include "channeldialog.h"
#include "neutubeconfig.h"
#include "zcursorstore.h"
#include "zstroke2d.h"
#include "tz_geo3d_utils.h"

ZStackPresenter::ZStackPresenter(ZStackFrame *parent) : QObject(parent),
  m_parent(parent),
  //m_zoomRatio(1),
  m_showObject(true),
  m_isStrokeOn(false)
{
  initInteractiveContext();

  m_greyScale.resize(1);
  m_greyScale[0] = 1.0;
  m_greyOffset.resize(1);
  m_greyOffset[0] = 0.0;

  m_objStyle = ZStackDrawable::NORMAL;
  m_threshold = -1;
  m_mouseLeftButtonPressed = false;
  m_mouseRightButtonPressed = false;

  for (int i = 0; i < 3; i++) {
    m_mouseLeftReleasePosition[i] = -1;
    m_mouseRightReleasePosition[i] = -1;
    m_mouseLeftPressPosition[i] = -1;
    m_mouseRightPressPosition[i] = -1;
    m_mouseLeftDoubleClickPosition[i] = -1;
    m_mouseMovePosition[i] = -1;
  }

  m_cursorRadius = 10;

  m_activeDecorationList.push_back(&m_stroke);

  createActions();
}

void ZStackPresenter::initInteractiveContext()
{
  if (NeutubeConfig::getInstance().getMainWindowConfig().isTracingOff()) {
    m_interactiveContext.setTraceMode(ZInteractiveContext::TRACE_OFF);
  } else {
    m_interactiveContext.setTraceMode(ZInteractiveContext::TRACE_TUBE);
  }

  m_interactiveContext.setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
}

void ZStackPresenter::createTraceActions()
{
  m_traceAction = new QAction(tr("trace"), this);
  connect(m_traceAction, SIGNAL(triggered()), this, SLOT(traceTube()));

  m_fitsegAction = new QAction(tr("fit"), this);
  connect(m_fitsegAction, SIGNAL(triggered()), this, SLOT(fitSegment()));

  m_dropsegAction = new QAction(tr("drop"), this);
  connect(m_dropsegAction, SIGNAL(triggered()), this, SLOT(dropSegment()));
}

void ZStackPresenter::createPunctaActions()
{
  m_markPunctaAction = new QAction(tr("mark Puncta"), this);
  connect(m_markPunctaAction, SIGNAL(triggered()), this, SLOT(markPuncta()));

  m_deleteAllPunctaAction = new QAction(tr("Delete All Puncta"), this);
  connect(m_deleteAllPunctaAction, SIGNAL(triggered()), this, SLOT(deleteAllPuncta()));

  m_enlargePunctaAction = new QAction(tr("Enlarge Puncta"), this);
  connect(m_enlargePunctaAction, SIGNAL(triggered()), this, SLOT(enlargePuncta()));

  m_narrowPunctaAction = new QAction(tr("Narrow Puncta"), this);
  connect(m_narrowPunctaAction, SIGNAL(triggered()), this, SLOT(narrowPuncta()));

  m_meanshiftPunctaAction = new QAction(tr("Mean Shift Puncta"), this);
  connect(m_meanshiftPunctaAction, SIGNAL(triggered()), this, SLOT(meanshiftPuncta()));

  m_meanshiftAllPunctaAction = new QAction(tr("Mean Shift All Puncta"), this);
  connect(m_meanshiftAllPunctaAction, SIGNAL(triggered()), this, SLOT(meanshiftAllPuncta()));
}

void ZStackPresenter::createTubeActions()
{
  m_hookAction = new QAction(tr("Straight hook"), this);
  m_hookAction->setIcon(QIcon(":/images/hook.png"));
  connect(m_hookAction, SIGNAL(triggered()), this, SLOT(enterHookMode()));

  m_spHookAction = new QAction(tr("Hook"), this);
  m_spHookAction->setIcon(QIcon(":/images/hook.png"));
  connect(m_spHookAction, SIGNAL(triggered()), this, SLOT(enterSpHookMode()));

  m_linkAction = new QAction(tr("Link"), this);
  m_linkAction->setIcon(QIcon(":/images/link.png"));
  connect(m_linkAction, SIGNAL(triggered()), this, SLOT(enterLinkMode()));

  m_cutAction = new QAction(tr("Cut"), this);
  m_cutAction->setIcon(QIcon(":/images/cut.png"));
  connect(m_cutAction, SIGNAL(triggered()), this, SLOT(cutTube()));

  m_breakAction = new QAction(tr("Break"), this);
  connect(m_breakAction, SIGNAL(triggered()), this, SLOT(breakTube()));

  m_mergeAction = new QAction(tr("Merge"), this);
  m_mergeAction->setIcon(QIcon(":/images/merge.png"));
  connect(m_mergeAction, SIGNAL(triggered()), this, SLOT(enterMergeMode()));

  m_walkAction = new QAction(tr("Walk to"), this);
  connect(m_walkAction, SIGNAL(triggered()), this, SLOT(enterWalkMode()));

  m_checkConnAction = new QAction(tr("Check connection"), this);
  connect(m_checkConnAction, SIGNAL(triggered()), this, SLOT(enterCheckConnMode()));

  m_connectAction = new QAction(tr("Connect to"), this);
  connect(m_connectAction, SIGNAL(triggered()), this, SLOT(enterConnectMode()));

  m_extendAction = new QAction(tr("Extend"), this);
  m_extendAction->setIcon(QIcon(":/images/extend.png"));
  connect(m_extendAction, SIGNAL(triggered()), this, SLOT(enterExtendMode()));

  m_disconnectAction = new QAction(tr("Disconnect from"), this);
  connect(m_disconnectAction, SIGNAL(triggered()),
          this, SLOT(enterDisconnectMode()));

  m_neighborAction = new QAction(tr("Neighbors"), this);
  connect(m_neighborAction, SIGNAL(triggered()),
          this, SLOT(selectNeighbor()));

  m_selectConnectedTubeAction = new QAction(tr("Connected"), this);
  connect(m_selectConnectedTubeAction, SIGNAL(triggered()),
          this, SLOT(selectConnectedTube()));

  m_refineEndAction = new QAction(tr("Refine end"), this);
  connect(m_refineEndAction, SIGNAL(triggered()),
          this, SLOT(refineChainEnd()));
}

//Doesn't work while connecting doc slots directly for unknown reason
void ZStackPresenter::createDocDependentActions()
{
  assert(buddyDocument());
  //  m_swcSelectAllNodeAction = new QAction(tr("Select All Nodes"), this);
  //  connect(m_swcSelectAllNodeAction, SIGNAL(triggered()), this,
  //          SLOT(selectAllSwcTreeNode()));
  m_selectSwcNodeDownstreamAction = new QAction("Select Downstream", this);
  connect(m_selectSwcNodeDownstreamAction, SIGNAL(triggered()), this,
          SLOT(selectDownstreamNode()));

  /*
  std::cout << "Connecting m_selectSwcNodeDownstreamAction - slotTest" << std::endl;
  connect(m_selectSwcNodeDownstreamAction, SIGNAL(triggered()), this,
          SLOT(slotTest()));
*/

  m_selectSwcConnectionAction = new QAction("Select Connection", this);
  connect(m_selectSwcConnectionAction, SIGNAL(triggered()), this,
          SLOT(selectSwcNodeConnection()));

  m_selectSwcNodeUpstreamAction = new QAction("Select Upstream", this);
  connect(m_selectSwcNodeUpstreamAction, SIGNAL(triggered()), this,
          SLOT(selectUpstreamNode()));

  m_selectSwcNodeBranchAction = new QAction("Select Branch", this);
  connect(m_selectSwcNodeBranchAction, SIGNAL(triggered()), this,
          SLOT(selectBranchNode()));

  m_selectSwcNodeTreeAction = new QAction("Select Tree", this);
  connect(m_selectSwcNodeTreeAction, SIGNAL(triggered()), this,
          SLOT(selectTreeNode()));

  m_selectAllConnectedSwcNodeAction = new QAction("Select All Connected Nodes", this);
  connect(m_selectAllConnectedSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(selectConnectedNode()));

  m_selectAllSwcNodeAction = new QAction("Select All Nodes", this);
  connect(m_selectAllSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(selectAllSwcTreeNode()));
}

void ZStackPresenter::createSwcActions()
{
  m_swcConnectToAction = new QAction(tr("Connect to"), this);
  connect(m_swcConnectToAction, SIGNAL(triggered()),
          this, SLOT(enterSwcConnectMode()));

  m_swcExtendAction = new QAction(tr("Extend"), this);
  connect(m_swcExtendAction, SIGNAL(triggered()),
          this, SLOT(enterSwcExtendMode()));

  m_swcSmartExtendAction = new QAction(tr("Smart extension"), this);
  connect(m_swcSmartExtendAction, SIGNAL(triggered()),
          this, SLOT(enterSwcSmartExtendMode()));

  m_swcDeleteAction = new  QAction(tr("Delete"), this);
  connect(m_swcDeleteAction, SIGNAL(triggered()),
          this, SLOT(deleteSwcNode()));

  m_swcConnectSelectedAction = new QAction(tr("Connect"), this);
  connect(m_swcConnectSelectedAction, SIGNAL(triggered()),
          this, SLOT(connectSelectedSwcNode()));

  m_swcBreakSelectedAction = new QAction(tr("Break"), this);
  connect(m_swcBreakSelectedAction, SIGNAL(triggered()),
          this, SLOT(breakSelectedSwcNode()));

  m_swcLockFocusAction = new QAction(tr("Lock Focus"), this);
  connect(m_swcLockFocusAction, SIGNAL(triggered()),
          this, SLOT(lockSelectedSwcNodeFocus()));

  m_swcEstimateRadiusAction = new QAction(tr("Estimate Radius"), this);
  connect(m_swcEstimateRadiusAction, SIGNAL(triggered()),
          this, SLOT(estimateSelectedSwcRadius()));
}

void ZStackPresenter::createActions()
{
  m_deleteSelectedAction = new QAction(tr("Delete Selected Object"), this);
  connect(m_deleteSelectedAction, SIGNAL(triggered()), this, SLOT(deleteSelected()));


  m_fitEllipseAction = new QAction(tr("fit ellipse"), this);
  connect(m_fitEllipseAction, SIGNAL(triggered()), this, SLOT(fitEllipse()));


  m_frontAction = new QAction(tr("Bring to front"), this);
  connect(m_frontAction, SIGNAL(triggered()),
          this, SLOT(bringSelectedToFront()));

  m_backAction = new QAction(tr("Send to back"), this);
  connect(m_backAction, SIGNAL(triggered()),
          this, SLOT(sendSelectedToBack()));

  createPunctaActions();
  createSwcActions();
  createTraceActions();
  createTubeActions();
}

ZStackPresenter::~ZStackPresenter()
{
  foreach(ZStackDrawable *decoration, m_decorationList) {
    delete decoration;
  }

  m_decorationList.clear();
}

void ZStackPresenter::turnOnStroke()
{
  buddyView()->paintActiveDecoration();
  m_isStrokeOn = true;
}

void ZStackPresenter::turnOffStroke()
{
  m_stroke.clear();
  if (isStrokeOn()) {
    buddyView()->paintActiveDecoration();
  }
  m_isStrokeOn = false;
}


ZStackDoc* ZStackPresenter::buddyDocument() const
{
  if (m_parent == 0) {
    return 0;
  }

  return m_parent->document().get();
}

ZStackView* ZStackPresenter::buddyView() const
{
  if (m_parent == 0) {
    return 0;
  }

  return m_parent->view();
}

void ZStackPresenter::addPunctaEditFunctionToRightMenu()
{
  updateRightMenu(m_enlargePunctaAction, false);
  updateRightMenu(m_narrowPunctaAction, false);
  updateRightMenu(m_meanshiftPunctaAction, false);
  updateRightMenu(m_meanshiftAllPunctaAction, false);
  updateRightMenu(m_deleteSelectedAction, false);
  updateRightMenu(m_deleteAllPunctaAction, false);
}

void ZStackPresenter::addTubeEditFunctionToRightMenu()
{
  updateRightMenu(m_cutAction, false);
#ifdef _ADVANCED_
  updateRightMenu(m_breakAction, false);
#endif
  updateRightMenu(m_hookAction, false);
  updateRightMenu(m_spHookAction, false);
  updateRightMenu(m_linkAction, false);
  updateRightMenu(m_mergeAction, false);
  updateRightMenu(m_extendAction, false);
#ifdef _ADVANCED_
  updateRightMenu(m_walkAction, false);
  updateRightMenu(m_checkConnAction, false);
  updateRightMenu(m_connectAction, false);
  updateRightMenu(m_disconnectAction, false);
#endif

  updateRightMenu(m_refineEndAction, false);

  QMenu *selectmenu = new QMenu("Select");
  selectmenu->addAction(m_neighborAction);
  selectmenu->addAction(m_selectConnectedTubeAction);
  updateRightMenu(selectmenu, false);

  QMenu *submenu = new QMenu("Arrange");
  submenu->addAction(m_frontAction);
  submenu->addAction(m_backAction);
  updateRightMenu(submenu, false);
}

void ZStackPresenter::addSwcEditFunctionToRightMenu()
{
  if (buddyDocument()->hasSelectedSwcNode()) {
    if (!buddyDocument()->hasMultipleSelectedSwcNode()) {
      updateRightMenu(m_swcConnectToAction, false);
      updateRightMenu(m_swcExtendAction, false);
      updateRightMenu(m_swcSmartExtendAction, false);
    } else {
      updateRightMenu(m_swcConnectSelectedAction, false);
      updateRightMenu(m_swcBreakSelectedAction, false);
    }
    updateRightMenu(m_swcDeleteAction, false);
    updateRightMenu(m_swcLockFocusAction, false);
    updateRightMenu(m_swcEstimateRadiusAction, false);
    //updateRightMenu(m_swcSelectAllNodeAction, false);
  } else {
    //updateRightMenu(m_swcSelectAllNodeAction, false);
  }
  QMenu *selectMenu = new QMenu("Select");
  selectMenu->addAction(m_selectSwcNodeDownstreamAction);
  selectMenu->addAction(m_selectSwcConnectionAction);
  selectMenu->addAction(m_selectSwcNodeUpstreamAction);
  selectMenu->addAction(m_selectSwcNodeBranchAction);
  //selectMenu->addAction(m_selectSwcNodeTreeAction);
  selectMenu->addAction(m_selectAllConnectedSwcNodeAction);
  selectMenu->addAction(m_selectAllSwcNodeAction);
  updateRightMenu(selectMenu, false);
}

void ZStackPresenter::prepareView()
{
  createDocDependentActions();
  updateLeftMenu(m_traceAction);
  buddyView()->rightMenu()->clear();
  addSwcEditFunctionToRightMenu();
}

void ZStackPresenter::updateLeftMenu(QAction *action, bool clear)
{
  if (clear == true) {
    buddyView()->leftMenu()->clear();
  }

  buddyView()->leftMenu()->addAction(action);
}

void ZStackPresenter::updateLeftMenu()
{
  bool traceOnFlag = false;
  if (interactiveContext().tracingTube()) {
    updateLeftMenu(this->m_traceAction, true);
    traceOnFlag = true;
  } else if (interactiveContext().fittingSegment()) {
    updateLeftMenu(this->m_fitsegAction, true);
    updateLeftMenu(this->m_dropsegAction, false);
    updateLeftMenu(this->m_fitEllipseAction, false);
    traceOnFlag = true;
  }

  if (interactiveContext().markPuncta()) {
    if (traceOnFlag) {
      updateLeftMenu(this->m_markPunctaAction, false);
    } else {
      updateLeftMenu(this->m_markPunctaAction, true);
    }
  }
}

void ZStackPresenter::updateRightMenu(QAction *action, bool clear)
{
  if (clear == true) {
    buddyView()->rightMenu()->clear();
  }

  buddyView()->rightMenu()->addAction(action);
}

void ZStackPresenter::updateRightMenu(QMenu *submenu, bool clear)
{
  if (clear == true) {
    buddyView()->rightMenu()->clear();
  }

  buddyView()->rightMenu()->addMenu(submenu);
}

void ZStackPresenter::updateView() const
{
  buddyView()->redraw();
}

int ZStackPresenter::zoomRatio() const
{
  return buddyView()->imageWidget()->zoomRatio();
}

bool ZStackPresenter::hasObjectToShow() const
{
  if (m_showObject == true) {
    if (m_decorationList.size() > 0) {
      return true;
    }
  }

  return false;
}

const QPoint ZStackPresenter::dataPositionFromMouse(MouseButtonAction mba)
{
  int x, y;
  switch (mba) {
  case LEFT_RELEASE:
    x = m_mouseLeftReleasePosition[0];
    y = m_mouseLeftReleasePosition[1];
    break;
  case RIGHT_RELEASE:
    x = m_mouseRightReleasePosition[0];
    y = m_mouseRightReleasePosition[1];
    break;
  case LEFT_PRESS:
    x = m_mouseLeftPressPosition[0];
    y = m_mouseLeftPressPosition[1];
    break;
  case RIGHT_PRESS:
    x = m_mouseRightPressPosition[0];
    y = m_mouseRightPressPosition[1];
    break;
  case LEFT_DOUBLE_CLICK:
    x = m_mouseLeftDoubleClickPosition[0];
    y = m_mouseLeftDoubleClickPosition[1];
    break;
  case MOVE:
    x = m_mouseMovePosition[0];
    y = m_mouseMovePosition[1];
    break;
  default:
    x = 0;
    y = 0;
  }

  return buddyView()->imageWidget()->canvasCoordinate(QPoint(x, y));
}

//Status: 0 no response; 1: right menu popup; 2: left menu popup; 3: select
ZStackPresenter::EMouseEventProcessStatus
ZStackPresenter::processMouseReleaseForPuncta(
    QMouseEvent *event, int *positionInStack)
{
  if (!isPointInStack(positionInStack[0], positionInStack[1])) {
    return MOUSE_EVENT_PASSED;
  }

  EMouseEventProcessStatus status = MOUSE_EVENT_PASSED;

  switch (event->button()) {
  case Qt::RightButton:
    if (m_interactiveContext.isContextMenuActivated() &&
        buddyDocument()->hasSelectedPuncta()) {
      buddyView()->rightMenu()->clear();
      addPunctaEditFunctionToRightMenu();
      buddyView()->popRightMenu(event->pos());

      status = CONTEXT_MENU_POPPED; //menu popup
    }
    break;
  case Qt::LeftButton:
  {
    int index = buddyDocument()->pickPunctaIndex(positionInStack[0],
        positionInStack[1], positionInStack[2]);
    if (index >= 0) {
      if (event->modifiers() != Qt::ControlModifier)
        buddyDocument()->deselectAllPuncta();
      buddyDocument()->selectPuncta(index);
      status = MOUSE_HIT_OBJECT;
    } else {
      if (m_interactiveContext.isNormalView()) {
        if (m_interactiveContext.isContextMenuActivated() &&
            interactiveContext().markPuncta() && buddyDocument()->hasStackData() &&
            (!buddyDocument()->stack()->isVirtual())) {
          buddyView()->popLeftMenu(event->pos());
          status = CONTEXT_MENU_POPPED;
        }
      }
    }
  }
    break;
  default:
    break;
  }

  return status;
}

ZStackPresenter::EMouseEventProcessStatus
ZStackPresenter::processMouseReleaseForTube(
    QMouseEvent *event, int *positionInStack)
{
  if (!isPointInStack(positionInStack[0], positionInStack[1])) {
    return MOUSE_EVENT_PASSED;
  }

  EMouseEventProcessStatus status = MOUSE_EVENT_PASSED;

  int id = buddyDocument()->pickLocsegChainId(positionInStack[0],
      positionInStack[1], positionInStack[2]);

  if (id >= 0) {
    bool showProfile = false;
    if (event->modifiers() == Qt::ShiftModifier) {
      showProfile = true;
    }

    if (m_interactiveContext.isTubeEditModeOff()) {
      buddyDocument()->selectLocsegChain(
            id, positionInStack[0], positionInStack[1], positionInStack[2],
          showProfile);
    } else {
      buddyDocument()->holdClosestSeg(
            id, positionInStack[0], positionInStack[1], positionInStack[2]);
    }

    status = MOUSE_HIT_OBJECT;
  } else {
    if (event->button() == Qt::RightButton) {
      if (!this->interactiveContext().isTubeEditModeOff()) {
        this->interactiveContext().
            setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
        updateCursor();
      }
    } else {
      if (m_interactiveContext.isNormalView()) {
        if ((event->button() == Qt::LeftButton) &&
            (event->modifiers() != Qt::ShiftModifier)) {
          if ((interactiveContext().fittingSegment() ||
               interactiveContext().tracingTube()) &&
              (interactiveContext().isTubeEditModeOff())) {
            if (m_interactiveContext.isContextMenuActivated() &&
                buddyDocument()->hasTracable()) {
              buddyView()->popLeftMenu(event->pos());
              status = CONTEXT_MENU_POPPED;
            }
          } else {
            if (interactiveContext().tubeEditMode() ==
                ZInteractiveContext::TUBE_EDIT_EXTEND) {
              id = 0;
            }
          }
        }
      }
    }
  }

  if (id >= 0) {
    if (event->button() == Qt::RightButton) {
      if (!this->interactiveContext().isTubeEditModeOff()) {
        this->interactiveContext().
            setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
        updateCursor();
        status = MOUSE_EVENT_PASSED;
      } else if (m_interactiveContext.isContextMenuActivated()) {
        /* pop up a menu to enter an editing mode */
        buddyView()->rightMenu()->clear();
        addTubeEditFunctionToRightMenu();
        buddyView()->popRightMenu(event->pos());
        status = CONTEXT_MENU_POPPED;
      }
    } else {
      bool editExecuted = false;
      switch (this->interactiveContext().tubeEditMode()) {
      case ZInteractiveContext::TUBE_EDIT_LINK:
        if (buddyDocument()->isMasterChainId(id) == false) {
          buddyDocument()->linkChain(id);
          editExecuted = true;
        }
        break;
      case ZInteractiveContext::TUBE_EDIT_HOOK:
        buddyDocument()->hookChain(id);
        editExecuted = true;
        break;
      case ZInteractiveContext::TUBE_EDIT_MERGE:
        buddyDocument()->mergeChain(id);
        editExecuted = true;
        break;
      case ZInteractiveContext::TUBE_EDIT_CONNECT:
        buddyDocument()->connectChain(id);
        editExecuted = true;
        break;
      case ZInteractiveContext::TUBE_EDIT_DISCONNECT:
        buddyDocument()->disconnectChain(id);
        editExecuted = true;
        break;
      case ZInteractiveContext::TUBE_EDIT_WALK:
        buddyDocument()->chainShortestPath(id);
        editExecuted = true;
        break;
      case ZInteractiveContext::TUBE_EDIT_CHECK_CONN:
        buddyDocument()->chainConnInfo(id);
        this->interactiveContext().
            setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
        updateCursor();
        break;
      case ZInteractiveContext::TUBE_EDIT_EXTEND:
        buddyDocument()->extendChain(
              positionInStack[0], positionInStack[1], positionInStack[2]);
        editExecuted = true;
        break;
      case ZInteractiveContext::TUBE_EDIT_SP_HOOK:
        if (buddyDocument()->hookChain(id, 1) == false) {
          QMessageBox::warning(buddyView(), "Operation Failed",
                               "Sorry, I cannot find a path to hook them. "
                               "Maybe the two tubes are too far away from "
                               "each other.");
        }
        editExecuted = true;
        break;
      default:
        break;
      }

      if (editExecuted) {
        this->interactiveContext().
            setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
        updateCursor();
        updateView();
        status = MOUSE_COMMAND_EXECUTED;
      }
    }
  }

  return status;
}

const Swc_Tree_Node* ZStackPresenter::getSelectedSwcNode() const
{
  std::set<Swc_Tree_Node*> *nodeSet = buddyDocument()->selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    return *(nodeSet->begin());
  }

  return NULL;
}

ZStackPresenter::EMouseEventProcessStatus
ZStackPresenter::processMouseReleaseForSwc(
    QMouseEvent *event, int *positionInStack)
{
  EMouseEventProcessStatus status = MOUSE_EVENT_PASSED;

  if (event->button() == Qt::RightButton) {
    if (m_interactiveContext.isContextMenuActivated()) {
      buddyView()->rightMenu()->clear();
      addSwcEditFunctionToRightMenu();
      buddyView()->popRightMenu(event->pos());
      status = CONTEXT_MENU_POPPED;
    } else {
      if (isStrokeOn()) {
        turnOffStroke();
      }
      /*
      if (m_interactiveContext.swcEditMode() ==
          ZInteractiveContext::SWC_EDIT_ADD_NODE ||
          m_interactiveContext.swcEditMode() ==
          ZInteractiveContext::SWC_EDIT_EXTEND) {
        m_stroke.clear();
        buddyView()->paintActiveDecoration();
      }
      */
      enterSwcSelectMode();
    }
  } else if (event->button() == Qt::LeftButton) {
    if (!isPointInStack(positionInStack[0], positionInStack[1])) {
      return status;
    }

    switch (m_interactiveContext.swcEditMode()) {
    case ZInteractiveContext::SWC_EDIT_SELECT:
      if (buddyDocument()->selectSwcTreeNode(
            positionInStack[0], positionInStack[1], positionInStack[2],
            event->modifiers() == Qt::ControlModifier ||
            event->modifiers() == Qt::ShiftModifier)) {
        if (event->modifiers() == Qt::ShiftModifier) {
          buddyDocument()->selectSwcNodeConnection();
        }
        status = MOUSE_HIT_OBJECT;
        if (event->modifiers() != Qt::ShiftModifier &&
            event->modifiers() != Qt::ControlModifier) {
          if (buddyDocument()->selectedSwcTreeNodes()->size() == 1 &&
              NeutubeConfig::getInstance().getApplication() == "Biocytin") {
            enterSwcExtendMode();
          }
        }
      }
      buddyDocument()->notifySwcModified();
      break;
    case ZInteractiveContext::SWC_EDIT_CONNECT:
    {
      status = MOUSE_EVENT_CAPTURED;
      std::set<Swc_Tree_Node*> *nodeSet = buddyDocument()->selectedSwcTreeNodes();
      if (!nodeSet->empty()) {
        Swc_Tree_Node *prevNode = *(nodeSet->begin());
        if (prevNode != NULL) {
          Swc_Tree_Node *tn = buddyDocument()->swcHitTest(
                positionInStack[0], positionInStack[1], positionInStack[2]);
          if (tn != NULL) {
            if (buddyDocument()->executeConnectSwcNodeCommand(prevNode, tn)) {
              enterSwcSelectMode();
              status = MOUSE_COMMAND_EXECUTED;
            }
            /*
            if (!SwcTreeNode::isRegular(SwcTreeNode::commonAncestor(tn, prevNode))) {
              SwcTreeNode::setAsRoot(tn);
              SwcTreeNode::setParent(tn, prevNode);

              status = MOUSE_COMMAND_EXECUTED;
            }
            */
          }
          //buddyDocument()->notifySwcModified();
        }
      }
    }
      break;
    case ZInteractiveContext::SWC_EDIT_EXTEND:
    {
      if (event->modifiers() == Qt::ControlModifier ||
          event->modifiers() == Qt::ShiftModifier) {
        if (buddyDocument()->selectSwcTreeNode(
              positionInStack[0], positionInStack[1], positionInStack[2],
              true)) {
          if (event->modifiers() == Qt::ShiftModifier) {
            buddyDocument()->selectSwcNodeConnection();
          }
          buddyDocument()->notifySwcModified();
          enterSwcSelectMode();
        }
      } else {
        if (buddyDocument()->executeSwcNodeExtendCommand(
              ZPoint(positionInStack[0], positionInStack[1], positionInStack[2]),
              m_stroke.getWidth() / 2.0)) {
          status = MOUSE_COMMAND_EXECUTED;
        }
      }
      //m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
    }
      break;
    case ZInteractiveContext::SWC_EDIT_SMART_EXTEND:
    {
      if (event->modifiers() == Qt::ControlModifier ||
          event->modifiers() == Qt::ShiftModifier) {
        if (buddyDocument()->selectSwcTreeNode(
              positionInStack[0], positionInStack[1], positionInStack[2],
              true)) {
          if (event->modifiers() == Qt::ShiftModifier) {
            buddyDocument()->selectSwcNodeConnection();
          }
          buddyDocument()->notifySwcModified();
          enterSwcSelectMode();
        }
      } else {
        if (buddyDocument()->executeSwcNodeSmartExtendCommand(
              ZPoint(positionInStack[0], positionInStack[1], positionInStack[2]),
              m_stroke.getWidth() / 2.0)) {
          status = MOUSE_COMMAND_EXECUTED;
        }
      }
      //m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
    }
      break;
    case ZInteractiveContext::SWC_EDIT_ADD_NODE:
    {
      //double radius = m_cursorRadius / buddyView()->getZoomRatio();
      double radius = m_stroke.getWidth() / 2.0;
      if (buddyDocument()->executeAddSwcNodeCommand(
            ZPoint(positionInStack[0], positionInStack[1], positionInStack[2]),
            radius)) {
        status = MOUSE_COMMAND_EXECUTED;
      }
    }
    default:
      break;
    }
  }

  return status;
}

bool ZStackPresenter::isPointInStack(double x, double y)
{
  return (x >= 0) && (x < buddyDocument()->stack()->width()) &&
      (y >= 0) && (y < buddyDocument()->stack()->height());
}

ZStackPresenter::EMouseEventProcessStatus
ZStackPresenter::processMouseReleaseForStroke(
    QMouseEvent *event, int */*positionInStack*/)
{
  EMouseEventProcessStatus status = MOUSE_EVENT_PASSED;

  if (event->button() == Qt::RightButton) {
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      exitStrokeEdit();
    }
  } else if (event->button() == Qt::LeftButton) {
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      ZStroke2d *newStroke = m_stroke.clone();
      if (!newStroke->isEraser()) {
        if (newStroke->getPointNumber() == 1 &&
            event->modifiers() == Qt::ShiftModifier) {
          if (!buddyDocument()->getStrokeList().empty()) {
            int start[3] = {0, 0, 0};
            int end[3] = {0, 0, 0};
            buddyDocument()->getLastStrokePoint(start, start + 1);
            newStroke->getLastPoint(end, end + 1);
            Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
            double pointDistance = Geo3d_Dist(start[0], start[1], start[2],
                end[0], end[1], end[2]) / 2;
            double cx = ((double) (start[0] + end[0])) / 2;
            double cy = ((double) (start[1] + end[1])) / 2;
            double cz = ((double) (start[2] + end[2])) / 2;
            int x0 = (int) (cx - pointDistance);
            int x1 = (int) (cx + pointDistance);
            int y0 = (int) (cy - pointDistance);
            int y1 = (int) (cy + pointDistance);
            int z0 = (int) (cz - pointDistance);
            int z1 = (int) (cz + pointDistance);

            Stack_Graph_Workspace_Set_Range(sgw, x0, x1, y0, y1, z0, z1);
            Stack_Graph_Workspace_Validate_Range(
                  sgw, buddyDocument()->stack()->width(),
                  buddyDocument()->stack()->height(),
                  buddyDocument()->stack()->depth());

            sgw->wf = Stack_Voxel_Weight;

            Int_Arraylist *path =
                Stack_Route(buddyDocument()->stack()->c_stack(),
                            start, end, sgw);

            newStroke->clear();
            for (int i = 0; i < path->length; ++i) {
              int x, y, z;
              C_Stack::indexToCoord(path->array[i], buddyDocument()->stack()->width(),
                                    buddyDocument()->stack()->height(),
                                    &x, &y, &z);
              newStroke->append(x, y);
            }
          }
        }
      } else {
        newStroke->setColor(QColor(0, 0, 0, 0));
      }
      buddyDocument()->executeAddStrokeCommand(newStroke);
    }

    m_stroke.clear();
    buddyView()->paintActiveDecoration();

    status = MOUSE_COMMAND_EXECUTED;
  }

  return status;
}

void ZStackPresenter::processMouseReleaseEvent(
    QMouseEvent *event, int sliceIndex)
{
  QPoint pos;

  if (event->button() == Qt::LeftButton) {
    m_mouseLeftReleasePosition[0] = event->x();
    m_mouseLeftReleasePosition[1] = event->y();
    m_mouseLeftReleasePosition[2] = sliceIndex;
    m_mouseLeftButtonPressed = false;
    pos = dataPositionFromMouse(LEFT_RELEASE);
  } else if (event->button() == Qt::RightButton) {
    m_mouseRightReleasePosition[0] = event->x();
    m_mouseRightReleasePosition[1] = event->y();
    m_mouseRightReleasePosition[2] = sliceIndex;
    m_mouseRightButtonPressed = false;
    pos = dataPositionFromMouse(RIGHT_RELEASE);
  }

  int positionInStack[3];
  positionInStack[0] = pos.x();
  positionInStack[1] = pos.y();
  positionInStack[2] = sliceIndex;

  switch (this->interactiveContext().exploreMode()) {
  case ZInteractiveContext::EXPLORE_CAPTURE_MOUSE: //It triggers a processing step
    if (isPointInStack(positionInStack[0], positionInStack[1])) {
      if (interactiveContext().isProjectView()) {
        positionInStack[2] = buddyDocument()->maxIntesityDepth(pos.x(), pos.y());
      }
      this->interactiveContext().restoreExploreMode();

      emit mousePositionCaptured(
            positionInStack[0], positionInStack[1], positionInStack[2]);
    }
    break;
  case ZInteractiveContext::EXPLORE_OFF:
  {
    if (interactiveContext().isProjectView()) {
      positionInStack[2] = -1;
    }

    ZStackPresenter::EMouseEventProcessStatus status = MOUSE_EVENT_PASSED;

    if (isPointInStack(positionInStack[0], positionInStack[1])) {
      //swc
      if (status == MOUSE_EVENT_PASSED) {
        status = processMouseReleaseForSwc(event, positionInStack);
      }

      // puncta process
      if (status == MOUSE_EVENT_PASSED) {
        status = processMouseReleaseForPuncta(event, positionInStack);
      }

      // tube chain
      if (status == MOUSE_EVENT_PASSED) {
        status = processMouseReleaseForTube(event, positionInStack);
      }

      if (status == MOUSE_EVENT_PASSED) {
        status = processMouseReleaseForStroke(event, positionInStack);
      }
    }
  }
    break;
  case ZInteractiveContext::EXPLORE_MOVE_IMAGE:
    this->interactiveContext().restoreExploreMode();
    break;
  default:
    break;
  }
}

void ZStackPresenter::setViewPortCenter(int x, int y, int z)
{
  buddyView()->imageWidget()->setViewPortOffset(
        x - buddyView()->imageWidget()->viewPort().width() / 2,
        y - buddyView()->imageWidget()->viewPort().height() / 2);
  buddyView()->setSliceIndex(z);
  buddyView()->updateImageScreen();
}

void ZStackPresenter::processMouseMoveEvent(QMouseEvent *event)
{
  m_mouseMovePosition[0] = event->x();
  m_mouseMovePosition[1] = event->y();
  if (interactiveContext().isProjectView())
    m_mouseMovePosition[2] = -1;
  else
    m_mouseMovePosition[2] = buddyView()->sliceIndex();

  switch (this->interactiveContext().exploreMode()) {
  case ZInteractiveContext::EXPLORE_MOVE_IMAGE:
    {
      int x, y;
      x = m_grabPosition.x()
          - event->x() * buddyView()->imageWidget()->viewPort().width()
          / buddyView()->imageWidget()->projectSize().width();
      y = m_grabPosition.y()
          - event->y() * buddyView()->imageWidget()->viewPort().height()
          / buddyView()->imageWidget()->projectSize().height();

      buddyView()->imageWidget()->setViewPortOffset(x, y);
      buddyView()->updateImageScreen();
    }

    break;
  default:
    if (m_mouseLeftButtonPressed == true){
      if (m_interactiveContext.strokeEditMode() ==
          ZInteractiveContext::STROKE_DRAW) {
        QPoint pos = buddyView()->imageWidget()->
                     canvasCoordinate(QPoint(event->x(), event->y()));
        m_stroke.append(pos.x(), pos.y());
        buddyView()->paintActiveDecoration();
      } else {
        //if (m_zoomRatio > 1) {
        if (buddyView()->imageWidget()->zoomRatio() > 1) {
          this->interactiveContext().backupExploreMode();
          this->interactiveContext().
              setExploreMode(ZInteractiveContext::EXPLORE_MOVE_IMAGE);
          m_grabPosition = buddyView()->screen()->canvasCoordinate(event->pos());
        }
      }
    } else {
      QPoint pos = buddyView()->imageWidget()->
                   canvasCoordinate(QPoint(event->x(), event->y()));
      int z = buddyView()->sliceIndex();
      if (interactiveContext().isProjectView()) {
        z = -1;
      }
      buddyView()->setInfo(buddyDocument()->dataInfo(pos.x(), pos.y(), z));

      if (isStrokeOn()) {
        m_stroke.set(pos);
        if (m_interactiveContext.strokeEditMode() !=
            ZInteractiveContext::STROKE_DRAW) {
          m_stroke.setFilled(false);
        }
        turnOnStroke();
        //buddyView()->paintActiveDecoration();
      }
      /*
      if (m_interactiveContext.strokeEditMode() ==
          ZInteractiveContext::STROKE_DRAW) {
        m_stroke.set(pos);
        buddyView()->paintActiveDecoration();
      } else if (m_interactiveContext.swcEditMode() ==
                 ZInteractiveContext::SWC_EDIT_ADD_NODE) {
        m_stroke.set(pos);
        m_stroke.setFilled(false);
        buddyView()->paintActiveDecoration();
      }
      */
    }
    break;
  }
}

QPoint ZStackPresenter::mapFromWidgetToStack(const QPoint &pos)
{
  return buddyView()->imageWidget()->canvasCoordinate(pos);
}

QPoint ZStackPresenter::mapFromGlobalToStack(const QPoint &pos)
{
  return mapFromWidgetToStack(buddyView()->imageWidget()->mapFromGlobal(pos));
}


void ZStackPresenter::processMousePressEvent(QMouseEvent *event)
{
  m_interactiveContext.setExitingEdit(false);

  if (event->button() == Qt::LeftButton) {
    m_mouseLeftButtonPressed = true;

    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      m_stroke.set(mapFromWidgetToStack(event->pos()));
    }
  }

  if (event->button() == Qt::RightButton) {
    m_mouseRightButtonPressed = true;
  }
}

#define REMOVE_SELECTED_OBJECT(objtype, list, iter)	\
QMutableListIterator<objtype*> iter(list);	\
    while (iter.hasNext()) {	\
      if (iter.next()->isSelected()) {	\
        iter.remove();	\
      }	\
}

bool ZStackPresenter::processKeyPressEventForSwc(QKeyEvent *event)
{
  bool taken = false;
  switch (event->key()) {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
  case Qt::Key_X:
    taken = buddyDocument()->executeDeleteSwcNodeCommand();
    break;
  case Qt::Key_W:
  {
    double dy = -1;
    if (event->modifiers() == Qt::ShiftModifier) {
      dy *= 10;
    }
    taken = buddyDocument()->executeMoveSwcNodeCommand(0, dy, 0);
  }
    break;
  case Qt::Key_A:
  {
    double dx = -1;
    if (event->modifiers() == Qt::ShiftModifier) {
      dx *= 10;
    }
    taken = buddyDocument()->executeMoveSwcNodeCommand(dx, 0, 0);
  }
    break;
  case Qt::Key_S:
  {
    double dy = 1;
    if (event->modifiers() == Qt::ShiftModifier) {
      dy *= 10;
    }
    taken = buddyDocument()->executeMoveSwcNodeCommand(0, dy, 0);
  }
    break;
  case Qt::Key_D:
  {
    double dx = 1;
    if (event->modifiers() == Qt::ShiftModifier) {
      dx *= 10;
    }
    taken = buddyDocument()->executeMoveSwcNodeCommand(dx, 0, 0);
  }
    break;
  case Qt::Key_G:
    if (event->modifiers() == Qt::ControlModifier) {
      QPoint pos = mapFromGlobalToStack(QCursor::pos());
      trySwcAddNodeMode(pos.x(), pos.y());
      /*
      if (interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
          interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT) {
        QPoint pos = mapFromGlobalToStack(QCursor::pos());
        enterSwcAddNodeMode(pos.x(), pos.y());
      }
      */
    }
    break;
  case Qt::Key_Comma:
  case Qt::Key_Q:
#if 0
    /*
    if (interactiveContext().swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_ADD_NODE) {
        */
    if (isStrokeOn()) {
      /*
        m_cursorRadius = ZCursorStore::prevCircleRadius(m_cursorRadius);
        updateCursor();
        taken = true;
        */
      m_stroke.addWidth(-1.0);
      buddyView()->paintActiveDecoration();
      taken = true;
    } else {
      //buddyDocument()->addSizeForSelectedSwcNode(-0.5);
      taken = buddyDocument()->executeSwcNodeChangeSizeCommand(-0.5);
    }
#endif

    if (isStrokeOff()) {
      taken = buddyDocument()->executeSwcNodeChangeSizeCommand(-0.5);
    }

    break;
  case Qt::Key_Period:
  case Qt::Key_E:
#if 0
    /*
    if (interactiveContext().swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_ADD_NODE) {
        */
    if (isStrokeOn()) {
      m_stroke.addWidth(1.0);
      buddyView()->paintActiveDecoration();
      taken = true;
    } else {
      //buddyDocument()->addSizeForSelectedSwcNode(0.5);
      taken = buddyDocument()->executeSwcNodeChangeSizeCommand(0.5);
    }
#endif
    if (isStrokeOff()) {
      taken = buddyDocument()->executeSwcNodeChangeSizeCommand(0.5);
    }
    break;
  default:
    break;
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForStroke(QKeyEvent *event)
{
  bool taken = false;
  switch (event->key()) {
#if 0
  case Qt::Key_Comma:
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      m_stroke.addWidth(-1.0);
      buddyView()->paintActiveDecoration();
      taken = true;
    }
    break;
  case Qt::Key_Period:
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      m_stroke.addWidth(1.0);
      buddyView()->paintActiveDecoration();
      taken = true;
    }
    break;
#endif
  case Qt::Key_R:
    if (event->modifiers() == Qt::ControlModifier) {
      QPoint pos = mapFromGlobalToStack(QCursor::pos());
      tryDrawStrokeMode(pos.x(), pos.y(), false);
      /*
      if (interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
          interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT ||
          interactiveContext().isTubeEditModeOff()) {
        QPoint pos = buddyView()->imageWidget()->canvasCoordinate(
              buddyView()->imageWidget()->mapFromGlobal(QCursor::pos()));
        enterDrawStrokeMode(pos.x(), pos.y());
      }
      */
    }
    break;
  case Qt::Key_E:
    if (event->modifiers() == Qt::ControlModifier) {
      QPoint pos = mapFromGlobalToStack(QCursor::pos());
      tryDrawStrokeMode(pos.x(), pos.y(), true);
      /*
      if (interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
          interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT ||
          interactiveContext().isTubeEditModeOff()) {
        QPoint pos = buddyView()->imageWidget()->canvasCoordinate(
              buddyView()->imageWidget()->mapFromGlobal(QCursor::pos()));
        enterEraseStrokeMode(pos.x(), pos.y());
      }
      */
    }
    break;
  default:
    break;
  }

  return taken;
}

void ZStackPresenter::setZoomRatio(int ratio)
{
  //m_zoomRatio = ratio;
  //CLIP_VALUE(m_zoomRatio, 1, 16);
  //buddyView()->imageWidget()->setZoomRatio(m_zoomRatio);
  buddyView()->imageWidget()->setZoomRatio(ratio);
}

void ZStackPresenter::processKeyPressEvent(QKeyEvent *event)
{
  if (processKeyPressEventForSwc(event)) {
    return;
  }

  if (processKeyPressEventForStroke(event)) {
    return;
  }

  switch (event->key()) {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
    buddyDocument()->executeRemoveObjectCommand();
    break;
  case Qt::Key_P:
    if (event->modifiers() == Qt::ControlModifier) {
      if (interactiveContext().isProjectView()) {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      } else {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
      }
      updateView();
      emit(viewModeChanged());
    } else if (event->modifiers() == Qt::ShiftModifier) {
      buddyDocument()->pushSelectedLocsegChain();
      updateView();
    }
    break;

  case Qt::Key_T:
    for (int i = 0; i < m_decorationList.size(); i++) {
      if (m_decorationList.at(i)->isSelected()) {
        buddyDocument()->
            fixLocsegChainTerminal((ZInterface*) (m_decorationList.at(i)));
      }
    }
    break;

  case Qt::Key_Equal:
  case Qt::Key_Up:
    //    if (m_zoomRatio < 16) {
    //      m_zoomRatio += 1;
    //      buddyView()->imageWidget()->setZoomRatio(m_zoomRatio);
    //      buddyView()->updateImageScreen();
    //    }
    buddyView()->imageWidget()->increaseZoomRatio();
    break;

  case Qt::Key_Minus:
  case Qt::Key_Down:
    //    if (m_zoomRatio > 1) {
    //      m_zoomRatio -= 1;
    //      buddyView()->imageWidget()->setZoomRatio(m_zoomRatio);
    //      buddyView()->updateImageScreen();
    //    }
    buddyView()->imageWidget()->decreaseZoomRatio();
    break;

  case Qt::Key_W:
    if (event->modifiers() == Qt::ShiftModifier) {
      //setZoomOffset(m_zoomOffset.x(), m_zoomOffset.y() + 10);
      buddyView()->imageWidget()->moveViewPort(0, 10);
    } else {
      //setZoomOffset(m_zoomOffset.x(), m_zoomOffset.y() + 1);
      buddyView()->imageWidget()->moveViewPort(0, 1);
    }
    buddyView()->updateImageScreen();
    break;
  case Qt::Key_A:
    if (event->modifiers() == Qt::ShiftModifier) {
      buddyView()->imageWidget()->moveViewPort(10, 0);
    } else {
      buddyView()->imageWidget()->moveViewPort(1, 0);
    }
    buddyView()->updateImageScreen();
    break;

  case Qt::Key_S:
    if (event->modifiers() == Qt::ShiftModifier) {
      buddyView()->imageWidget()->moveViewPort(0, -10);
    } else {
      buddyView()->imageWidget()->moveViewPort(0, -1);
    }
    buddyView()->updateImageScreen();
    break;

  case Qt::Key_D:
    if (event->modifiers() == Qt::ShiftModifier) {
      buddyView()->imageWidget()->moveViewPort(-10, 0);
    } else {
      buddyView()->imageWidget()->moveViewPort(-1, 0);
    }
    buddyView()->updateImageScreen();
    break;

  case Qt::Key_Left:
    if (interactiveContext().isNormalView()) {
      int step = -1;

      if (event->modifiers() & Qt::ShiftModifier) {
        step = -5;
      }
      buddyView()->stepSlice(step);
    }
    break;

  case Qt::Key_Right:
    if (interactiveContext().isNormalView()) {
      int step = 1;

      if (event->modifiers() & Qt::ShiftModifier) {
        step = 5;
      }
      buddyView()->stepSlice(step);
    }
    break;


  case Qt::Key_M:
    if (m_interactiveContext.isNormalView()) {
      if (interactiveContext().markPuncta() && buddyDocument()->hasStackData() &&
          (!buddyDocument()->stack()->isVirtual())) {
        QPoint dataPos = dataPositionFromMouse(MOVE);
        buddyDocument()->markPunctum(dataPos.x(), dataPos.y(),
                                     buddyView()->sliceIndex());
      }
    }
    break;

  case Qt::Key_Escape:
    m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
    m_interactiveContext.setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
    updateCursor();
    break;

  case Qt::Key_Comma:
  case Qt::Key_Q:
    if (isStrokeOn()) {
      m_stroke.addWidth(-1.0);
      buddyView()->paintActiveDecoration();
    }
    break;
  case Qt::Key_Period:
  case Qt::Key_E:
    if (isStrokeOn()) {
      m_stroke.addWidth(1.0);
      buddyView()->paintActiveDecoration();
    }
    break;
  default:
    break;
  }
}

void ZStackPresenter::processMouseDoubleClickEvent(QMouseEvent *event,
                                                   int sliceIndex)
{
  if ((event->button() == Qt::RightButton)) {
    return;
  }

  m_mouseLeftReleasePosition[0] = event->x();
  m_mouseLeftReleasePosition[1] = event->y();
  m_mouseLeftReleasePosition[2] = sliceIndex;
  m_mouseLeftButtonPressed = false;

  QPoint pos = dataPositionFromMouse(LEFT_RELEASE);

  if (interactiveContext().isProjectView()) {
    Swc_Tree_Node *tn = buddyDocument()->swcHitTest(pos.x(), pos.y(), -1);
    if (tn != NULL) {
      buddyView()->setSliceIndex(iround(SwcTreeNode::z(tn)));
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      updateView();
      emit(viewModeChanged());

      return;
    }
  }

  if (!interactiveContext().isTraceModeOff()) {
    if (interactiveContext().isProjectView()) {

        /*
      }

      int id = buddyDocument()->pickLocsegChainId(pos.x(), pos.y(), -1);

      if (id >= 0) {
        int found = buddyDocument()->
                    selectLocsegChain(id, pos.x(), pos.y(), -1, false);
        if (found) {
          buddyView()->setSliceIndex(found - 1);
          //m_mode = NORMAL_MODE;
          interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
          emit(viewModeChanged());
          //updateView();
        }
        */
      //} else {

      if (buddyDocument()->hasStackData()) {
        int sliceIndex =
            buddyDocument()->maxIntesityDepth(pos.x(), pos.y());
        buddyView()->setSliceIndex(sliceIndex);
      }
      //m_mode = NORMAL_MODE;
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      updateView();
      emit(viewModeChanged());
      //}
    } else {
#if 0
      if (event->modifiers() == Qt::ShiftModifier) {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
        updateView();
        emit(viewModeChanged());
      }
#endif
    }
  } else {
    if (interactiveContext().isProjectView()) {
      if (buddyDocument()->hasStackData()) {
        QPoint pos = dataPositionFromMouse(LEFT_RELEASE);
        int sliceIndex =
            buddyDocument()->maxIntesityDepth(pos.x(), pos.y());
        buddyView()->setSliceIndex(sliceIndex);
      }
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
    } else { /* this event will be interrupted by mouse release listener */
      if (event->modifiers() == Qt::ShiftModifier) {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
      }
    }
    updateView();
    emit(viewModeChanged());
  }
}

void ZStackPresenter::setObjectVisible(bool v)
{
  if (m_showObject != v) {
    m_showObject = v;
    buddyView()->paintObject();
  }
}

void ZStackPresenter::setObjectStyle(ZStackDrawable::Display_Style style)
{
  if (m_objStyle != style) {
    m_objStyle = style;
    updateView();
  }
}

bool ZStackPresenter::isObjectVisible()
{
  return m_showObject;
}

void ZStackPresenter::removeLastDecoration(ZInterface *obj)
{
  if (!m_decorationList.isEmpty()) {
    if (obj == NULL) {
      delete m_decorationList.takeLast();
      updateView();
    } else {
      if (obj == (ZInterface *) m_decorationList.last()) {
        m_decorationList.removeLast();
        updateView();
      } else if (obj == (ZInterface *) m_decorationList.first()) {
        m_decorationList.removeFirst();
        updateView();
      }
    }
  }
}

void ZStackPresenter::removeDecoration(ZStackDrawable *obj, bool redraw)
{
  for (int i = 0; i < m_decorationList.size(); i++) {
    if (m_decorationList.at(i) == obj) {
      m_decorationList.removeAt(i);
      break;
    }
  }

  if (redraw == true) {
    updateView();
  }
}

void ZStackPresenter::removeAllDecoration()
{
  m_decorationList.clear();
}

void ZStackPresenter::addDecoration(ZStackDrawable *obj, bool tail)
{
  if (tail == true) {
    m_decorationList.append(obj);
  } else {
    m_decorationList.prepend(obj);
  }
}



void ZStackPresenter::setStackBc(double scale, double offset, int c)
{
  if (c >= 0 && c < (int) m_greyScale.size()) {
    m_greyScale[c] = scale;
    m_greyOffset[c] = offset;
  }
}

void ZStackPresenter::optimizeStackBc()
{
  if (buddyDocument() != NULL) {
    ZStack *stack = buddyDocument()->stack();
    if (stack != NULL) {
      if (!stack->isVirtual()) {
        double scale, offset;
        m_greyScale.resize(stack->channelNumber());
        m_greyOffset.resize(stack->channelNumber());
        for (int i=0; i<stack->channelNumber(); i++) {
          stack->bcAdjustHint(&scale, &offset, i);
          setStackBc(scale, offset, i);
        }
      }
    }
  }
}

void ZStackPresenter::autoThreshold()
{
  buddyView()->setThreshold(buddyDocument()->autoThreshold());
}

void ZStackPresenter::binarizeStack()
{
  buddyDocument()->binarize(buddyView()->getIntensityThreshold());
}

void ZStackPresenter::solidifyStack()
{
  buddyDocument()->bwsolid();
}

void ZStackPresenter::autoTrace()
{
  buddyDocument()->executeAutoTraceCommand();
}

void ZStackPresenter::traceTube()
{
  QPoint dataPos = dataPositionFromMouse(LEFT_RELEASE);
  buddyView()->setImageWidgetCursor(Qt::WaitCursor);
  if (buddyDocument()->stack()->channelNumber() == 1) {
//  if (buddyDocument()->stack()->depth() == 1) {
//    buddyDocument()->traceRect(dataPos.x(), dataPos.y(),
//                               m_mouseLeftReleasePosition[2]);
//  } else {
    /*
    buddyDocument()->executeTraceTubeCommand(dataPos.x(), dataPos.y(),
                                             m_mouseLeftReleasePosition[2]);
                                             */
    buddyDocument()->executeTraceSwcBranchCommand(
          dataPos.x(), dataPos.y(), m_mouseLeftReleasePosition[2]);
#if 0
    QUndoCommand *traceTubeCommand = new ZStackDocTraceTubeCommand(buddyDocument(),
                                                                   dataPos.x(), dataPos.y(),
                                                                   m_mouseLeftReleasePosition[2]);
    buddyDocument()->pushUndoCommand(traceTubeCommand);
#endif
  } else if (buddyDocument()->stack()->channelNumber() > 1) {
    ChannelDialog dlg(NULL, buddyDocument()->stack()->channelNumber());
    if (dlg.exec() == QDialog::Accepted) {
      int channel = dlg.channel();
      buddyDocument()->executeTraceTubeCommand(
            dataPos.x(), dataPos.y(), m_mouseLeftReleasePosition[2], channel);
#if 0
      QUndoCommand *traceTubeCommand = new ZStackDocTraceTubeCommand(buddyDocument(),
                                                                     dataPos.x(), dataPos.y(),
                                                                     m_mouseLeftReleasePosition[2],
                                                                     channel);
      buddyDocument()->pushUndoCommand(traceTubeCommand);
#endif
    }
  }
  //buddyView()->setImageWidgetCursor(Qt::CrossCursor);
  updateCursor();
  //updateView();
}

void ZStackPresenter::fitSegment()
{
  QPoint dataPos = dataPositionFromMouse(LEFT_RELEASE);
  /*
  buddyDocument()->fitseg(dataPos.x(), dataPos.y(),
                          m_mouseLeftReleasePosition[2]);
                          */
  if (buddyDocument()->stack()->depth() == 1) {
    buddyDocument()->fitRect(dataPos.x(), dataPos.y(),
            m_mouseLeftReleasePosition[2]);
  } else {
    buddyDocument()->fitseg(dataPos.x(), dataPos.y(),
                             m_mouseLeftReleasePosition[2]);
  }
}

void ZStackPresenter::fitEllipse()
{
  QPoint dataPos = dataPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->fitEllipse(dataPos.x(), dataPos.y(),
                              m_mouseLeftReleasePosition[2]);
}

void ZStackPresenter::markPuncta()
{
  QPoint dataPos = dataPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->markPunctum(dataPos.x(), dataPos.y(),
                          m_mouseLeftReleasePosition[2]);
}

void ZStackPresenter::dropSegment()
{
  QPoint dataPos = dataPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->dropseg(dataPos.x(), dataPos.y(),
                           m_mouseLeftReleasePosition[2]);
}


QStringList ZStackPresenter::toStringList() const
{
  QStringList list;
  list.append(QString("Number of decorations: %1")
              .arg(m_decorationList.size()));

  return list;
}

void ZStackPresenter::deleteSelected()
{
  buddyDocument()->executeRemoveObjectCommand();
#if 0
  QUndoCommand *removeselectedcommand =
      new ZStackDocRemoveSelectedObjectCommand(buddyDocument());
  buddyDocument()->pushUndoCommand(removeselectedcommand);
  //m_parent->undoStack()->push(removeselectedcommand);
  //buddyDocument()->removeSelectedObject(true);
  //updateView();
#endif
}

void ZStackPresenter::deleteAllPuncta()
{
  buddyDocument()->deleteAllPuncta();
  updateView();
}

void ZStackPresenter::enlargePuncta()
{
  buddyDocument()->expandSelectedPuncta();
  updateView();
}

void ZStackPresenter::narrowPuncta()
{
  buddyDocument()->shrinkSelectedPuncta();
  updateView();
}

void ZStackPresenter::meanshiftPuncta()
{
  buddyDocument()->meanshiftSelectedPuncta();
  updateView();
}

void ZStackPresenter::meanshiftAllPuncta()
{
  buddyDocument()->meanshiftAllPuncta();
  updateView();
}

void ZStackPresenter::updateStackBc()
{
  optimizeStackBc();
}

void ZStackPresenter::enterHookMode()
{
  //m_intMode[1] = INT_HOOK_TUBE;
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_HOOK);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterSpHookMode()
{
  //m_intMode[1] = INT_SP_HOOK_TUBE;
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_SP_HOOK);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterLinkMode()
{
  //m_intMode[1] = INT_LINK_TUBE;
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_LINK);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterMergeMode()
{
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_MERGE);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterWalkMode()
{
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_WALK);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterCheckConnMode()
{
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_CHECK_CONN);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterConnectMode()
{
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_CONNECT);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterExtendMode()
{
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_EXTEND);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterDisconnectMode()
{
  this->interactiveContext().
      setTubeEditMode(ZInteractiveContext::TUBE_EDIT_DISCONNECT);
  buddyDocument()->updateMasterLocsegChain();
  buddyView()->setScreenCursor(Qt::PointingHandCursor);
}

void ZStackPresenter::enterMouseCapturingMode()
{
  this->interactiveContext().setExploreMode(ZInteractiveContext::EXPLORE_CAPTURE_MOUSE);
}

void ZStackPresenter::enterSwcConnectMode()
{
  this->interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_CONNECT);
  updateCursor();
}

void ZStackPresenter::enterSwcExtendMode()
{
  const Swc_Tree_Node *tn = getSelectedSwcNode();
  if (tn != NULL) {
    m_stroke.setFilled(false);
    m_stroke.set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
    m_stroke.setWidth(SwcTreeNode::radius(tn) * 2.0);
    turnOnStroke();
    interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_EXTEND);
    updateCursor();
  }
}

void ZStackPresenter::enterSwcSmartExtendMode()
{
  const Swc_Tree_Node *tn = getSelectedSwcNode();
  if (tn != NULL) {
    m_stroke.setFilled(false);
    m_stroke.set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
    m_stroke.setWidth(SwcTreeNode::radius(tn) * 2.0);
    turnOnStroke();
    interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_SMART_EXTEND);
    updateCursor();
  }
}


void ZStackPresenter::enterSwcAddNodeMode(double x, double y)
{
  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_ADD_NODE);
  m_stroke.setWidth(6.0);
  m_stroke.set(x, y);
  m_stroke.setEraser(false);
  m_stroke.setFilled(false);
  m_stroke.setTarget(ZStackDrawable::WIDGET);
  turnOnStroke();
  //buddyView()->paintActiveDecoration();
  updateCursor();
}

void ZStackPresenter::trySwcAddNodeMode(double x, double y)
{
  if (interactiveContext().strokeEditMode() !=
      ZInteractiveContext::STROKE_DRAW) {
    enterSwcAddNodeMode(x, y);
  }
}

void ZStackPresenter::tryDrawStrokeMode(double x, double y, bool isEraser)
{
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
         interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT) &&
        interactiveContext().tubeEditMode() == ZInteractiveContext::TUBE_EDIT_OFF) {
      if (isEraser) {
        enterEraseStrokeMode(x, y);
      } else {
        enterDrawStrokeMode(x, y);
      }
    }
  }
}

void ZStackPresenter::enterDrawStrokeMode(double x, double y)
{
  m_stroke.set(x, y);
  m_stroke.setEraser(false);
  m_stroke.setFilled(true);
  m_stroke.setTarget(ZStackDrawable::OBJECT_CANVAS);
  turnOnStroke();
  //buddyView()->paintActiveDecoration();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  updateCursor();
}

void ZStackPresenter::enterEraseStrokeMode(double x, double y)
{
  m_stroke.set(x, y);
  m_stroke.setFilled(true);
  m_stroke.setEraser(true);
  m_stroke.setTarget(ZStackDrawable::OBJECT_CANVAS);
  turnOnStroke();
  //buddyView()->paintActiveDecoration();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  updateCursor();
}

void ZStackPresenter::exitStrokeEdit()
{
  turnOffStroke();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_EDIT_OFF);
  updateCursor();

  m_interactiveContext.setExitingEdit(true);
}

void ZStackPresenter::deleteSwcNode()
{
  buddyDocument()->executeDeleteSwcNodeCommand();
}

void ZStackPresenter::lockSelectedSwcNodeFocus()
{
  this->interactiveContext().setSwcEditMode(
        ZInteractiveContext::SWC_EDIT_LOCK_FOCUS);
  buddyDocument()->executeSwcNodeChangeZCommand(buddyView()->sliceIndex());
  updateCursor();
}

void ZStackPresenter::estimateSelectedSwcRadius()
{
  buddyDocument()->executeSwcNodeEstimateRadiusCommand();
}

void ZStackPresenter::connectSelectedSwcNode()
{
  buddyDocument()->executeConnectSwcNodeCommand();
}

void ZStackPresenter::breakSelectedSwcNode()
{
  buddyDocument()->executeBreakSwcConnectionCommand();
}

void ZStackPresenter::processSliceChangeEvent(int z)
{
  if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_LOCK_FOCUS) {
    buddyDocument()->executeSwcNodeChangeZCommand(z);
  }
}

void ZStackPresenter::cutTube()
{
  //buddyDocument()->cutSelectedLocsegChain();
  //updateView();


#if 0
  QUndoCommand *cutSelectedTubeCommand = new ZStackDocCutSelectedLocsegChainCommand(buddyDocument());
  buddyDocument()->pushUndoCommand(cutSelectedTubeCommand);
#endif
  //m_parent->undoStack()->push(cutSelectedTubeCommand);
}

void ZStackPresenter::breakTube()
{
  //buddyDocument()->breakSelectedLocsegChain();
  //updateView();
#if 0
  QUndoCommand *breakSelectedTubeCommand = new ZStackDocBreakSelectedLocsegChainCommand(buddyDocument());
  buddyDocument()->pushUndoCommand(breakSelectedTubeCommand);
#endif
  //m_parent->undoStack()->push(breakSelectedTubeCommand);
}

void ZStackPresenter::refineChainEnd()
{
  buddyView()->setImageWidgetCursor(Qt::WaitCursor);
  buddyDocument()->refineSelectedChainEnd();
  buddyView()->resetScreenCursor();
  updateView();
}

void ZStackPresenter::bringSelectedToFront()
{
  buddyDocument()->bringChainToFront();
  updateView();
}

void ZStackPresenter::sendSelectedToBack()
{
  buddyDocument()->sendChainToBack();
  updateView();
}

void ZStackPresenter::selectNeighbor()
{
  buddyDocument()->selectNeighbor();
  updateView();
}

void ZStackPresenter::selectConnectedTube()
{
  buddyDocument()->selectConnectedChain();
  updateView();
}

void ZStackPresenter::enterSwcSelectMode()
{
  if (m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_OFF ||
      m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_SELECT) {
    m_interactiveContext.setExitingEdit(true);
  }

  m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
  updateCursor();
}

void ZStackPresenter::updateCursor()
{
  if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_EXTEND ||
      this->interactiveContext().swcEditMode() ==
            ZInteractiveContext::SWC_EDIT_SMART_EXTEND) {
    buddyView()->setScreenCursor(Qt::PointingHandCursor);
  } else if (this->interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_LOCK_FOCUS) {
    buddyView()->setScreenCursor(Qt::ClosedHandCursor);
  } else if (interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_ADD_NODE){
    buddyView()->setScreenCursor(
          ZCursorStore::getInstance().getSmallCrossCursor());
  } else if (this->interactiveContext().tubeEditMode() ==
             ZInteractiveContext::TUBE_EDIT_HOOK ||
             interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_CONNECT) {
    buddyView()->setScreenCursor(Qt::SizeBDiagCursor);
  } else if (this->interactiveContext().strokeEditMode() ==
             ZInteractiveContext::STROKE_DRAW) {
    buddyView()->setScreenCursor(
          ZCursorStore::getInstance().getSmallCrossCursor());
    //buddyView()->setScreenCursor(Qt::PointingHandCursor);
  } else {
    buddyView()->setScreenCursor(Qt::CrossCursor);
  }
}

void ZStackPresenter::selectAllSwcTreeNode()
{
  buddyDocument()->selectAllSwcTreeNode();
}

void ZStackPresenter::slotTest()
{
  std::cout << "action triggered" << std::endl;
  buddyDocument()->selectDownstreamNode();
}

void ZStackPresenter::selectDownstreamNode()
{
  buddyDocument()->selectDownstreamNode();
}

void ZStackPresenter::selectSwcNodeConnection()
{
  buddyDocument()->selectSwcNodeConnection();
}

void ZStackPresenter::selectUpstreamNode()
{
  buddyDocument()->selectUpstreamNode();
}

void ZStackPresenter::selectBranchNode()
{
  buddyDocument()->selectBranchNode();
}

void ZStackPresenter::selectTreeNode()
{
  buddyDocument()->selectTreeNode();
}

void ZStackPresenter::selectConnectedNode()
{
  buddyDocument()->selectConnectedNode();
}
