#ifndef Z3DWINDOW_H
#define Z3DWINDOW_H

#include <QMainWindow>
#include <vector>
#include <set>
#include <map>
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif
#include <QDir>
#include "zparameter.h"
#include "znumericparameter.h"
#include "zglmutils.h"
#include "z3dcameraparameter.h"
#include "zactionactivator.h"

class ZStackDoc;
class Z3DTrackballInteractionHandler;
class Z3DPunctaFilter;
class Z3DSwcFilter;
class Z3DVolumeSource;
class Z3DVolumeRaycaster;
class Z3DGraphFilter;
class ZPunctum;
class ZSwcTree;
struct _Swc_Tree_Node;
typedef _Swc_Tree_Node Swc_Tree_Node;
class Z3DCompositor;
class Z3DCanvasRenderer;
class Z3DTakeScreenShotWidget;
class Z3DAxis;
class ZWidgetsGroup;
class Z3DCanvas;
class Z3DNetworkEvaluator;
class Z3DProcessorNetwork;
class Z3DVolumeRaycasterRenderer;
class Z3DTriangleList;

class Z3DWindow : public QMainWindow
{
  Q_OBJECT
public:
  enum EInitMode {
    NORMAL_INIT, EXCLUDE_VOLUME
  };

  explicit Z3DWindow(std::tr1::shared_ptr<ZStackDoc> doc, EInitMode initMode,
                     bool stereoView = false, QWidget *parent = 0);
  virtual ~Z3DWindow();

  void gotoPosition(double x, double y, double z, double radius = 64);
  void gotoPosition(std::vector<double> bound, double minRadius = 64,
                    double range = 128);

  // useful stuff
  Z3DCameraParameter* getCamera();
  Z3DTrackballInteractionHandler* getInteractionHandler();

  inline Z3DCanvasRenderer* getCanvasRenderer() { return m_canvasRenderer; }
  inline Z3DPunctaFilter* getPunctaFilter() { return m_punctaFilter; }
  inline Z3DSwcFilter* getSwcFilter() { return m_swcFilter; }
  inline Z3DVolumeRaycaster* getVolumeRaycaster() { return m_volumeRaycaster; }

  Z3DVolumeRaycasterRenderer* getVolumeRaycasterRenderer();
  inline Z3DGraphFilter* getGraphFilter() { return m_graphFilter; }
  inline Z3DCompositor* getCompositor() { return m_compositor; }
  inline Z3DVolumeSource *getVolumeSource() { return m_volumeSource; }
  inline Z3DAxis *getAxis() { return m_axis; }
  const std::vector<double>& getBoundBox() const { return m_boundBox; }

  void updateVolumeBoundBox();
  void updateSwcBoundBox();
  void updateGraphBoundBox();
  void updatePunctaBoundBox();
  void updateOverallBoundBox(std::vector<double> bound);
  void updateOverallBoundBox();       //get bounding box of all objects in world coordinate :[xmin xmax ymin ymax zmin zmax]

  /*!
   * \brief Get the document associated with the window
   */
  inline ZStackDoc* getDocument() const { return m_doc.get(); }

protected:

private:
  // UI
  void createMenus();
  void createActions();
  void customizeContextMenu();
  void createContextMenu();
  void createStatusBar();
  void createDockWindows();
  void customizeDockWindows(QTabWidget *m_settingTab);
  void setWindowSize();

  // init 3D view
  void init(EInitMode mode = NORMAL_INIT);

  void cleanup();

  int channelNumber();

  void setupCamera(const std::vector<double> &bound, Z3DCamera::ResetCameraOptions options);

  bool hasVolume();

  //conditions for customization
  bool hasSwc() const;
  bool hasSelectedSwc() const;
  bool hasSelectedSwcNode() const;
  bool hasMultipleSelectedSwcNode() const;

signals:
  
public slots:
  void resetCamera();  // set up camera based on visible objects in scene, original position

  void flipView(); //Look from the oppsite side
  void recordView(); //Record the current view parameters
  void diffView(); //Output difference between current view and recorded view

  void resetCameraClippingRange(); // // Reset the camera clipping range to include this entire bounding box
  // redraw changed parts
  void volumeChanged();
  void swcChanged();
  void punctaChanged();
  void updateNetworkDisplay();
  void updateDisplay();

  void volumeScaleChanged();
  void swcCoordScaleChanged();
  void punctaCoordScaleChanged();
  void swcSizeScaleChanged();
  void punctaSizeScaleChanged();

  void selectedPunctumChangedFrom3D(ZPunctum* p, bool append);
  void selectedSwcChangedFrom3D(ZSwcTree* p, bool append);
  void selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node* p, bool append);
  void addNewSwcTreeNode(double x, double y, double z, double r);
  void extendSwcTreeNode(double x, double y, double z);
  void connectSwcTreeNode(Swc_Tree_Node *tn);
  void deleteSelectedSwcNode();
  void locateSwcNodeIn2DView();
  void removeSwcTurn();

  void convertSelectedChainToSwc();

  void punctaSelectionChanged();
  void swcSelectionChanged();
  void swcTreeNodeSelectionChanged();

  void swcDoubleClicked(ZSwcTree* tree);
  void swcNodeDoubleClicked(Swc_Tree_Node* node);
  void punctaDoubleClicked(ZPunctum* p);
  void pointInVolumeLeftClicked(QPoint pt, glm::ivec3 pos);

  void changeSelectedSwcNodeType();
  void setRootAsSelectedSwcNode();
  void breakSelectedSwcNode();
  void connectSelectedSwcNode();
  void startConnectingSwcNode();
  void mergeSelectedSwcNode();
  void tranlateSelectedSwcNode();
  void changeSelectedSwcNodeSize();
  void showSeletedSwcNodeLength();

  void saveSelectedSwc();
  void changeSelectedSwcType();
  void changeSelectedSwcSize();
  void transformSelectedSwc();
  void breakSelectedSwc();
  void groupSelectedSwc();
  void showSelectedSwcInfo();
  void refreshTraceMask();
  void test();
  void changeSelectedSwcColor();
  void changeSelectedSwcAlpha();

  void transformSelectedPuncta();
  void transformAllPuncta();
  void convertPunctaToSwc();

  //
  void show3DViewContextMenu(QPoint pt);

  // trace menu action
  void traceTube();

  void openZoomInView();
  void exitZoomInView();

  void removeSelectedObject();

  // puncta context menu action
  void markSelectedPunctaProperty1();
  void markSelectedPunctaProperty2();
  void markSelectedPunctaProperty3();
  void unmarkSelectedPunctaProperty1();
  void unmarkSelectedPunctaProperty2();
  void unmarkSelectedPunctaProperty3();
  void saveSelectedPunctaAs();
  void saveAllPunctaAs();
  void markPunctum();
  void locatePunctumIn2DView();

  void takeScreenShot(QString filename, int width, int height, Z3DScreenShotType sst);
  void takeScreenShot(QString filename, Z3DScreenShotType sst);

  void openAdvancedSetting(const QString &name);

  void takeSeriesScreenShot(const QDir& dir, const QString &namePrefix, glm::vec3 axis,
                            bool clockWise, int numFrame, int width, int height,
                            Z3DScreenShotType sst);
  void takeSeriesScreenShot(const QDir& dir, const QString &namePrefix, glm::vec3 axis,
                            bool clockWise, int numFrame, Z3DScreenShotType sst);

  void updateSettingsDockWidget();

  void toogleAddSwcNodeMode(bool checked);
  void toogleExtendSelectedSwcNodeMode(bool checked);
  void toogleSmartExtendSelectedSwcNodeMode(bool checked);
  void changeBackground();

  void toogleMoveSelectedObjectsMode(bool checked);
  void moveSelectedObjects(double x, double y, double z);

protected:
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
  virtual void keyPressEvent(QKeyEvent *event);

private:
  QTabWidget* createBasicSettingTabWidget();
  QTabWidget* createAdvancedSettingTabWidget();

  // update menu based on context information
  void updateContextMenu(const QString &group);

private:
  // menu
  std::map<QString, QMenu*> m_contextMenuGroup;
  QMenu *m_mergedContextMenu;

  QMenu *m_viewMenu;
  QMenu *m_editMenu;
  QAction *m_removeSelectedObjectsAction;
  QAction *m_openVolumeZoomInViewAction;
  QAction *m_exitVolumeZoomInViewAction;
  QAction *m_markPunctumAction;
  QAction *m_toogleAddSwcNodeModeAction;
  QAction *m_changeBackgroundAction;
  QAction *m_toogleMoveSelectedObjectsAction;
  QAction *m_toogleExtendSelectedSwcNodeAction;
  QAction *m_toogleSmartExtendSelectedSwcNodeAction;
  QAction *m_locateSwcNodeIn2DAction;

  QAction *m_undoAction;
  QAction *m_redoAction ;
  QAction *m_changeSwcNodeTypeAction;
  QAction *m_setSwcRootAction;
  QAction *m_breakSwcConnectionAction;
  QAction *m_connectSwcNodeAction;
  QAction *m_connectToSwcNodeAction;
  QAction *m_mergeSwcNodeAction;
  QAction *m_selectSwcNodeDownstreamAction;
  QAction *m_selectSwcConnectionAction;
  QAction *m_selectSwcNodeBranchAction;
  QAction *m_selectSwcNodeUpstreamAction;
  QAction *m_selectSwcNodeTreeAction;
  QAction *m_selectAllConnectedSwcNodeAction;
  QAction *m_selectAllSwcNodeAction;
  QAction *m_translateSwcNodeAction;
  QAction *m_changeSwcNodeSizeAction;

  QAction *m_refreshTraceMaskAction;

  QAction *m_saveSwcAction;
  QAction *m_changeSwcTypeAction;
  QAction *m_changeSwcSizeAction;
  QAction *m_transformSwcAction;
  QAction *m_groupSwcAction;
  QAction *m_breakForestAction;
  QAction *m_changeSwcColorAction;
  QAction *m_changeSwcAlphaAction;
  QAction *m_swcInfoAction;
  QAction *m_swcNodeLengthAction;
  QAction *m_removeSwcTurnAction;
  QAction *m_resolveCrossoverAction;

  QAction *m_saveSelectedPunctaAsAction;
  QAction *m_saveAllPunctaAsAction;
  QAction *m_locatePunctumIn2DAction;

  /*
  QMenu *m_punctaContextMenu;
  QMenu *m_traceMenu;
  QMenu *m_volumeContextMenu;
  QMenu *m_swcContextMenu;
*/

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;


  std::tr1::shared_ptr<ZStackDoc> m_doc;

  Z3DNetworkEvaluator *m_networkEvaluator;
  Z3DCanvas *m_canvas;

  // processors
  Z3DCanvasRenderer *m_canvasRenderer;
  Z3DAxis *m_axis;
  Z3DVolumeRaycaster *m_volumeRaycaster;
  Z3DPunctaFilter *m_punctaFilter;
  Z3DCompositor *m_compositor;
  Z3DSwcFilter *m_swcFilter;
  Z3DVolumeSource *m_volumeSource;
  Z3DGraphFilter *m_graphFilter;

  std::vector<double> m_volumeBoundBox;
  std::vector<double> m_swcBoundBox;
  std::vector<double> m_punctaBoundBox;
  std::vector<double> m_graphBoundBox;
  std::vector<double> m_boundBox;    //overall bound box

  bool m_isClean;   //already cleanup?

  glm::ivec3 m_lastClickedPosInVolume;

  Z3DTakeScreenShotWidget *m_screenShotWidget;

  ZWidgetsGroup *m_widgetsGroup;

  QDockWidget *m_settingsDockWidget;
  QDockWidget *m_objectsDockWidget;
  QDockWidget *m_advancedSettingDockWidget;

  bool m_isStereoView;

  Z3DCamera m_cameraRecord;

  QString m_lastOpenedFilePath;
};

#endif // Z3DWINDOW_H
