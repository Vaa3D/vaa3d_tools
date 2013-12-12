/**@file zstackdoc.h
 * @brief Stack document
 * @author Ting Zhao
 */
#ifndef _ZSTACKDOC_H_
#define _ZSTACKDOC_H_

/**@file zstackdoc.h
 * @brief Stack document
 * @author Ting Zhao
 */

#include <QString>
#include <QList>
#include <QUrl>
#include <set>
#include <QObject>
#include <QUndoCommand>
#include <QMap>
#include <string>

#include "neutube.h"
#include "zcurve.h"
#include "zswctree.h"
#include "zobject3d.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain.h"
#include "tz_trace_defs.h"
#include "zpunctum.h"
#include "zprogressreporter.h"
#include "zrescaleswcdialog.h"
#include "zreportable.h"
#include "biocytin/zstackprojector.h"
#include "zstackreadthread.h"
#include "zstackfile.h"

class ZStackFrame;
class ZInterface;
class ZDocumentable;
class ZSwcExportable;
class ZVrmlExportable;
class ZLocalNeuroseg;
class ZStackDrawable;
class ZLocsegChain;
class ZLocsegChainConn;
class QXmlStreamReader;
class QProgressBar;
class ZStack;
class ZResolution;
class ZSwcNetwork;
class ZSwcObjsModel;
class ZPunctaObjsModel;
class ZStroke2d;
class QWidget;
class ZSwcNodeObjsModel;

class ZStackDoc : public QObject, public ZReportable, public ZProgressable
{
  Q_OBJECT

public:
  ZStackDoc(ZStack *stack = 0);
  virtual ~ZStackDoc();

  enum TubeImportOption {
    ALL_TUBE,
    GOOD_TUBE,
    BAD_TUBE
  };

  enum LoadObjectOption {
    REPLACE_OBJECT,
    APPEND_OBJECT
  };

  enum EComponent {
    STACK, STACK_MASK, STACK_SEGMENTATION, SEGMENTATION_OBJECT,
    SEGMENTATION_GRAPH, SEGMENTATION_INDEX_MAP
  };

  enum EDocumentDataType {
    SWC_DATA, PUNCTA_DATA, STACK_DATA, NETWORK_DATA
  };

  enum EActionItem {
    ACTION_MEASURE_SWC_NODE_LENGTH, ACTION_SWC_SUMMARIZE,
    ACTION_SELECT_DOWNSTREAM, ACTION_SELECT_UPSTREAM,
    ACTION_CHANGE_SWC_TYPE, ACTION_CHANGE_SWC_SIZE, ACTION_REMOVE_TURN,
    ACTION_RESOLVE_CROSSOVER
  };

public: //attributes
  // isEmpty() returns true iff it has no stack data or object.
  bool isEmpty();

  // hasStackData() returns true iff it has stack array data.
  bool hasStackData() const;
  bool hasStackMask();

  // hasTracable() returns true iff it has tracable data.
  bool hasTracable();

  // hasObject() returns true iff it has an object.
  bool hasObject();
  // hasSwc() returns true iff it has an SWC object.
  bool hasSwc();
  // hasDrawable() returns true iff it has a drawable object.
  bool hasDrawable();

  bool hasSelectedSwc() const;
  bool hasSelectedSwcNode() const;
  bool hasMultipleSelectedSwcNode() const;

  int stackWidth() const;
  int stackHeight() const;
  int stackChannelNumber() const;

  virtual void deprecateDependent(EComponent component);
  virtual void deprecate(EComponent component);
  virtual bool isDeprecated(EComponent component);

  // Prefix for tracing project.
  const char *tubePrefix() const;

  inline QList<ZStackDrawable*>* drawableList() { return &m_drawableList; }

  inline QList<ZSwcTree*>* swcList() {return &m_swcList;}
  bool hasSwcList();       //to test swctree
  inline QList<ZLocsegChain*>* chainList() {return &m_chainList;}
  inline QList<ZLocsegChainConn*>* connList() {return &m_connList;}
  inline QList<ZPunctum*>* punctaList() {return &m_punctaList;}
  inline ZSwcObjsModel* swcObjsModel() {return m_swcObjsModel;}
  inline ZSwcNodeObjsModel* swcNodeObjsModel() {return m_swcNodeObjsModel;}
  inline ZPunctaObjsModel* punctaObjsModel() {return m_punctaObjsModel;}
  inline std::set<ZPunctum*>* selectedPuncta() {return &m_selectedPuncta;}
  inline std::set<ZLocsegChain*>* selectedChains() {return &m_selectedChains;}
  inline std::set<ZSwcTree*>* selectedSwcs() {return &m_selectedSwcs;}
  inline const std::set<ZSwcTree*>* selectedSwcs() const {return &m_selectedSwcs;}
  inline std::set<Swc_Tree_Node*>* selectedSwcTreeNodes() {return &m_selectedSwcTreeNodes;}
  inline const std::set<Swc_Tree_Node*>* selectedSwcTreeNodes() const {
    return &m_selectedSwcTreeNodes;}
  inline ZSwcNetwork* swcNetwork() { return m_swcNetwork; }
  ZResolution stackResolution() const;
  QString stackSourcePath() const;
  bool hasChainList();

  //void setStackMask(ZStack *stack);

  void createActions();
  inline QAction* getAction(EActionItem item) {
    return m_actionMap[item];
  }

  bool isUndoClean();

  /*
  void setProgressReporter(ZProgressReporter *reporter);
  inline ZProgressReporter* getProgressReporter() {
    return m_progressReporter;
  }
*/
public: //swc tree edit
  // move soma (first root) to new location
  void swcTreeTranslateRootTo(double x, double y, double z);
  // rescale location and radius
  void swcTreeRescale(double scaleX, double scaleY, double scaleZ);
  void swcTreeRescale(double srcPixelPerUmXY, double srcPixelPerUmZ, double dstPixelPerUmXY, double dstPixelPerUmZ);
  // rescale radius of nodes of certain depth
  void swcTreeRescaleRadius(double scale, int startdepth, int enddepth);
  // reduce node number, similar to Swc_Tree_Merge_Close_Node, but only merge Continuation node
  void swcTreeReduceNodeNumber(double lengthThre);
  void updateVirtualStackSize();

  void deleteSelectedSwcNode();
  void addSizeForSelectedSwcNode(double dr);

  void estimateSwcRadius();

public: //swc selection
  //Select connection
  //void selectSwcNodeConnection(); // change to slot
  void selectSwcNodeNeighbor();
  std::string getSwcSource() const;

public:
  void loadFileList(const QList<QUrl> &urlList);
  void loadFileList(const QStringList &filePath);
  void loadFile(const QString &filePath, bool emitMessage = false);
  virtual void loadStack(Stack *stack, bool isOwner = true);
  virtual void loadStack(ZStack *zstack);
  virtual ZStack*& stackRef();

  void readStack(const char *filePath);
  void readSwc(const char *filePath);

  void saveSwc(QWidget *parentWidget);

  inline ZStackFrame* getParentFrame() { return m_parentFrame; }
  void setParentFrame(ZStackFrame* parent);

  virtual ZStack* stack() const;
  virtual ZStack *stackMask() const;
  inline QList<ZDocumentable*>* objects() { return &m_objs; }
  void setStackSource(const char *filePath);
  void loadSwcNetwork(const QString &filePath);
  void loadSwcNetwork(const char *filePath);
  bool importImageSequence(const char *filePath);

  void loadSwc(const QString &filePath);
  void loadLocsegChain(const QString &filePath);

  void importFlyEmNetwork(const char *filePath);

  void exportSwc(const char *filePath);
  void exportVrml(const char *filePath);
  void exportSvg(const char *filePath);
  void exportBinary(const char *prefix);
  void exportSwcTree(const char *filePath);
  int exportMultipleSwcTree(const QString &filepath);   //if we want each tree seperated
  void exportChainFileList(const char *filepath);
  void exportPuncta(const char *filePath);
  void exportObjectMask(const std::string &filePath);

  //Those functions do not notify object modification
  void removeLastObject(bool deleteObject = false);
  void removeAllObject(bool deleteObject = true);
  void removeObject(ZInterface *obj, bool deleteObject = false);
  void removeSelectedObject(bool deleteObject = false);

  void removeSelectedPuncta(bool deleteObject = false);
  void removeLocsegChain(ZInterface *obj);
  void removeSmallLocsegChain(double thre);   //remove small locseg chain (geolen < thre)
  void removeAllLocsegChain();
  std::set<ZSwcTree*> removeEmptySwcTree(bool deleteObject = true);
  void removeAllSwcTree(bool deleteObject = true);

  void addDocumentable(ZDocumentable *obj);
  void appendSwcNetwork(ZSwcNetwork &network);

  QString toString();
  QStringList toStringList() const;
  virtual QString dataInfo(int x, int y, int z) const;

  ZCurve locsegProfileCurve(int option) const;

  void cutLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain*> *pResult = NULL);   //optional output cut result
  void cutSelectedLocsegChain();
  void breakLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain*> *pResult = NULL);  //optional output break result
  void breakSelectedLocsegChain();

  static int autoThreshold(Stack* stack);
  int autoThreshold();
  bool binarize(int threshold);
  bool bwsolid();
  bool enhanceLine();
  bool watershed();
  bool invert();
  int findLoop();
  void bwthin();

  int maxIntesityDepth(int x, int y);
  ZStack* projectBiocytinStack(Biocytin::ZStackProjector &projector);

  void updateStackFromSource();

public: /* tracing routines */
  ZLocsegChain* fitseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitRpiseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitRect(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitEllipse(int x, int y, int z, double r = 1.0);
  ZLocsegChain* dropseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* traceTube(int x, int y, int z, double r = 3.0, int c = 0);
  ZLocsegChain* traceRect(int x, int y, int z, double r = 3.0, int c = 0);

  void refreshTraceMask();

public: /* puncta related methods */
  ZPunctum* markPunctum(int x, int y, int z, double r = 2.0);
  int pickPunctaIndex(int x, int y, int z) const;
  bool selectPuncta(int index);
  bool deleteAllPuncta();
  bool expandSelectedPuncta();
  bool shrinkSelectedPuncta();
  bool meanshiftSelectedPuncta();
  bool meanshiftAllPuncta();
  inline bool hasSelectedPuncta() {return !m_selectedPuncta.empty();}

  void addLocsegChain(ZLocsegChain *chain);
  void addSwcTree(ZSwcTree *obj, bool uniqueSource = true);
  void addSwcTree(const QList<ZSwcTree*> &swcList, bool uniqueSource = true);
  void addPunctum(ZPunctum *obj);
  void addObj3d(ZObject3d *obj);
  void addStroke(ZStroke2d *obj);

  void addObject(ZDocumentable *obj, NeuTube::EDocumentableType type);

  void updateLocsegChain(ZLocsegChain *chain);
  void importLocsegChain(const QStringList &files,
                         TubeImportOption option = ALL_TUBE,
                         LoadObjectOption objopt = APPEND_OBJECT);
  void importGoodTube(const char *dirpath = NULL, const char *prefix = NULL,
                      QProgressBar *pb = NULL);
  void importBadTube(const char *dirpath, const char *prefix = NULL);
  void importSwc(QStringList files, LoadObjectOption objopt = APPEND_OBJECT);
  void importPuncta(const QStringList &files,
                    LoadObjectOption objopt = APPEND_OBJECT);

  bool importPuncta(const char *filePath);

  int pickLocsegChainId(int x, int y, int z) const;
  void holdClosestSeg(int id, int x, int y, int z);
  int selectLocsegChain(int id, int x = -1, int y = -1, int z = -1,
  		bool showProfile = false);
  bool selectSwcTreeBranch(int x, int y, int z);
  bool pushLocsegChain(ZInterface *obj);
  void pushSelectedLocsegChain();
  bool fixLocsegChainTerminal(ZInterface *obj);

  bool importSynapseAnnotation(const std::string &filePath);

  Swc_Tree_Node *swcHitTest(int x, int y, int z);
  Swc_Tree_Node *selectSwcTreeNode(int x, int y, int z, bool append = false);
  void selectSwcTreeNode(Swc_Tree_Node *tn, bool append = false);

  // (de)select objects and emit signals for 3D view and 2D view to sync
  void setPunctumSelected(ZPunctum* punctum, bool select);
  template <class InputIterator>
  void setPunctumSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllPuncta();
  void setChainSelected(ZLocsegChain* chain, bool select);
  void setChainSelected(const std::vector<ZLocsegChain*> &chains, bool select);
  void deselectAllChains();
  void setSwcSelected(ZSwcTree* tree, bool select);
  template <class InputIterator>
  void setSwcSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllSwcs();
  void setSwcTreeNodeSelected(Swc_Tree_Node* tn, bool select);
  template <class InputIterator>
  void setSwcTreeNodeSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllSwcTreeNodes();
  void deselectAllObject();

  // show/hide objects and emit signals for 3D view and 2D view to sync
  void setPunctumVisible(ZPunctum* punctum, bool visible);
  void setChainVisible(ZLocsegChain* chain, bool visible);
  void setSwcVisible(ZSwcTree* tree, bool visible);

  void setTraceMinScore(double score);
  void setReceptor(int option, bool cone = false);

  void updateMasterLocsegChain();
  bool linkChain(int id);
  bool hookChain(int id, int option = 0);
  bool mergeChain(int id);
  bool connectChain(int id);
  bool disconnectChain(int id);
  bool isMasterChainId(int id);
  inline bool isMasterChain(const ZLocsegChain *chain) const {
    return m_masterChain == chain;
  }
  inline void setMasterChain(ZLocsegChain *chain) {
    m_masterChain = chain;
  }

  void eraseTraceMask(const ZLocsegChain *chain);

  bool chainShortestPath(int id);
  void chainConnInfo(int id);

  void extendChain(double x, double y, double z);

  void addLocsegChainConn(ZLocsegChain *hook, ZLocsegChain *loop);
  void addLocsegChainConn(ZLocsegChain *hook, ZLocsegChain *loop, int hookSpot,
                          int loopSpot, int mode = NEUROCOMP_CONN_HL);
  void addLocsegChainConn(ZLocsegChainConn *obj);

  ZInterface* bringChainToFront();
  ZInterface* sendChainToBack();
  void refineSelectedChainEnd();
  void refineLocsegChainEnd();
  void mergeAllChain();

  void importLocsegChainConn(const char *filePath);
  void exportLocsegChainConn(const char *filePath);
  void exportLocsegChainConnFeat(const char *filePath);

  void buildLocsegChainConn();
  void clearLocsegChainConn();
  void selectNeighbor();
  void selectConnectedChain();

  void setWorkdir(const QString &filePath);
  void setWorkdir(const char *filePath);
  void setTubePrefix(const char *filePath);
  void setBadChainScreen(const char *screen);

  void autoTrace();
  void autoTrace(Stack* stack);
  void traceFromSwc(QProgressBar *pb = NULL);

  void test(QProgressBar *pb = NULL);

  inline QUndoStack* undoStack() const { return m_undoStack; }
  inline void pushUndoCommand(QUndoCommand *command) { m_undoStack->push(command); }

  inline std::string additionalSource() { return m_additionalSource; }
  inline void setAdditionalSource(const std::string &filePath) {
    m_additionalSource = filePath;
  }

  bool hasObjectSelected();

  inline const QList<ZDocumentable*>& getObjectList() const { return m_objs; }
  inline QList<ZDocumentable*>& getObjectList() { return m_objs; }

  inline const QList<ZObject3d*>& getObj3dList() const { return m_obj3dList; }
  inline QList<ZObject3d*>& getObj3dList() { return m_obj3dList; }

  inline const QList<ZLocsegChain*>& getChainList() const { return m_chainList; }
  inline QList<ZLocsegChain*>& getChainList() { return m_chainList; }

  inline const QList<ZPunctum*>& getPunctaList() const { return m_punctaList; }
  inline QList<ZPunctum*>& getPunctaList() { return m_punctaList; }

  inline const QList<ZStroke2d*>& getStrokeList() const { return m_strokeList; }
  inline QList<ZStroke2d*>& getStrokeList() { return m_strokeList; }

  inline const QList<ZLocsegChainConn*>& getConnList() const { return m_connList; }
  inline QList<ZLocsegChainConn*>& getConnList() { return m_connList; }

  inline const QList<ZStackDrawable*>& getDrawableList() const { return m_drawableList; }
  inline QList<ZStackDrawable*>& getDrawableList() { return m_drawableList; }

  inline const QList<ZSwcTree*>& getSwcList() const { return m_swcList; }
  inline QList<ZSwcTree*>& getSwcList() { return m_swcList; }
  inline ZSwcTree* getSwcTree(size_t index) { return m_swcList[index]; }
  QList<ZSwcTree*>::iterator getSwcIteratorBegin() { return m_swcList.begin(); }
  QList<ZSwcTree*>::iterator getSwcIteratorEnd() { return m_swcList.end(); }
  QList<ZSwcTree*>::const_iterator getSwcIteratorBegin() const {
    return m_swcList.begin(); }
  QList<ZSwcTree*>::const_iterator getSwcIteratorEnd() const {
    return m_swcList.end(); }

  std::vector<ZSwcTree*> getSwcArray() const;
  bool getLastStrokePoint(int *x, int *y) const;

  void updateModelData(EDocumentDataType type);

public:
  inline void deprecateTraceMask() { m_isTraceMaskObsolete = true; }
  void updateTraceWorkspace(int traceEffort, bool traceMasked,
                            double xRes, double yRes, double zRes);
  void updateConnectionTestWorkspace(double xRes, double yRes, double zRes,
                                     char unit, double distThre, bool spTest,
                                     bool crossoverTest);

  inline Trace_Workspace* getTraceWorkspace() { return m_traceWorkspace; }

  /*
  inline ZSwcTree* previewSwc() { return m_previewSwc; }
  void updatePreviewSwc();
  */
  void notifyObjectModified();

  /*!
   * \brief Notify any connect slots about the modification of SWC objects
   *
   * It explicitly deprecate all intermediate components of all the SWC objects
   */
  void notifySwcModified();

  void notifyPunctumModified();
  void notifyChainModified();
  void notifyObj3dModified();
  void notifyStackModified();
  void notifyStrokeModified();
  void notifyAllObjectModified();

public:
  inline QAction* getUndoAction() { return m_undoAction; }
  inline QAction* getRedoAction() { return m_redoAction; }

public slots: //undoable commands
  bool executeAddObjectCommand(ZDocumentable *obj, NeuTube::EDocumentableType type);
  bool executeRemoveObjectCommand();
  //bool executeRemoveUnselectedObjectCommand();
  bool executeMoveObjectCommand(
      double x, double y, double z,
      double punctaScaleX, double punctaScaleY, double punctaScaleZ,
      double swcScaleX, double swcScaleY, double swcScaleZ);

  bool executeTraceTubeCommand(double x, double y, double z, int c = 0);
  bool executeRemoveTubeCommand();
  bool executeAutoTraceCommand();
  bool executeAutoTraceAxonCommand();

  bool executeAddSwcCommand(ZSwcTree *tree);
  void executeSwcRescaleCommand(const ZRescaleSwcSetting &setting);
  bool executeSwcNodeExtendCommand(const ZPoint &center);
  bool executeSwcNodeExtendCommand(const ZPoint &center, double radius);
  bool executeSwcNodeSmartExtendCommand(const ZPoint &center);
  bool executeSwcNodeSmartExtendCommand(const ZPoint &center, double radius);
  bool executeSwcNodeChangeZCommand(double z);
  bool executeSwcNodeEstimateRadiusCommand();
  bool executeMoveSwcNodeCommand(double dx, double dy, double dz);
  bool executeDeleteSwcNodeCommand();
  bool executeConnectSwcNodeCommand();
  bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn);
  bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  bool executeBreakSwcConnectionCommand();
  bool executeAddSwcNodeCommand(const ZPoint &center, double radius);
  bool executeSwcNodeChangeSizeCommand(double dr);
  bool executeMergeSwcNodeCommand();
  bool executeTraceSwcBranchCommand(double x, double y, double z, int c = 0);
  bool executeBreakForestCommand();
  bool executeGroupSwcCommand();
  bool executeSetRootCommand();
  bool executeRemoveTurnCommand();
  bool executeResolveCrossoverCommand();

  bool executeBinarizeCommand(int thre);
  bool executeBwsolidCommand();
  bool executeEnhanceLineCommand();
  bool executeWatershedCommand();

  bool executeAddStrokeCommand(ZStroke2d *stroke);

public slots:
  void selectAllSwcTreeNode();
  void autoSave();
  bool saveSwc(const std::string &filePath);
  void loadReaderResult();
  void selectDownstreamNode();
  void selectSwcNodeConnection();
  void selectUpstreamNode();
  void selectBranchNode();
  void selectTreeNode();
  void selectConnectedNode();

  void hideSelectedPuncta();
  void showSelectedPuncta();

/*
public:
  inline void notifyStackModified() {
    emit stackModified();
  }
*/
signals:
  void locsegChainSelected(ZLocsegChain*);
  void stackDelivered(Stack *stack, bool beOwner);
  void frameDelivered(ZStackFrame *frame);
  void stackModified();
  void stackReadDone();
  void stackLoaded();
  void punctaModified();
  void swcModified();
  void chainModified();
  void obj3dModified();
  void strokeModified();
  void objectModified();
  void swcNetworkModified();
  void punctaSelectionChanged(QList<ZPunctum*> selected,
                              QList<ZPunctum*> deselected);
  void chainSelectionChanged(QList<ZLocsegChain*> selected,
                             QList<ZLocsegChain*> deselected);
  void swcSelectionChanged(QList<ZSwcTree*> selected,
                           QList<ZSwcTree*> deselected);
  void swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*> selected,
                                   QList<Swc_Tree_Node*> deselected);
  void punctumVisibleStateChanged(ZPunctum* punctum, bool visible);
  void chainVisibleStateChanged(ZLocsegChain* chain, bool visible);
  void swcVisibleStateChanged(ZSwcTree* swctree, bool visible);
  void cleanChanged(bool);
  void holdSegChanged();

private:
  void connectSignalSlot();
  void initTraceWorkspace();
  void initConnectionTestWorkspace();
  void loadTraceMask(bool traceMasked);
  int xmlConnNode(QXmlStreamReader *xml, QString *filePath, int *spot);
  int xmlConnMode(QXmlStreamReader *xml);
  Swc_Tree* swcReconstruction(int rootOption, bool singleTree,
                              bool removingOvershoot);
  ZSwcTree* nodeToSwcTree(Swc_Tree_Node* node) const;

private:
  //Main stack
  ZStack *m_stack;

  //Concrete objects
  QList<ZSwcTree*> m_swcList;
  QList<ZPunctum*> m_punctaList;
  QList<ZStroke2d*> m_strokeList;
  QList<ZObject3d*> m_obj3dList;

  //Roles
  QList<ZDocumentable*> m_objs;
  QList<ZStackDrawable*> m_drawableList;

  //Subset of selected objects
  std::set<ZPunctum*> m_selectedPuncta;
  std::set<ZSwcTree*> m_selectedSwcs;
  std::set<Swc_Tree_Node*> m_selectedSwcTreeNodes;

  //model-view structure for obj list and edit
  ZSwcObjsModel *m_swcObjsModel;
  ZSwcNodeObjsModel *m_swcNodeObjsModel;
  ZPunctaObjsModel *m_punctaObjsModel;

  //Special object
  ZSwcNetwork *m_swcNetwork;

  //Parent frame
  ZStackFrame *m_parentFrame;

  /* workspaces */
  Trace_Workspace *m_traceWorkspace;
  bool m_isTraceMaskObsolete;
  Connection_Test_Workspace *m_connectionTestWorkspace;

  //Meta information
  ZStackFile m_stackSource;
  std::string m_additionalSource;

  //Thread
  ZStackReadThread m_reader;

  //Actions
  //  Undo/Redo
  QUndoStack *m_undoStack;
  QAction *m_undoAction;
  QAction *m_redoAction;

  //  Action map
  QMap<EActionItem, QAction*> m_actionMap;

  //obsolete fields
  QList<ZLocsegChain*> m_chainList;
  QList<ZLocsegChainConn*> m_connList;
  std::set<ZLocsegChain*> m_selectedChains;
  ZLocsegChain *m_masterChain;
  QString m_badChainScreen;
};

//   template  //
template <class InputIterator>
void ZStackDoc::setPunctumSelected(InputIterator first, InputIterator last, bool select)
{
  QList<ZPunctum*> selected;
  QList<ZPunctum*> deselected;
  for (InputIterator it = first; it != last; ++it) {
    ZPunctum *punctum = *it;
    if (punctum->isSelected() != select) {
      punctum->setSelected(select);
      if (select) {
        m_selectedPuncta.insert(punctum);
        selected.push_back(punctum);
      } else {
        m_selectedPuncta.erase(punctum);
        deselected.push_back(punctum);
      }
    }
  }
  if (!selected.empty() || !deselected.empty()) {
    emit punctaSelectionChanged(selected, deselected);
  }
}

template <class InputIterator>
void ZStackDoc::setSwcSelected(InputIterator first, InputIterator last, bool select)
{
  QList<ZSwcTree*> selected;
  QList<ZSwcTree*> deselected;
  std::vector<Swc_Tree_Node *> tns;
  for (InputIterator it = first; it != last; ++it) {
    ZSwcTree *tree = *it;
    if (tree->isSelected() != select) {
      tree->setSelected(select);
      if (select) {
        m_selectedSwcs.insert(tree);
        selected.push_back(tree);
        // deselect its nodes
        for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin();
             it != m_selectedSwcTreeNodes.end(); ++it) {
          if (tree == nodeToSwcTree(*it))
            tns.push_back(*it);
        }
      } else {
        m_selectedSwcs.erase(tree);
        deselected.push_back(tree);
      }
    }
  }
  setSwcTreeNodeSelected(tns.begin(), tns.end(), false);
  if (!selected.empty() || !deselected.empty()) {
    emit swcSelectionChanged(selected, deselected);
  }
}

template <class InputIterator>
void ZStackDoc::setSwcTreeNodeSelected(InputIterator first, InputIterator last, bool select)
{
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;

  for (InputIterator it = first; it != last; ++it) {
    Swc_Tree_Node *tn = *it;
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
  }
  if (!selected.empty() || !deselected.empty()) {
    emit swcTreeNodeSelectionChanged(selected, deselected);
  }
}

#endif
