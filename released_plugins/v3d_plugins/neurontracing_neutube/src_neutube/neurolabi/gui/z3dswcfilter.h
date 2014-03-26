#ifndef Z3DSWCFILTER_H
#define Z3DSWCFILTER_H

class Z3DConeRenderer;
class Z3DLineRenderer;
class Z3DSphereRenderer;
class Z3DLineWithFixedWidthColorRenderer;

#include <QObject>
#include "z3dgeometryfilter.h"
#include "zoptionparameter.h"
#include <map>
#include <QString>
#include <vector>
#include <utility>

#include "zswctree.h"
#include "zcolormap.h"
#include "z3drendererbase.h"
#include "zwidgetsgroup.h"
#include "zswccolorscheme.h"

class Z3DSwcFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  enum InteractionMode {
    Select, AddSwcNode, ExtendSwcNode, ConnectSwcNode, SmartExtendSwcNode
  };

  explicit Z3DSwcFilter();
  virtual ~Z3DSwcFilter();

  void setData(std::vector<ZSwcTree*> *swcList);
  void setData(QList<ZSwcTree*> *swcList);
  inline void setSelectedSwcs(std::set<ZSwcTree*> *list) {
    m_selectedSwcs = list;
  }
  inline void setSelectedSwcTreeNodes(std::set<Swc_Tree_Node*> *list) {
    m_selectedSwcTreeNodes = list;
  }

  //get bounding box of swc tree in world coordinate :[xmin xmax ymin ymax zmin zmax]
  std::vector<double> getTreeBound(ZSwcTree *tree) const;

  //get bounding box of swc tree node in world coordinate :[xmin xmax ymin ymax zmin zmax]
  std::vector<double> getTreeNodeBound(Swc_Tree_Node *tn) const;

  virtual bool isReady(Z3DEye eye) const;

  // caller should clean up this (by delete parent of this zwidgetgroup)
  ZWidgetsGroup *getWidgetsGroup();

  inline void setRenderingPrimitive(const std::string &mode) {
    m_renderingPrimitive.select(mode.c_str());
  }

  inline int xCutLowerValue() { return m_xCut.lowerValue(); }
  inline int xCutUpperValue() { return m_xCut.upperValue(); }
  inline int yCutLowerValue() { return m_yCut.lowerValue(); }
  inline int yCutUpperValue() { return m_yCut.upperValue(); }
  inline int zCutLowerValue() { return m_zCut.lowerValue(); }
  inline int zCutUpperValue() { return m_zCut.upperValue(); }

  inline int xCutMin() { return m_xCut.minimum(); }
  inline int xCutMax() { return m_xCut.maximum(); }
  inline int yCutMin() { return m_yCut.minimum(); }
  inline int yCutMax() { return m_yCut.maximum(); }
  inline int zCutMin() { return m_zCut.minimum(); }
  inline int zCutMax() { return m_zCut.maximum(); }


  inline void setXCutLower(int v) { m_xCut.setLowerValue(v); }
  inline void setXCutUpper(int v) { m_xCut.setUpperValue(v); }
  inline void setYCutLower(int v) { m_yCut.setLowerValue(v); }
  inline void setYCutUpper(int v) { m_yCut.setUpperValue(v); }
  inline void setZCutLower(int v) { m_zCut.setLowerValue(v); }
  inline void setZCutUpper(int v) { m_zCut.setUpperValue(v); }

  bool isNodeRendering() const { return m_renderingPrimitive.isSelected("Sphere"); }

  void setInteractionMode(InteractionMode mode) { m_interactionMode = mode; }
  inline InteractionMode getInteractionMode() { return m_interactionMode; }


signals:
  void treeSelected(ZSwcTree*, bool append);
  void treeNodeSelected(Swc_Tree_Node*, bool append);
  void connectingSwcTreeNode(Swc_Tree_Node*);
  void treeNodeSelectConnection();
  void addNewSwcTreeNode(double x, double y, double z, double r);
  void extendSwcTreeNode(double x, double y, double z);

public slots:
  void prepareColor();
  void setClipPlanes();
  void adjustWidgets();
  void selectSwc(QMouseEvent *e, int w, int h);

  void updateSwcVisibleState();

  void setColorMode(const std::string &mode);

protected:
  virtual void initialize();
  virtual void deinitialize();
  virtual void process(Z3DEye);

  virtual void registerPickingObjects(Z3DPickingManager *pm);
  virtual void deregisterPickingObjects(Z3DPickingManager *pm);

  void render(Z3DEye eye);
  void renderPicking(Z3DEye eye);
  void renderSelectionBox(Z3DEye eye);
  void prepareData();

private:
  void initTopologyColor();
  void initTypeColor();
  void initSubclassTypeColor();

  void decompseSwcTree();
  glm::vec4 getColorByType(Swc_Tree_Node *n);
  glm::vec4 getColorByDirection(Swc_Tree_Node *tn);

  void updateWidgetsGroup();

  // get visible data from origSwcList put into swcList
  void getVisibleData();

  glm::dvec3 projectPointOnRay(
      glm::dvec3 pt, const glm::dvec3 &v1, const glm::dvec3 &v2);

  void addSelectionBox(const std::pair<Swc_Tree_Node *, Swc_Tree_Node *> &nodePair,
                       std::vector<glm::vec3> &lines);
  void addSelectionBox(const Swc_Tree_Node *tn, std::vector<glm::vec3> &lines);

  void createColorMapperWidget(
      const std::map<ZSwcTree*, ZVec4Parameter*>& mapper,
      std::vector<ZWidgetsGroup*> &widgetGroup);

private:
  Z3DLineRenderer *m_lineRenderer;
  Z3DConeRenderer *m_coneRenderer;
  Z3DSphereRenderer *m_sphereRenderer;
  Z3DSphereRenderer *m_sphereRendererForCone;
  Z3DLineWithFixedWidthColorRenderer *m_boundBoxRenderer;

  ZBoolParameter m_showSwcs;

  ZOptionParameter<QString> m_renderingPrimitive;
  ZOptionParameter<QString> m_colorMode;
  //std::vector<ZVec4Parameter*> m_colorsForDifferentSource;
  std::map<ZSwcTree*, ZVec4Parameter*> m_individualTreeColorMapper;
  std::map<ZSwcTree*, ZVec4Parameter*> m_randomTreeColorMapper;
  std::vector<ZVec4Parameter*> m_colorsForDifferentType;
  std::vector<ZVec4Parameter*> m_colorsForSubclassType;
  std::map<int, size_t> m_subclassTypeColorMapper;
  std::vector<ZVec4Parameter*> m_colorsForDifferentTopology;
  std::map<int, ZVec4Parameter*> m_biocytinColorMapper;
  //std::map<int, ZVec4Parameter*> m_JinTypeColorMapper;

  //std::map<std::string, size_t> m_sourceColorMapper;   // should use unordered_map
  // swc list used for rendering, it is a subset of m_origSwcList. Some swcs are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single swc tree.
  std::vector<ZSwcTree*> m_swcList;
  std::vector<ZSwcTree*> m_registeredSwcList;    // used for picking
  std::vector<Swc_Tree_Node*> m_registeredSwcTreeNodeList;    // used for picking

  ZEventListenerParameter* m_selectSwcEvent;
  glm::ivec2 m_startCoord;
  ZSwcTree *m_pressedSwc;
  std::set<ZSwcTree*> *m_selectedSwcs;   //point to all selected swcs, managed by other class
  Swc_Tree_Node *m_pressedSwcTreeNode;
  std::set<Swc_Tree_Node*> *m_selectedSwcTreeNodes;   //point to all selected swcs, managed by other class

  std::vector<glm::vec4> m_baseAndBaseRadius;
  std::vector<glm::vec4> m_axisAndTopRadius;
  std::vector<glm::vec4> m_swcColors1;
  std::vector<glm::vec4> m_swcColors2;
  std::vector<glm::vec4> m_swcPickingColors;
  std::vector<glm::vec4> m_sphereForConePickingColors;
  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec4> m_linePickingColors;
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;

  std::vector<std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > > m_decompsedNodePairs;
  std::vector<std::vector<Swc_Tree_Node* > > m_decomposedNodes;
  std::set<Swc_Tree_Node*> m_allNodesSet;  // for fast search
  std::set<int> m_allNodeType;   // all node type of current opened swc, used for adjust widget (hide irrelavant stuff)

  ZColorMapParameter m_colorMap;

  ZIntSpanParameter m_xCut;
  ZIntSpanParameter m_yCut;
  ZIntSpanParameter m_zCut;

  ZWidgetsGroup *m_widgetsGroup;
  std::vector<ZWidgetsGroup*> m_randomColorWidgetGroup;
  std::vector<ZWidgetsGroup*> m_individualColorWidgetGroup;
  std::vector<ZWidgetsGroup*> m_colorsForBiocytinTypeWidgetsGroup;
  //std::vector<ZWidgetsGroup*> m_colorsForJinColorWidgetsGroup;
  bool m_dataIsInvalid;

  std::vector<ZSwcTree*> m_origSwcList;

  InteractionMode m_interactionMode;
  ZSwcColorScheme m_colorScheme;
};

#endif // Z3DSWCFILTER_H
