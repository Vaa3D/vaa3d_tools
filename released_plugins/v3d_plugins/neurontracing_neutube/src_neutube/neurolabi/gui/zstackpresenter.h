/**@file zstackpresenter.h
 * @brief Stack presenter
 * @author Ting Zhao
 */

#ifndef _ZSTACKPRESENTER_H_
#define _ZSTACKPRESENTER_H_

#include <QPoint>
#include <QSize>
#include <QObject>
#include <vector>
#include <QMap>

#include "zstackdrawable.h"
#include "zinteractivecontext.h"
#include "zstroke2d.h"
#include "swctreenode.h"
#include "zactionactivator.h"

class ZStackView;
class ZStackDoc;
class ZStackFrame;
class QMouseEvent;
class QKeyEvent;
class QAction;
class QMenu;

class ZStackPresenter : public QObject {
  Q_OBJECT

public:
  ZStackPresenter(ZStackFrame *parent = 0);
  ~ZStackPresenter();
  ZStackDoc* buddyDocument() const;
  ZStackView* buddyView() const;

  void updateView() const;

  enum MouseButtonAction {
    LEFT_RELEASE, RIGHT_RELEASE, LEFT_PRESS, RIGHT_PRESS, LEFT_DOUBLE_CLICK, MOVE
  };

  enum EMouseEventProcessStatus {
    MOUSE_EVENT_PASSED, CONTEXT_MENU_POPPED, MOUSE_HIT_OBJECT,
    MOUSE_COMMAND_EXECUTED, MOUSE_EVENT_CAPTURED
  };

  enum EActionItem {
    ACTION_EXTEND_SWC_NODE, ACTION_SMART_EXTEND_SWC_NODE,
    ACTION_CONNECT_TO_SWC_NODE, ACTION_ADD_SWC_NODE,
    ACTION_LOCK_SWC_NODE_FOCUS, ACTION_ESTIMATE_SWC_NODE_RADIUS,
    ACTION_PAINT_STROKE, ACTION_ERASE_STROKE
  };

  inline double greyScale(int c = 0) const {return m_greyScale[c];}
  inline double greyOffset(int c = 0) const {return m_greyOffset[c];}
  //inline int zoomRatio() const { return m_zoomRatio; }
  int zoomRatio() const;
  inline QList<ZStackDrawable*>* decorations() { return &m_decorationList; }
  inline const QList<ZStackDrawable*>& getActiveDecorationList() const {
    return m_activeDecorationList;
  }
  inline ZStackDrawable::Display_Style objectStyle() { return m_objStyle; }
  inline ZInteractiveContext& interactiveContext() {
    return m_interactiveContext;
  }

  bool hasObjectToShow() const;
  void setObjectVisible(bool v);
  bool isObjectVisible();
  void setObjectStyle(ZStackDrawable::Display_Style style);

  void initInteractiveContext();

  /*
  void updateZoomOffset(int cx, int cy, int r0);
  void updateZoomOffset(int cx, int cy, int wx, int wy);
  void setZoomOffset(int x, int y);
*/
  void processMouseReleaseEvent(QMouseEvent *event, int sliceIndex);
  void processKeyPressEvent(QKeyEvent *event);
  bool processKeyPressEventForSwc(QKeyEvent *event);
  bool processKeyPressEventForStroke(QKeyEvent *event);
  void processMouseMoveEvent(QMouseEvent *event);
  void processMousePressEvent(QMouseEvent *event);
  void processMouseDoubleClickEvent(QMouseEvent *eventint, int sliceIndex);

  void createActions();
  void createTraceActions();
  void createPunctaActions();
  void createSwcActions();
  void createTubeActions();
  void createStrokeActions();
  void createDocDependentActions();

  inline QAction* getAction(EActionItem item) const {
    return m_actionMap[item];
  }

  void createSwcNodeContextMenu();
  QMenu* getSwcNodeContextMenu();

  void createStrokeContextMenu();
  QMenu* getStrokeContextMenu();

  void setStackBc(double scale, double offset, int c = 0);

  /* optimize stack brightness and contrast */
  void optimizeStackBc();

  void autoThreshold();
  void binarizeStack();
  void solidifyStack();

  void autoTrace();

  void prepareView();

  void updateLeftMenu(QAction *action, bool clear = true);
  void updateRightMenu(QAction *action, bool clear = true);
  void updateRightMenu(QMenu *submenu, bool clear = true);
  void updateLeftMenu();

  void addTubeEditFunctionToRightMenu();
  void addPunctaEditFunctionToRightMenu();
  void addSwcEditFunctionToRightMenu();

  void setViewPortCenter(int x, int y, int z);

  const QPoint dataPositionFromMouse(MouseButtonAction mba);

  QStringList toStringList() const;

  //void enterSwcEditMode();

  void updateCursor();

  inline const ZStroke2d& getStroke() const { return m_stroke; }

  void setZoomRatio(int ratio);

public slots:
  void addDecoration(ZStackDrawable *obj, bool tail = true);
  void removeLastDecoration(ZInterface *obj);
  void removeDecoration(ZStackDrawable *obj, bool redraw = true);
  void removeAllDecoration();
  void traceTube();
  void fitSegment();
  void fitEllipse();
  void dropSegment();
  void enterHookMode();
  void enterSpHookMode();
  void enterLinkMode();
  void enterMergeMode();
  void enterWalkMode();
  void enterCheckConnMode();
  void enterExtendMode();
  void enterConnectMode();
  void enterDisconnectMode();
  void enterMouseCapturingMode();
  void cutTube();
  void breakTube();
  void refineChainEnd();
  void bringSelectedToFront();
  void sendSelectedToBack();
  void selectNeighbor();
  void selectConnectedTube();
  void markPuncta();
  void deleteSelected();
  void deleteAllPuncta();
  void enlargePuncta();
  void narrowPuncta();
  void meanshiftPuncta();
  void meanshiftAllPuncta();
  void updateStackBc();

  void enterSwcConnectMode();
  void enterSwcExtendMode();
  void exitSwcExtendMode();
  void enterSwcSmartExtendMode();
  void enterSwcAddNodeMode(double x, double y);
  void enterSwcSelectMode();
  void enterDrawStrokeMode(double x, double y);
  void enterEraseStrokeMode(double x, double y);
  void exitStrokeEdit();
  void deleteSwcNode();
  void lockSelectedSwcNodeFocus();
  void processSliceChangeEvent(int z);
  void estimateSelectedSwcRadius();
  void connectSelectedSwcNode();
  void breakSelectedSwcNode();
  void selectAllSwcTreeNode();

  void trySwcAddNodeMode(double x, double y);
  void tryPaintStrokeMode();
  void tryEraseStrokeMode();
  void tryDrawStrokeMode(double x, double y, bool isEraser);

  void selectDownstreamNode();
  void selectSwcNodeConnection();
  void selectUpstreamNode();
  void selectBranchNode();
  void selectTreeNode();
  void selectConnectedNode();

  void slotTest();

  /*!
   * \brief Turn on the active stroke
   */
  void turnOnStroke();


  /*!
   * \brief Turn off the active stroke
   */
  void turnOffStroke();

  inline bool isStrokeOn() { return m_isStrokeOn; }
  inline bool isStrokeOff() { return !isStrokeOn(); }

  const Swc_Tree_Node* getSelectedSwcNode() const;

signals:
  void mousePositionCaptured(int x, int y, int z);

private:
  EMouseEventProcessStatus processMouseReleaseForPuncta(
      QMouseEvent *event, int *positionInStack);
  EMouseEventProcessStatus processMouseReleaseForTube(
      QMouseEvent *event, int *positionInStack);
  EMouseEventProcessStatus processMouseReleaseForSwc(
      QMouseEvent *event, int *positionInStack);
  EMouseEventProcessStatus processMouseReleaseForStroke(
      QMouseEvent *event, int *positionInStack);
  bool isPointInStack(double x, double y);
  QPoint mapFromWidgetToStack(const QPoint &pos);
  QPoint mapFromGlobalToStack(const QPoint &pos);

private:
  ZStackFrame *m_parent;
  QList<ZStackDrawable*> m_decorationList;
  QList<ZStackDrawable*> m_activeDecorationList;

  //Mode m_mode;
  //InteractiveMode m_intMode[2];
  //int m_zoomRatio;
  //QPoint m_zoomOffset;
  bool m_showObject;
  std::vector<double> m_greyScale;
  std::vector<double> m_greyOffset;
  int m_threshold;
  ZStackDrawable::Display_Style m_objStyle;
  //MouseState m_mouseState;
  bool m_mouseLeftButtonPressed;
  bool m_mouseRightButtonPressed;
  ZInteractiveContext m_interactiveContext;
  int m_cursorRadius;

  //actions
  QAction *m_traceAction;
  QAction *m_fitsegAction;
  QAction *m_fitEllipseAction;
  QAction *m_dropsegAction;
  QAction *m_cutAction;
  QAction *m_breakAction;
  QAction *m_hookAction;
  QAction *m_spHookAction;
  QAction *m_linkAction;
  QAction *m_mergeAction;
  QAction *m_frontAction;
  QAction *m_backAction;
  QAction *m_refineEndAction;
  QAction *m_connectAction;
  QAction *m_extendAction;
  QAction *m_walkAction;
  QAction *m_checkConnAction;
  QAction *m_disconnectAction;
  QAction *m_neighborAction;
  QAction *m_selectConnectedTubeAction;
  QAction *m_markPunctaAction;
  QAction *m_deleteSelectedAction;
  QAction *m_deleteAllPunctaAction;
  QAction *m_enlargePunctaAction;
  QAction *m_narrowPunctaAction;
  QAction *m_meanshiftPunctaAction;
  QAction *m_meanshiftAllPunctaAction;

  QAction *m_swcConnectToAction;
  QAction *m_swcExtendAction;
  QAction *m_swcSmartExtendAction;
  QAction *m_swcDeleteAction;
  QAction *m_swcConnectSelectedAction;
  QAction *m_swcSelectConnectionAction;
  QAction *m_swcLockFocusAction;
  QAction *m_swcEstimateRadiusAction;
  //QAction *m_swcSelectAllNodeAction;
  QAction *m_swcBreakSelectedAction;

  QAction *m_selectSwcNodeDownstreamAction;
  QAction *m_selectSwcConnectionAction;
  QAction *m_selectSwcNodeBranchAction;
  QAction *m_selectSwcNodeUpstreamAction;
  QAction *m_selectSwcNodeTreeAction;
  QAction *m_selectAllConnectedSwcNodeAction;
  QAction *m_selectAllSwcNodeAction;

  QAction *m_paintStrokeAction;
  QAction *m_eraseStrokeAction;

  //  Action map
  QMap<EActionItem, QAction*> m_actionMap;

  QMenu *m_swcNodeContextMenu;
  QMenu *m_strokePaintContextMenu;

  //recorded information
  int m_mouseMovePosition[3];
  int m_mouseLeftReleasePosition[3];
  int m_mouseRightReleasePosition[3];
  int m_mouseLeftPressPosition[3];
  int m_mouseRightPressPosition[3];
  int m_mouseLeftDoubleClickPosition[3];
  QPoint m_grabPosition;

  ZStroke2d m_stroke;
  bool m_isStrokeOn;

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;

signals:
  void viewModeChanged();
};

#endif
