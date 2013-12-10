#include "z3dswcfilter.h"

#include <iostream>

#include "zrandom.h"
#include "tz_3dgeom.h"
#include "z3dlinerenderer.h"
#include "z3dlinewithfixedwidthcolorrenderer.h"
#include "z3dsphererenderer.h"
#include "z3dconerenderer.h"
#include "zeventlistenerparameter.h"
#include "QsLog.h"
#include "swctreenode.h"
#include "tz_geometry.h"
#include "neutubeconfig.h"

Z3DSwcFilter::Z3DSwcFilter()
  : Z3DGeometryFilter()
  , m_lineRenderer(NULL)
  , m_coneRenderer(NULL)
  , m_sphereRenderer(NULL)
  , m_sphereRendererForCone(NULL)
  , m_boundBoxRenderer(NULL)
  , m_showSwcs("Show Swcs", true)
  , m_renderingPrimitive("Rendering Primitive")
  , m_colorMode("Color Mode")
  , m_pressedSwc(NULL)
  , m_selectedSwcs(NULL)
  , m_pressedSwcTreeNode(NULL)
  , m_selectedSwcTreeNodes(NULL)
  , m_colorMap("Color Map")
  , m_xCut("X Cut", glm::ivec2(0,0), 0, 0)
  , m_yCut("Y Cut", glm::ivec2(0,0), 0, 0)
  , m_zCut("Z Cut", glm::ivec2(0,0), 0, 0)
  , m_widgetsGroup(NULL)
  , m_dataIsInvalid(false)
  , m_interactionMode(Select)
{
  initTopologyColor();
  initTypeColor();
  initSubclassTypeColor();


  addParameter(m_showSwcs);


  // rendering primitive
  m_renderingPrimitive.addOptions("Normal", "Line", "Sphere", "Cylinder");
  m_renderingPrimitive.select("Sphere");

  const NeutubeConfig::Z3DWindowConfig::SwcTabConfig &config =
      NeutubeConfig::getInstance().getZ3DWindowConfig().getSwcTabConfig();
  if (!config.getPrimitive().empty()) {
    m_renderingPrimitive.select(config.getPrimitive().c_str());
  }

  // Color Mode
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_colorMode.addOption("Biocytin Branch Type");
  }

  m_colorMode.addOptions("Individual",
                         "Random Tree Color",
                         "Branch Type",
                         "Topology",
                         "Colormap Branch Type",
                         "Subclass",
                         "Direction",
                         "Intrinsic");

  m_colorMode.select("Branch Type");
  if (!config.getColorMode().empty()) {
    m_colorMode.select(config.getColorMode().c_str());
  }

  connect(&m_colorMode, SIGNAL(valueChanged()), this, SLOT(prepareColor()));
  connect(&m_colorMode, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));

  addParameter(m_renderingPrimitive);
  addParameter(m_colorMode);

  for (size_t i=0; i<m_colorsForDifferentType.size(); i++) {
    addParameter(m_colorsForDifferentType[i]);
  }

  for (size_t i=0; i<m_colorsForDifferentTopology.size(); i++) {
    addParameter(m_colorsForDifferentTopology[i]);
  }

  for (size_t i=0; i<m_colorsForSubclassType.size(); i++) {
    addParameter(m_colorsForSubclassType[i]);
  }

  m_selectSwcEvent = new ZEventListenerParameter("Select Puncta", true, false, this);
  m_selectSwcEvent->listenTo("select swc", Qt::LeftButton,
                             Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent->listenTo("select swc", Qt::LeftButton,
                             Qt::NoModifier, QEvent::MouseButtonRelease);
  /*
  m_selectSwcEvent->listenTo("select swc", Qt::RightButton,
                             Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent->listenTo("select swc", Qt::RightButton, Qt::NoModifier,
                             QEvent::MouseButtonRelease);
                             */
  m_selectSwcEvent->listenTo("select swc connection", Qt::LeftButton,
                             Qt::ShiftModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent->listenTo("select swc connection", Qt::LeftButton,
                             Qt::ShiftModifier, QEvent::MouseButtonRelease);

  m_selectSwcEvent->listenTo("append select swc", Qt::LeftButton,
                             Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent->listenTo("append select swc", Qt::LeftButton,
                             Qt::ControlModifier, QEvent::MouseButtonRelease);

  /*
  m_selectSwcEvent->listenTo("append select swc", Qt::RightButton,
                             Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent->listenTo("append select swc", Qt::RightButton,
                             Qt::ControlModifier, QEvent::MouseButtonRelease);
*/
  connect(m_selectSwcEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)),
          this, SLOT(selectSwc(QMouseEvent*,int,int)));
  addEventListener(m_selectSwcEvent);

  addParameter(m_colorMap);
  connect(&m_colorMap, SIGNAL(valueChanged()), this, SLOT(prepareColor()));

  addParameter(m_xCut);
  addParameter(m_yCut);
  addParameter(m_zCut);
  connect(&m_xCut, SIGNAL(valueChanged()), this, SLOT(setClipPlanes()));
  connect(&m_yCut, SIGNAL(valueChanged()), this, SLOT(setClipPlanes()));
  connect(&m_zCut, SIGNAL(valueChanged()), this, SLOT(setClipPlanes()));

  adjustWidgets();
}

Z3DSwcFilter::~Z3DSwcFilter()
{
  //  for(size_t i=0; i<m_colorsForDifferentSource.size(); i++) {
  //    delete m_colorsForDifferentSource[i];
  //  }
  for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator it = m_individualTreeColorMapper.begin();
       it != m_individualTreeColorMapper.end(); ++it) {
    delete it->second;
  }

  for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator it = m_randomTreeColorMapper.begin();
       it != m_randomTreeColorMapper.end(); ++it) {
    delete it->second;
  }

  for (std::map<int, ZVec4Parameter*>::iterator it = m_biocytinColorMapper.begin();
       it != m_biocytinColorMapper.end(); ++it) {
    delete it->second;
  }

  for(size_t i=0; i<m_colorsForDifferentType.size(); i++) {
    delete m_colorsForDifferentType[i];
  }
  for(size_t i=0; i<m_colorsForDifferentTopology.size(); i++) {
    delete m_colorsForDifferentTopology[i];
  }
  for(size_t i=0; i<m_colorsForSubclassType.size(); i++) {
    delete m_colorsForSubclassType[i];
  }

  delete m_selectSwcEvent;
}

void Z3DSwcFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DSwcFilter::initTopologyColor()
{
  // topology colors (root, branch point, leaf, others)
  m_colorsForDifferentTopology.push_back(
        new ZVec4Parameter("Root Color", glm::vec4(0/255.f, 0/255.f, 255/255.f, 1.f)));
  m_colorsForDifferentTopology.push_back(
        new ZVec4Parameter("Branch Point Color", glm::vec4(0/255.f, 255/255.f, 0/255.f, 1.f)));
  m_colorsForDifferentTopology.push_back(
        new ZVec4Parameter("Leaf Color", glm::vec4(255/255.f, 255/255.f, 0/255.f, 1.f)));
  m_colorsForDifferentTopology.push_back(
        new ZVec4Parameter("Other", glm::vec4(255/255.f, 0/255.f, 0/255.f, 1.f)));
  for (size_t i=0; i<m_colorsForDifferentTopology.size(); i++) {
    m_colorsForDifferentTopology[i]->setStyle("COLOR");
    connect(m_colorsForDifferentTopology[i], SIGNAL(valueChanged()), this, SLOT(prepareColor()));
  }
}

void Z3DSwcFilter::initTypeColor()
{
  // type colors
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    ZSwcColorScheme colorScheme;
    colorScheme.setColorScheme(ZSwcColorScheme::JIN_TYPE_COLOR);

    int index = 0;
    QStringList nameList;
    nameList << QString("Type %1 Color").arg(0)
             << QString("Type %1 (Soma) Color").arg(1)
             << QString("Type %1 (Axon) Color").arg(2)
             << QString("Type %1 (Basal Dendrite) Color").arg(3)
             << QString("Type %1 (Apical Dendrite) Color").arg(4)
             << QString("Type %1 (Undefined) Color").arg(5);

    foreach (QString name, nameList) {
      QColor color = colorScheme.getColor(index++);
      m_colorsForDifferentType.push_back(
            new ZVec4Parameter(name, glm::vec4(color.redF(), color.greenF(),
                                               color.blueF(), 1.f)));
    }
  } else {
    int index = 0;
    QString name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(255/255.f, 255/255.f, 255/255.f, 1.f))); //white
    // 1
    name = QString("Type %1 (Soma) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(20/255.f, 20/255.f, 20/255.f, 1.f))); //black
    // 2
    name = QString("Type %1 (Axon) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(200/255.f, 20/255.f, 0/255.f, 1.f))); //red
    // 3
    name = QString("Type %1 (Basal Dendrite) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(0/255.f, 20/255.f, 200/255.f, 1.f))); //blue
    // 4
    name = QString("Type %1 (Apical Dendrite) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(200/255.f, 0/255.f, 200/255.f, 1.f))); //purple
    // 5
    name = QString("Type %1 (Main Trunk) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(0/255.f, 200/255.f, 200/255.f, 1.f))); //cyan
    // 6
    name = QString("Type %1 (Basal Intermediate) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(220/255.f, 200/255.f, 0/255.f, 1.f))); //yellow
    // 7
    name = QString("Type %1 (Basal Terminal) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(0/255.f, 200/255.f, 20/255.f, 1.f))); //green
    // 8
    name = QString("Type %1 (Apical Oblique Intermediate) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(188/255.f, 94/255.f, 37/255.f, 1.f))); //coffee
    // 9
    name = QString("Type %1 (Apical Oblique Terminal) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(180/255.f, 200/255.f, 120/255.f, 1.f))); //asparagus
    // 10
    name = QString("Type %1 (Apical Tuft) Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(250/255.f, 100/255.f, 120/255.f, 1.f))); //salmon
    // 11
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(120/255.f, 200/255.f, 200/255.f, 1.f))); //ice
    // 12
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(100/255.f, 120/255.f, 200/255.f, 1.f))); //orchid
    // 13
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(255/255.f, 128/255.f, 168/255.f, 1.f)));
    // 14
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(128/255.f, 255/255.f, 168/255.f, 1.f)));
    // 15
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(128/255.f, 168/255.f, 255/255.f, 1.f)));
    // 16
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(128/255.f, 255/255.f, 168/255.f, 1.f)));
    // 17
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(255/255.f, 168/255.f, 128/255.f, 1.f)));
    // 18
    name = QString("Type %1 Color").arg(index++);
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(168/255.f, 128/255.f, 255/255.f, 1.f)));
    // 19
    name = QString("Undefined Type Color");
    m_colorsForDifferentType.push_back(new ZVec4Parameter(name, glm::vec4(0xcc/255.f, 0xcc/255.f, 0xcc/255.f, 1.f)));

  }
  for (size_t i=0; i<m_colorsForDifferentType.size(); i++) {
    m_colorsForDifferentType[i]->setStyle("COLOR");
    connect(m_colorsForDifferentType[i], SIGNAL(valueChanged()), this, SLOT(prepareColor()));
  }
}

void Z3DSwcFilter::initSubclassTypeColor()
{
  // subclass type color
  QString name = QString("Root Color");
  m_subclassTypeColorMapper[1] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0/255.f, 0/255.f, 0/255.f, 1.f)));
  name = QString("Main Trunk Color");
  m_subclassTypeColorMapper[5] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0/255.f, 0/255.f, 0/255.f, 1.f)));
  name = QString("Basal Intermediate Color");
  m_subclassTypeColorMapper[6] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0x33/255.f, 0xcc/255.f, 0xff/255.f, 1.f)));
  name = QString("Basal Terminal Color");
  m_subclassTypeColorMapper[7] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0x33/255.f, 0x66/255.f, 0xcc/255.f, 1.f)));
  name = QString("Apical Oblique Intermediate Color");
  m_subclassTypeColorMapper[8] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0xff/255.f, 0xff/255.f, 0/255.f, 1.f)));
  name = QString("Apical Oblique Terminal Color");
  m_subclassTypeColorMapper[9] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0xcc/255.f, 0x33/255.f, 0x66/255.f, 1.f)));
  name = QString("Apical Tuft Color");
  m_subclassTypeColorMapper[10] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0/255.f, 0x99/255.f, 0/255.f, 1.f)));
  name = QString("Other Undefined class Color");
  m_colorsForSubclassType.push_back(new ZVec4Parameter(name, glm::vec4(0xcc/255.f, 0xcc/255.f, 0xcc/255.f, 1.f)));
  for (size_t i=0; i<m_colorsForSubclassType.size(); i++) {
    m_colorsForSubclassType[i]->setStyle("COLOR");
    connect(m_colorsForSubclassType[i], SIGNAL(valueChanged()), this, SLOT(prepareColor()));
  }
}

void Z3DSwcFilter::registerPickingObjects(Z3DPickingManager *pm)
{
  if (pm && !m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_swcList.size(); i++) {
      pm->registerObject(m_swcList[i]);
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        pm->registerObject(m_decomposedNodes[i][j]);
        m_registeredSwcTreeNodeList.push_back(m_decomposedNodes[i][j]);
      }
    }
    m_registeredSwcList = m_swcList;
    m_swcPickingColors.clear();
    m_linePickingColors.clear();
    m_pointPickingColors.clear();
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      glm::col4 pickingColor = pm->getColorFromObject(m_swcList[i]);
      glm::vec4 fPickingColor(pickingColor[0]/255.f, pickingColor[1]/255.f, pickingColor[2]/255.f, pickingColor[3]/255.f);
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        m_swcPickingColors.push_back(fPickingColor);
        m_linePickingColors.push_back(fPickingColor);
        m_linePickingColors.push_back(fPickingColor);
      }
      m_sphereForConePickingColors = m_swcPickingColors;
      m_sphereForConePickingColors.push_back(fPickingColor);
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        pickingColor = pm->getColorFromObject(m_decomposedNodes[i][j]);
        fPickingColor = glm::vec4(pickingColor[0]/255.f, pickingColor[1]/255.f, pickingColor[2]/255.f, pickingColor[3]/255.f);
        m_pointPickingColors.push_back(fPickingColor);
      }
    }

    m_coneRenderer->setDataPickingColors(&m_swcPickingColors);
    m_sphereRendererForCone->setDataPickingColors(&m_sphereForConePickingColors);
    m_lineRenderer->setDataPickingColors(&m_linePickingColors);
    m_sphereRenderer->setDataPickingColors(&m_pointPickingColors);
  }

  m_pickingObjectsRegistered = true;
}

void Z3DSwcFilter::deregisterPickingObjects(Z3DPickingManager *pm)
{
  if (pm && m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_registeredSwcList.size(); i++) {
      pm->deregisterObject(m_registeredSwcList[i]);
    }
    for (size_t i=0; i<m_registeredSwcTreeNodeList.size(); i++) {
      pm->deregisterObject(m_registeredSwcTreeNodeList[i]);
    }
    m_registeredSwcList.clear();
    m_registeredSwcTreeNodeList.clear();
  }

  m_pickingObjectsRegistered = false;
}

void Z3DSwcFilter::setData(std::vector<ZSwcTree *> *swcList)
{
  m_origSwcList.clear();
  if (swcList) {
    m_origSwcList = *swcList;
    LINFO() << getClassName() << "Read" << m_origSwcList.size() << "swcs.";
  }
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DSwcFilter::setData(QList<ZSwcTree *> *swcList)
{
  m_origSwcList.clear();
  if (swcList) {
    for (int i=0; i<swcList->size(); i++)
      m_origSwcList.push_back(swcList->at(i));
    LINFO() << getClassName() << "Read" << m_origSwcList.size() << "swcs.";
  }
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

std::vector<double> Z3DSwcFilter::getTreeBound(ZSwcTree *tree) const
{
  tree->updateIterator(1);   //depth first
  Swc_Tree_Node *tn = tree->begin();
  while (Swc_Tree_Node_Is_Virtual(tn)) {
    tn = tree->next();
  }
  if (tn == NULL)
    return std::vector<double>(6, 0);
  std::vector<double> result = getTreeNodeBound(tn);
  tn = tree->next();
  for (; tn != tree->end(); tn = tree->next()) {
    std::vector<double> nodeBound = getTreeNodeBound(tn);
    result[0] = std::min(result[0], nodeBound[0]);
    result[1] = std::max(result[1], nodeBound[1]);
    result[2] = std::min(result[2], nodeBound[2]);
    result[3] = std::max(result[3], nodeBound[3]);
    result[4] = std::min(result[4], nodeBound[4]);
    result[5] = std::max(result[5], nodeBound[5]);
  }

#ifdef _DEBUG_
  std::cout << getCoordScales().z << std::endl;
  std::cout << "Bound: " << result[4] << " " << result[5] << std::endl;
#endif

  return result;
}

std::vector<double> Z3DSwcFilter::getTreeNodeBound(Swc_Tree_Node *tn) const
{
  std::vector<double> result(6);
  result[0] = tn->node.x * getCoordScales().x - tn->node.d * getSizeScale();
  result[1] = tn->node.x * getCoordScales().x + tn->node.d * getSizeScale();
  result[2] = tn->node.y * getCoordScales().y - tn->node.d * getSizeScale();
  result[3] = tn->node.y * getCoordScales().y + tn->node.d * getSizeScale();
  result[4] = tn->node.z * getCoordScales().z - tn->node.d * getSizeScale();
  result[5] = tn->node.z * getCoordScales().z + tn->node.d * getSizeScale();

  return result;
}

bool Z3DSwcFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && m_showSwcs.get() && !m_origSwcList.empty();
}

namespace {

bool compareParameterName(const ZParameter *p1, const ZParameter *p2)
{
  QString n1 = p1->getName().mid(4); // "Swc "
  QString n2 = p2->getName().mid(4);
  n1.remove(n1.size()-6, 6); //" Color"
  n2.remove(n2.size()-6, 6);
  return n1.toInt() < n2.toInt();
}

}

ZWidgetsGroup *Z3DSwcFilter::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Swcs", NULL, 1);
    new ZWidgetsGroup(&m_showSwcs, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_stayOnTop, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_renderingPrimitive, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_colorMode, m_widgetsGroup, 1);
    //for(size_t i=0; i<m_colorsForDifferentSource.size(); i++) {
      //m_colorsForDifferentSourceWidgetsGroup.push_back(new ZWidgetsGroup(m_colorsForDifferentSource[i], m_widgetsGroup, 1));
    //}

    createColorMapperWidget(m_randomTreeColorMapper, m_randomColorWidgetGroup);
    createColorMapperWidget(m_individualTreeColorMapper,
                            m_individualColorWidgetGroup);
    /*
    std::vector<ZParameter*> allTreeColorParas;
    for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator it = m_randomTreeColorMapper.begin();
         it != m_randomTreeColorMapper.end(); ++it) {
      allTreeColorParas.push_back(it->second);
    }
    std::sort(allTreeColorParas.begin(), allTreeColorParas.end(), compareParameterName);

    allTreeColorParas.clear();
    for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator it = m_individualTreeColorMapper.begin();
         it != m_individualTreeColorMapper.end(); ++it) {
      allTreeColorParas.push_back(it->second);
    }
    std::sort(allTreeColorParas.begin(), allTreeColorParas.end(), compareParameterName);

    for (size_t i=0; i<allTreeColorParas.size(); ++i) {
      m_randomColorWidgetGroup.push_back(
            new ZWidgetsGroup(allTreeColorParas[i], m_widgetsGroup, 1));
    }
    */

    for (std::map<int, ZVec4Parameter*>::iterator it = m_biocytinColorMapper.begin();
         it != m_biocytinColorMapper.end(); ++it) {
      m_colorsForBiocytinTypeWidgetsGroup.push_back(
            new ZWidgetsGroup(it->second, m_widgetsGroup, 1));
    }

    for(size_t i=0; i<m_colorsForDifferentType.size(); i++) {
      new ZWidgetsGroup(m_colorsForDifferentType[i], m_widgetsGroup, 1);
    }
    for(size_t i=0; i<m_colorsForSubclassType.size(); i++) {
      new ZWidgetsGroup(m_colorsForSubclassType[i], m_widgetsGroup, 1);
    }
    for(size_t i=0; i<m_colorsForDifferentTopology.size(); i++) {
      new ZWidgetsGroup(m_colorsForDifferentTopology[i], m_widgetsGroup, 1);
    }
    new ZWidgetsGroup(&m_colorMap, m_widgetsGroup, 1);

    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      if (para->getName() == "Z Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 2);
      else if (para->getName() == "Size Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 3);
      else if (para->getName() == "Rendering Method")
        new ZWidgetsGroup(para, m_widgetsGroup, 4);
      else if (para->getName() == "Opacity")
        new ZWidgetsGroup(para, m_widgetsGroup, 5);
      else
        new ZWidgetsGroup(para, m_widgetsGroup, 7);
    }
    new ZWidgetsGroup(&m_xCut, m_widgetsGroup, 5);
    new ZWidgetsGroup(&m_yCut, m_widgetsGroup, 5);
    new ZWidgetsGroup(&m_zCut, m_widgetsGroup, 5);
    m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DSwcFilter::render(Z3DEye eye)
{
  if (!m_showSwcs.get())
      return;
  if (m_swcList.empty())
    return;

  if (m_renderingPrimitive.isSelected("Normal")) {
    m_rendererBase->activateRenderer(m_sphereRendererForCone, m_coneRenderer);
  } else if (m_renderingPrimitive.isSelected("Cylinder")) {
    m_rendererBase->activateRenderer(m_coneRenderer);
  }
  else if (m_renderingPrimitive.isSelected("Line"))
    m_rendererBase->activateRenderer(m_lineRenderer);
  else /* (m_renderingPrimitive.get() == "Sphere") */{
    m_rendererBase->activateRenderer(m_lineRenderer, m_sphereRenderer);
  }
  m_rendererBase->render(eye);
  renderSelectionBox(eye);
}

void Z3DSwcFilter::renderPicking(Z3DEye eye)
{
  if (!m_showSwcs.get())
      return;
  if (!getPickingManager())
      return;
  if (m_swcList.empty())
    return;

  if (!m_pickingObjectsRegistered)
    registerPickingObjects(getPickingManager());

  if (m_renderingPrimitive.isSelected("Normal")) {
    m_rendererBase->activateRenderer(m_coneRenderer, m_sphereRendererForCone);
  } else if (m_renderingPrimitive.isSelected("Cylinder"))
    m_rendererBase->activateRenderer(m_coneRenderer);
  else if (m_renderingPrimitive.isSelected("Line"))
    m_rendererBase->activateRenderer(m_lineRenderer);
  else /* (m_renderingPrimitive.get() == "Sphere") */{
    m_rendererBase->activateRenderer(m_lineRenderer, m_sphereRenderer);
  }
  m_rendererBase->renderPicking(eye);
}

void Z3DSwcFilter::addSelectionBox(
    const std::pair<Swc_Tree_Node *, Swc_Tree_Node *> &nodePair,
    std::vector<glm::vec3> &lines)
{
  Swc_Tree_Node *n1 = nodePair.first;
  Swc_Tree_Node *n2 = nodePair.second;
  glm::vec3 bPos(n1->node.x * getCoordScales().x,
                 n1->node.y * getCoordScales().y,
                 n1->node.z * getCoordScales().z);
  glm::vec3 tPos(n2->node.x * getCoordScales().x,
                 n2->node.y * getCoordScales().y,
                 n2->node.z * getCoordScales().z);
  float bRadius = n1->node.d * m_rendererBase->getSizeScale();
  float tRadius = n2->node.d * m_rendererBase->getSizeScale();
  glm::vec3 axis = tPos - bPos;
  // vector perpendicular to axis
  glm::vec3 v1, v2;
  glm::getOrthogonalVectors(axis, v1, v2);

  glm::vec3 p1 = bPos - bRadius * v1  - v2 * bRadius;
  glm::vec3 p2 = bPos - v1 * bRadius + v2 * bRadius;
  glm::vec3 p3 = bPos + v1 * bRadius + v2 * bRadius;
  glm::vec3 p4 = bPos + v1 * bRadius - v2 * bRadius;
  glm::vec3 p5 = tPos - v1 * tRadius - v2 * tRadius;
  glm::vec3 p6 = tPos - v1 * tRadius + v2 * tRadius;
  glm::vec3 p7 = tPos + v1 * tRadius + v2 * tRadius;
  glm::vec3 p8 = tPos + v1 * tRadius - v2 * tRadius;

  lines.push_back(p1); lines.push_back(p2);
  lines.push_back(p2); lines.push_back(p3);
  lines.push_back(p3); lines.push_back(p4);
  lines.push_back(p4); lines.push_back(p1);

  lines.push_back(p5); lines.push_back(p6);
  lines.push_back(p6); lines.push_back(p7);
  lines.push_back(p7); lines.push_back(p8);
  lines.push_back(p8); lines.push_back(p5);

  lines.push_back(p1); lines.push_back(p5);
  lines.push_back(p2); lines.push_back(p6);
  lines.push_back(p3); lines.push_back(p7);
  lines.push_back(p4); lines.push_back(p8);
}

void Z3DSwcFilter::addSelectionBox(
    const Swc_Tree_Node *tn, std::vector<glm::vec3> &lines)
{
  float radius = tn->node.d * m_rendererBase->getSizeScale();
  float xmin = tn->node.x * getCoordScales().x - radius;
  float xmax = tn->node.x * getCoordScales().x + radius;
  float ymin = tn->node.y * getCoordScales().y - radius;
  float ymax = tn->node.y * getCoordScales().y + radius;
  float zmin = tn->node.z * getCoordScales().z - radius;
  float zmax = tn->node.z * getCoordScales().z + radius;
  lines.push_back(glm::vec3(xmin, ymin, zmin));
  lines.push_back(glm::vec3(xmin, ymin, zmax));
  lines.push_back(glm::vec3(xmin, ymax, zmin));
  lines.push_back(glm::vec3(xmin, ymax, zmax));

  lines.push_back(glm::vec3(xmax, ymin, zmin));
  lines.push_back(glm::vec3(xmax, ymin, zmax));
  lines.push_back(glm::vec3(xmax, ymax, zmin));
  lines.push_back(glm::vec3(xmax, ymax, zmax));

  lines.push_back(glm::vec3(xmin, ymin, zmin));
  lines.push_back(glm::vec3(xmax, ymin, zmin));
  lines.push_back(glm::vec3(xmin, ymax, zmin));
  lines.push_back(glm::vec3(xmax, ymax, zmin));

  lines.push_back(glm::vec3(xmin, ymin, zmax));
  lines.push_back(glm::vec3(xmax, ymin, zmax));
  lines.push_back(glm::vec3(xmin, ymax, zmax));
  lines.push_back(glm::vec3(xmax, ymax, zmax));

  lines.push_back(glm::vec3(xmin, ymin, zmin));
  lines.push_back(glm::vec3(xmin, ymax, zmin));
  lines.push_back(glm::vec3(xmax, ymin, zmin));
  lines.push_back(glm::vec3(xmax, ymax, zmin));

  lines.push_back(glm::vec3(xmin, ymin, zmax));
  lines.push_back(glm::vec3(xmin, ymax, zmax));
  lines.push_back(glm::vec3(xmax, ymin, zmax));
  lines.push_back(glm::vec3(xmax, ymax, zmax));
}

void Z3DSwcFilter::renderSelectionBox(Z3DEye eye)
{
  if (m_selectedSwcs && m_selectedSwcs->size() > 0) {
    std::vector<glm::vec3> lines;
    for (std::set<ZSwcTree*>::iterator it=m_selectedSwcs->begin();
         it != m_selectedSwcs->end(); it++) {
      ZSwcTree *selectedSwc = *it;
      int index = -1;
      for (size_t i = 0; i<m_swcList.size(); i++) {
        if (m_swcList.at(i) == selectedSwc) {
          index = i;
          break;
        }
      }
      if (index == -1) {
        if (selectedSwc->isVisible()) {
          LERROR() << "selected swc not found.. Need Check..";
        }
        continue;
      }

      for (size_t j=0; j<m_decompsedNodePairs[index].size(); j++) {
        addSelectionBox(m_decompsedNodePairs[index][j], lines);
#if 0
        Swc_Tree_Node *n1 = m_decompsedNodePairs[index][j].first;
        Swc_Tree_Node *n2 = m_decompsedNodePairs[index][j].second;
        glm::vec3 bPos(n1->node.x * getCoordScales().x,
                       n1->node.y * getCoordScales().y,
                       n1->node.z * getCoordScales().z);
        glm::vec3 tPos(n2->node.x * getCoordScales().x,
                       n2->node.y * getCoordScales().y,
                       n2->node.z * getCoordScales().z);
        float bRadius = n1->node.d * m_rendererBase->getSizeScale();
        float tRadius = n2->node.d * m_rendererBase->getSizeScale();
        glm::vec3 axis = tPos - bPos;
        // vector perpendicular to axis
        glm::vec3 v1, v2;
        glm::getOrthogonalVectors(axis, v1, v2);

        glm::vec3 p1 = bPos - bRadius * v1  - v2 * bRadius;
        glm::vec3 p2 = bPos - v1 * bRadius + v2 * bRadius;
        glm::vec3 p3 = bPos + v1 * bRadius + v2 * bRadius;
        glm::vec3 p4 = bPos + v1 * bRadius - v2 * bRadius;
        glm::vec3 p5 = tPos - v1 * tRadius - v2 * tRadius;
        glm::vec3 p6 = tPos - v1 * tRadius + v2 * tRadius;
        glm::vec3 p7 = tPos + v1 * tRadius + v2 * tRadius;
        glm::vec3 p8 = tPos + v1 * tRadius - v2 * tRadius;

        lines.push_back(p1); lines.push_back(p2);
        lines.push_back(p2); lines.push_back(p3);
        lines.push_back(p3); lines.push_back(p4);
        lines.push_back(p4); lines.push_back(p1);

        lines.push_back(p5); lines.push_back(p6);
        lines.push_back(p6); lines.push_back(p7);
        lines.push_back(p7); lines.push_back(p8);
        lines.push_back(p8); lines.push_back(p5);

        lines.push_back(p1); lines.push_back(p5);
        lines.push_back(p2); lines.push_back(p6);
        lines.push_back(p3); lines.push_back(p7);
        lines.push_back(p4); lines.push_back(p8);
#endif
      }

      for (size_t j=0; j<m_decomposedNodes[index].size(); j++) {
        Swc_Tree_Node *tn = m_decomposedNodes[index][j];
        if (SwcTreeNode::isRoot(tn) && !SwcTreeNode::hasChild(tn)) {
          addSelectionBox(m_decomposedNodes[index][j], lines);
        }
      }
    }

    m_rendererBase->activateRenderer(m_boundBoxRenderer);
    m_boundBoxRenderer->setData(&lines);
    m_rendererBase->render(eye);
    m_boundBoxRenderer->setData(NULL); // lines will go out of scope
  }

  if (m_selectedSwcTreeNodes && m_selectedSwcTreeNodes->size() > 0) {
    std::vector<glm::vec3> lines;
    for (std::set<Swc_Tree_Node*>::iterator it=m_selectedSwcTreeNodes->begin();
         it != m_selectedSwcTreeNodes->end(); it++) {
      addSelectionBox(*it, lines);
#if 0
      Swc_Tree_Node *selectedSwcTreeNode = *it;

      float radius = selectedSwcTreeNode->node.d * m_rendererBase->getSizeScale();
      float xmin = selectedSwcTreeNode->node.x * getCoordScales().x - radius;
      float xmax = selectedSwcTreeNode->node.x * getCoordScales().x + radius;
      float ymin = selectedSwcTreeNode->node.y * getCoordScales().y - radius;
      float ymax = selectedSwcTreeNode->node.y * getCoordScales().y + radius;
      float zmin = selectedSwcTreeNode->node.z * getCoordScales().z - radius;
      float zmax = selectedSwcTreeNode->node.z * getCoordScales().z + radius;
      lines.push_back(glm::vec3(xmin, ymin, zmin));
      lines.push_back(glm::vec3(xmin, ymin, zmax));
      lines.push_back(glm::vec3(xmin, ymax, zmin));
      lines.push_back(glm::vec3(xmin, ymax, zmax));

      lines.push_back(glm::vec3(xmax, ymin, zmin));
      lines.push_back(glm::vec3(xmax, ymin, zmax));
      lines.push_back(glm::vec3(xmax, ymax, zmin));
      lines.push_back(glm::vec3(xmax, ymax, zmax));

      lines.push_back(glm::vec3(xmin, ymin, zmin));
      lines.push_back(glm::vec3(xmax, ymin, zmin));
      lines.push_back(glm::vec3(xmin, ymax, zmin));
      lines.push_back(glm::vec3(xmax, ymax, zmin));

      lines.push_back(glm::vec3(xmin, ymin, zmax));
      lines.push_back(glm::vec3(xmax, ymin, zmax));
      lines.push_back(glm::vec3(xmin, ymax, zmax));
      lines.push_back(glm::vec3(xmax, ymax, zmax));

      lines.push_back(glm::vec3(xmin, ymin, zmin));
      lines.push_back(glm::vec3(xmin, ymax, zmin));
      lines.push_back(glm::vec3(xmax, ymin, zmin));
      lines.push_back(glm::vec3(xmax, ymax, zmin));

      lines.push_back(glm::vec3(xmin, ymin, zmax));
      lines.push_back(glm::vec3(xmin, ymax, zmax));
      lines.push_back(glm::vec3(xmax, ymin, zmax));
      lines.push_back(glm::vec3(xmax, ymax, zmax));
#endif
    }

    m_rendererBase->activateRenderer(m_boundBoxRenderer);
    m_boundBoxRenderer->setData(&lines);
    m_rendererBase->render(eye);
    m_boundBoxRenderer->setData(NULL); // lines will go out of scope
  }
}

void Z3DSwcFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  decompseSwcTree();

  // get min max of type for colormap
  m_colorMap.blockSignals(true);
  if (m_allNodeType.empty())
    m_colorMap.get().reset();
  else
    m_colorMap.get().reset(m_allNodeType.begin(), m_allNodeType.end(),
                           glm::col4(0,0,255,255), glm::col4(255,0,0,255));
  m_colorMap.blockSignals(false);

  deregisterPickingObjects(getPickingManager());

  //convert swc to format that glsl can use
  m_axisAndTopRadius.clear();
  m_baseAndBaseRadius.clear();
  m_pointAndRadius.clear();
  //m_sourceColorMapper.clear();
  m_lines.clear();

  bool needUpdateWidget = false;

  int xMin = std::numeric_limits<int>::max();
  int xMax = std::numeric_limits<int>::min();
  int yMin = std::numeric_limits<int>::max();
  int yMax = std::numeric_limits<int>::min();
  int zMin = std::numeric_limits<int>::max();
  int zMax = std::numeric_limits<int>::min();
  for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
    for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
      Swc_Tree_Node *n1 = m_decompsedNodePairs[i][j].first;
      Swc_Tree_Node *n2 = m_decompsedNodePairs[i][j].second;
      glm::vec4 baseAndbRadius, axisAndtRadius;
      // make sure base has smaller radius.
      if (Swc_Tree_Node_Const_Data(n1)->d <= Swc_Tree_Node_Const_Data(n2)->d) {
        baseAndbRadius = glm::vec4(n1->node.x, n1->node.y, n1->node.z, n1->node.d);
        axisAndtRadius = glm::vec4(n2->node.x - n1->node.x,
                                   n2->node.y - n1->node.y,
                                   n2->node.z - n1->node.z, n2->node.d);
      } else {
        baseAndbRadius = glm::vec4(n2->node.x, n2->node.y, n2->node.z, n2->node.d);
        axisAndtRadius = glm::vec4(n1->node.x - n2->node.x,
                                   n1->node.y - n2->node.y,
                                   n1->node.z - n2->node.z, n1->node.d);
      }
      m_baseAndBaseRadius.push_back(baseAndbRadius);
      m_axisAndTopRadius.push_back(axisAndtRadius);
      m_lines.push_back(baseAndbRadius.xyz());
      m_lines.push_back(glm::vec3(baseAndbRadius.xyz()) + glm::vec3(axisAndtRadius.xyz()));
    }
    for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
      Swc_Tree_Node *tn = m_decomposedNodes[i][j];
      m_pointAndRadius.push_back(glm::vec4(tn->node.x, tn->node.y, tn->node.z, tn->node.d));
      if (tn->node.x > xMax)
        xMax = static_cast<int>(std::ceil(tn->node.x));
      if (tn->node.x < xMin)
        xMin = static_cast<int>(std::floor(tn->node.x));
      if (tn->node.y > yMax)
        yMax = static_cast<int>(std::ceil(tn->node.y));
      if (tn->node.y < yMin)
        yMin = static_cast<int>(std::floor(tn->node.y));
      if (tn->node.z > zMax)
        zMax = static_cast<int>(std::ceil(tn->node.z));
      if (tn->node.z < zMin)
        zMin = static_cast<int>(std::floor(tn->node.z));

      int type = SwcTreeNode::type(tn);
      m_colorScheme.setColorScheme(ZSwcColorScheme::BIOCYTIN_TYPE_COLOR);
      QString guiname = QString("Type %1 Color").arg(type);
      if (m_biocytinColorMapper.find(type) == m_biocytinColorMapper.end()) {
        QColor color = m_colorScheme.getColor(type);
        m_biocytinColorMapper[type] =
            new ZVec4Parameter(guiname, glm::vec4(color.redF(), color.greenF(),
                                                  color.blueF(), 1.f));
        m_biocytinColorMapper[type]->setStyle("COLOR");
        connect(m_biocytinColorMapper[type], SIGNAL(valueChanged()),
            this, SLOT(prepareColor()));
        addParameter(m_biocytinColorMapper[type]);
        needUpdateWidget = true;
      } else {
        m_biocytinColorMapper[type]->setName(guiname);
      }
    }
  }
  /*
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    m_sourceColorMapper.insert(std::pair<std::string, size_t>(m_origSwcList[i]->source(), 0));
    */

  m_xCut.setRange(xMin, xMax);
  m_xCut.set(glm::ivec2(xMin, xMax));
  m_yCut.setRange(yMin, yMax);
  m_yCut.set(glm::ivec2(yMin, yMax));
  m_zCut.setRange(zMin, zMax);
  m_zCut.set(glm::ivec2(zMin, zMax));

  //std::map<std::string,size_t>::iterator it;

  std::set<ZSwcTree*> allSources;
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    allSources.insert(m_origSwcList[i]);
    QString guiname = QString("Swc %1 Color").arg(i + 1);
    if (m_randomTreeColorMapper.find(m_origSwcList[i]) == m_randomTreeColorMapper.end()) {
      m_randomTreeColorMapper[m_origSwcList[i]] =
          new ZVec4Parameter(guiname, glm::vec4(ZRandomInstance.randReal<float>(),
                                                ZRandomInstance.randReal<float>(),
                                                ZRandomInstance.randReal<float>(),
                                                1.f));
      m_randomTreeColorMapper[m_origSwcList[i]]->setStyle("COLOR");
      connect(m_randomTreeColorMapper[m_origSwcList[i]], SIGNAL(valueChanged()),
          this, SLOT(prepareColor()));
      addParameter(m_randomTreeColorMapper[m_origSwcList[i]]);
      needUpdateWidget = true;
    } else {
      m_randomTreeColorMapper[m_origSwcList[i]]->setName(guiname);
    }
  }

  ZSwcColorScheme colorScheme;
  colorScheme.setColorScheme(ZSwcColorScheme::UNIQUE_COLOR);

  allSources.clear();
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    allSources.insert(m_origSwcList[i]);
    QString guiname = QString("Swc %1 Color").arg(i + 1);
    if (m_individualTreeColorMapper.find(m_origSwcList[i]) == m_individualTreeColorMapper.end()) {
      QColor color = colorScheme.getColor(i);
      m_individualTreeColorMapper[m_origSwcList[i]] = new ZVec4Parameter(
            guiname,
            glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f));

      m_individualTreeColorMapper[m_origSwcList[i]]->setStyle("COLOR");
      connect(m_individualTreeColorMapper[m_origSwcList[i]], SIGNAL(valueChanged()),
          this, SLOT(prepareColor()));
      addParameter(m_individualTreeColorMapper[m_origSwcList[i]]);
      needUpdateWidget = true;
    } else {
      m_individualTreeColorMapper[m_origSwcList[i]]->setName(guiname);
    }
  }


  // remove colors for not exist swc
  std::map<ZSwcTree*, ZVec4Parameter*>::iterator it = m_randomTreeColorMapper.begin();
  while (it != m_randomTreeColorMapper.end()) {
    if (allSources.find(it->first) == allSources.end()) {
      std::map<ZSwcTree*, ZVec4Parameter*>::iterator itCopy = it;
      ++it;
      removeParameter(itCopy->second);
      delete itCopy->second;
      m_randomTreeColorMapper.erase(itCopy);
      needUpdateWidget = true;
    } else
      ++it;
  }

  it = m_individualTreeColorMapper.begin();
  while (it != m_individualTreeColorMapper.end()) {
    if (allSources.find(it->first) == allSources.end()) {
      std::map<ZSwcTree*, ZVec4Parameter*>::iterator itCopy = it;
      ++it;
      removeParameter(itCopy->second);
      delete itCopy->second;
      m_individualTreeColorMapper.erase(itCopy);
      needUpdateWidget = true;
    } else
      ++it;
  }

  if (needUpdateWidget)
    updateWidgetsGroup();

  m_coneRenderer->setData(&m_baseAndBaseRadius, &m_axisAndTopRadius);
  m_lineRenderer->setData(&m_lines);
  m_sphereRenderer->setData(&m_pointAndRadius);
  m_sphereRendererForCone->setData(&m_pointAndRadius);
  prepareColor();
  adjustWidgets();
  //if (numOfPrevColor != index)   // number of swc changed
  //  updateWidgetsGroup();
  m_dataIsInvalid = false;
}

glm::vec4 Z3DSwcFilter::getColorByDirection(Swc_Tree_Node *tn)
{
  if (SwcTreeNode::type(tn) == 1) {
     return glm::vec4(0.8, 0.8, 0.8, 1.f);
  }

  ZPoint vec = SwcTreeNode::localDirection(tn, 15);

  vec.normalize();
  ZPoint colorVec;
/*
  ZPoint colorCode[4] = {
    ZPoint(0, 1, 0), ZPoint(1, 0, 0), ZPoint(0, 0, 1), ZPoint(1, 0, 1)
  };
*/

  ZPoint colorCode[4] = {
    ZPoint(1, 0, 0), ZPoint(0, 0, 1), ZPoint(1, 0, 1), ZPoint(0, 1, 0)
  };

  ZPoint axis[4] = {
    ZPoint(1, 0, 0), ZPoint(0, 1, 0), ZPoint(-1, 0, 0), ZPoint(0, -1, 0)
  };

  /*
  ZPoint axis[4] = {
    ZPoint(0.8801, -0.1720, 0), ZPoint(-0.2595, 0.8541, 0),
    ZPoint(-0.4797, 0.5570, 0), ZPoint(-0.0106, -0.2400, 0)
  };
  */

#if 0
  //from gmm
  ZPoint axis[4] = {
    ZPoint(-0.4586,    0.8887, 0), ZPoint(-0.7185,    0.6955, 0),
    ZPoint(0.4059,   -0.9139, 0), ZPoint(0.7220,   -0.6919, 0)
  };
#endif

#if 0
  //Direction for the TEM paper
  ZPoint axis[4] = {
    ZPoint(-0.0872,    0.9962, 0), ZPoint(-0.7185,    0.6955, 0),
    ZPoint(0.0872 ,  -0.9962, 0), ZPoint(0.9659,   -0.2588, 0)
  };
#endif

#if 0
  //from fft
  std::cout << "fft direction" << std::endl;
  ZPoint axis[4] = {
    ZPoint(-0.4276, 0.8965, 0), ZPoint(-0.8400,    0.3506, 0),
    ZPoint(0.1515,   -0.9806, 0), ZPoint(0.9368,   -0.2822, 0)
  };
#endif


  double angle[4];
  int axisIndex[2] = { 0, 0 };
  for (int k = 0; k < 4; ++k) {
    angle[k] = Vector_Angle2(
          vec.x(), vec.y(), axis[k].x(), axis[k].y(), TRUE);
  }

  double minAngle = angle[0];
  for (int k = 1; k < 4; ++k) {
    if (minAngle > angle[k]) {
      minAngle = angle[k];
      axisIndex[0] = k;
    }
  }

  axisIndex[1] = axisIndex[0] - 1;
  if (axisIndex[1] < 0) {
    axisIndex[1] = 3;
  }

#ifdef _DEBUG_2
  std::cout << axisIndex[0] << " " << axisIndex[1] << std::endl;
  std::cout << angle[axisIndex[0]] << " " << angle[axisIndex[1]] << std::endl;
#endif

  //Intepolate color
  if ((angle[axisIndex[0]] == 0) && (angle[axisIndex[1]] == 0)) {
    colorVec.set(0, 0, 0);
  } else {
    angle[axisIndex[1]] = TZ_2PI - angle[axisIndex[1]];
    double alpha = angle[axisIndex[0]] / (angle[axisIndex[1]] + angle[axisIndex[0]]);
    colorVec = colorCode[axisIndex[0]] * (1.0 - alpha) +
        colorCode[axisIndex[1]] * alpha;
  }

  double z = fabs(vec.z());
  QColor color;
  color.setRgbF(colorVec.x(), colorVec.y(), colorVec.z());
  color.setHsvF(color.hueF(), color.valueF(),
                std::min(1.0, color.saturationF() + z));

  //return glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f);

  return glm::vec4(fabs(vec.x()), fabs(vec.y()), fabs(vec.z()), 1.f);
}

void Z3DSwcFilter::prepareColor()
{
  m_swcColors1.clear();
  m_swcColors2.clear();
  m_lineColors.clear();
  m_pointColors.clear();

  if (m_colorMode.isSelected("Branch Type") ||
      m_colorMode.isSelected("Colormap Branch Type") ||
      m_colorMode.isSelected("Subclass") ||
      m_colorMode.isSelected("Biocytin Branch Type")) {
    if (m_colorMode.isSelected("Biocytin Branch Type")) {
      m_colorScheme.setColorScheme(ZSwcColorScheme::BIOCYTIN_TYPE_COLOR);
    }

    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        glm::vec4 color1 = getColorByType(m_decompsedNodePairs[i][j].first);
        glm::vec4 color2 = getColorByType(m_decompsedNodePairs[i][j].second);
        if (m_decompsedNodePairs[i][j].first->node.d > m_decompsedNodePairs[i][j].second->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(getColorByType(m_decomposedNodes[i][j]));
      }
    }
  } else if (m_colorMode.isSelected("Random Tree Color")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      /*glm::vec4 color = m_colorsForDifferentSource[
          m_sourceColorMapper[m_swcList[i]->source()]]->get();*/
      //glm::vec4 color = m_colorsForDifferentSource[i]->get();
      glm::vec4 color = m_randomTreeColorMapper[m_swcList[i]]->get();
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {

        m_swcColors1.push_back(color);
        m_swcColors2.push_back(color);
        m_lineColors.push_back(color);
        m_lineColors.push_back(color);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(color);
      }
    }
  } else if (m_colorMode.isSelected("Individual")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      glm::vec4 color = m_individualTreeColorMapper[m_swcList[i]]->get();
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        m_swcColors1.push_back(color);
        m_swcColors2.push_back(color);
        m_lineColors.push_back(color);
        m_lineColors.push_back(color);
      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(color);
      }
    }
  } else if (m_colorMode.isSelected("Topology")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        Swc_Tree_Node *n1 = m_decompsedNodePairs[i][j].first;
        Swc_Tree_Node *n2 = m_decompsedNodePairs[i][j].second;
        glm::vec4 color1, color2;
        if (Swc_Tree_Node_Is_Regular_Root(n1))
          color1 = m_colorsForDifferentTopology[0]->get();
        else if (Swc_Tree_Node_Is_Branch_Point(n1))
          color1 = m_colorsForDifferentTopology[1]->get();
        else if (Swc_Tree_Node_Is_Leaf(n1))
          color1 = m_colorsForDifferentTopology[2]->get();
        else
          color1 = m_colorsForDifferentTopology[3]->get();
        if (Swc_Tree_Node_Is_Regular_Root(n2))
          color2 = m_colorsForDifferentTopology[0]->get();
        else if (Swc_Tree_Node_Is_Branch_Point(n2))
          color2 = m_colorsForDifferentTopology[1]->get();
        else if (Swc_Tree_Node_Is_Leaf(n2))
          color2 = m_colorsForDifferentTopology[2]->get();
        else
          color2 = m_colorsForDifferentTopology[3]->get();
        if (n1->node.d > n2->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);
      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        Swc_Tree_Node *n1 = m_decomposedNodes[i][j];
        glm::vec4 color1;
        if (Swc_Tree_Node_Is_Regular_Root(n1))
          color1 = m_colorsForDifferentTopology[0]->get();
        else if (Swc_Tree_Node_Is_Branch_Point(n1))
          color1 = m_colorsForDifferentTopology[1]->get();
        else if (Swc_Tree_Node_Is_Leaf(n1))
          color1 = m_colorsForDifferentTopology[2]->get();
        else
          color1 = m_colorsForDifferentTopology[3]->get();
        m_pointColors.push_back(color1);
      }
    }
  } else if (m_colorMode.isSelected("Direction")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        glm::vec4 color1 = getColorByDirection(m_decompsedNodePairs[i][j].first);
        glm::vec4 color2 = getColorByDirection(m_decompsedNodePairs[i][j].second);
        if (m_decompsedNodePairs[i][j].first->node.d > m_decompsedNodePairs[i][j].second->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(getColorByDirection(m_decomposedNodes[i][j]));
      }
    }
  } else if (m_colorMode.isSelected("Intrinsic")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      QColor swcColor = m_swcList[i]->getColor();
      glm::vec4 color(swcColor.redF(), swcColor.greenF(), swcColor.blueF(),
                      swcColor.alphaF());
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {

        m_swcColors1.push_back(color);
        m_swcColors2.push_back(color);
        m_lineColors.push_back(color);
        m_lineColors.push_back(color);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(color);
      }
    }
  }

  m_coneRenderer->setDataColors(&m_swcColors1, &m_swcColors2);
  m_lineRenderer->setDataColors(&m_lineColors);
  m_sphereRenderer->setDataColors(&m_pointColors);
  m_sphereRendererForCone->setDataColors(&m_pointColors);
}

void Z3DSwcFilter::setClipPlanes()
{
  std::vector<glm::dvec4> clipPlanes;
  if (m_xCut.lowerValue() != m_xCut.minimum())
    clipPlanes.push_back(glm::dvec4(1., 0., 0., -m_xCut.lowerValue()));
  if (m_xCut.upperValue() != m_xCut.maximum())
    clipPlanes.push_back(glm::dvec4(-1., 0., 0., m_xCut.upperValue()));
  if (m_yCut.lowerValue() != m_yCut.minimum())
    clipPlanes.push_back(glm::dvec4(0., 1., 0., -m_yCut.lowerValue()));
  if (m_yCut.upperValue() != m_yCut.maximum())
    clipPlanes.push_back(glm::dvec4(0., -1., 0., m_yCut.upperValue()));
  if (m_zCut.lowerValue() != m_zCut.minimum())
    clipPlanes.push_back(glm::dvec4(0., 0., 1., -m_zCut.lowerValue()));
  if (m_zCut.upperValue() != m_zCut.maximum())
    clipPlanes.push_back(glm::dvec4(0., 0., -1., m_zCut.upperValue()));
  m_rendererBase->setClipPlanes(&clipPlanes);
}

void Z3DSwcFilter::adjustWidgets()
{
  for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator
       it = m_randomTreeColorMapper.begin();
       it != m_randomTreeColorMapper.end(); ++it) {
    if (m_colorMode.isSelected("Random Tree Color"))
      it->second->setVisible(true);
    else
      it->second->setVisible(false);
  }

  for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator
       it = m_individualTreeColorMapper.begin();
       it != m_individualTreeColorMapper.end(); ++it) {
    if (m_colorMode.isSelected("Individual"))
      it->second->setVisible(true);
    else
      it->second->setVisible(false);
  }

  for (std::map<int, ZVec4Parameter*>::iterator it = m_biocytinColorMapper.begin();
       it != m_biocytinColorMapper.end(); ++it) {
    if (m_colorMode.isSelected("Biocytin Branch Type"))
      it->second->setVisible(true);
    else
      it->second->setVisible(false);
  }

  for (size_t i=0; i<m_colorsForDifferentType.size(); i++) {
    if (m_allNodeType.find(i) != m_allNodeType.end() && m_colorMode.get() == "Branch Type") {
      m_colorsForDifferentType[i]->setVisible(true);
    } else {
      m_colorsForDifferentType[i]->setVisible(false);
    }
  }
  for (size_t i=0; i<m_colorsForSubclassType.size(); i++) {
    if (m_colorMode.isSelected("Subclass")) {
      m_colorsForSubclassType[i]->setVisible(true);
    } else {
      m_colorsForSubclassType[i]->setVisible(false);
    }
  }
  for (size_t i=0; i<m_colorsForDifferentTopology.size(); i++) {
    if (m_colorMode.isSelected("Topology")) {
      m_colorsForDifferentTopology[i]->setVisible(true);
    } else {
      m_colorsForDifferentTopology[i]->setVisible(false);
    }
  }
  if (m_colorMode.isSelected("Colormap Branch Type")) {
    m_colorMap.setVisible(true);
  } else {
    m_colorMap.setVisible(false);
  }
}

void Z3DSwcFilter::selectSwc(QMouseEvent *e, int w, int h)
{
  if (m_swcList.empty())
    return;
  if (!getPickingManager())
    return;

  e->ignore();
  // Mouse button presend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();
    //const void* obj = getPickingManager()->getObjectAtPos(glm::ivec2(e->x(), h - e->y()));
    const void* obj = getPickingManager()->getObjectAtWidgetPos(
          glm::ivec2(e->x(), e->y()));

    if (obj == NULL) {
      return;
    }

    // Check if any swc or node was selected...
    for (std::vector<ZSwcTree*>::iterator it=m_swcList.begin(); it!=m_swcList.end(); ++it)
      if (*it == obj) {
        m_pressedSwc = *it;
        return;
      }
    //    for (size_t i=0; i<m_swcList.size(); i++) {
    //      for (size_t j=0; j<m_decompsedNodes[i].size(); j++) {
    //        if (m_decompsedNodes[i][j] == obj) {
    //          m_pressedSwcTreeNode = m_decompsedNodes[i][j];
    //          return;
    //        }
    //      }
    //    }
    if (isNodeRendering()) {
      std::set<Swc_Tree_Node*>::iterator it = m_allNodesSet.find((Swc_Tree_Node*)obj);
      if (it != m_allNodesSet.end())
        m_pressedSwcTreeNode = *it;
    }
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) { 
    if (std::abs(e->x() - m_startCoord.x) < 2 && std::abs(m_startCoord.y - e->y()) < 2) {
      //bool showingContextMenu = (e->button() == Qt::RightButton);
      bool appending = (e->modifiers() == Qt::ControlModifier) ||
          (e->modifiers() == Qt::ShiftModifier);

      if (m_pressedSwc || m_pressedSwcTreeNode) {  // hit something
        // do not select tree when it is node rendering, but allow deselecting swc tree in node rendering mode
        if (!(isNodeRendering() && m_pressedSwc))
          emit treeSelected(m_pressedSwc, appending);

        if (m_interactionMode == ConnectSwcNode && isNodeRendering()) {
          emit(connectingSwcTreeNode(m_pressedSwcTreeNode));
        } else {
          emit treeNodeSelected(m_pressedSwcTreeNode, appending);
          if (e->modifiers() == Qt::ShiftModifier) {
            emit treeNodeSelectConnection();
          }
        }
        e->accept();
      } else if (m_interactionMode == Select) {  // hit nothing in Select mode, if not appending, will deselect all nodes and swcs
        emit treeSelected(m_pressedSwc, appending);
        emit treeNodeSelected(m_pressedSwcTreeNode, appending);
      } else if (m_interactionMode == AddSwcNode && isNodeRendering()) { // hit nothing, add node
        Swc_Tree_Node *tn = NULL;
        // search within a radius first to speed up
        const std::vector<const void*> &objs =
            getPickingManager()->sortObjectsByDistanceToPos(glm::ivec2(e->x(), h-e->y()), 100);
        for (size_t i=0; i<objs.size(); ++i) {
          std::set<Swc_Tree_Node*>::iterator it = m_allNodesSet.find((Swc_Tree_Node*)objs[i]);
          if (it != m_allNodesSet.end()) {
            tn = *it;
            break;
          }
        }
        // not found, search the whole image
        if (!tn) {
          const std::vector<const void*> &objs1 =
              getPickingManager()->sortObjectsByDistanceToPos(glm::ivec2(e->x(), h-e->y()), -1);
          for (size_t i=0; i<objs1.size(); ++i) {
            std::set<Swc_Tree_Node*>::iterator it = m_allNodesSet.find((Swc_Tree_Node*)objs1[i]);
            if (it != m_allNodesSet.end()) {
              tn = *it;
              break;
            }
          }
        }

        if (tn) {
          glm::dvec3 v1,v2;
          get3DRayUnderScreenPoint(v1, v2, e->x(), e->y(), w, h);
          glm::dvec3 nodePos(tn->node.x * getCoordScales().x,
                             tn->node.y * getCoordScales().y,
                             tn->node.z * getCoordScales().z);
          glm::dvec3 pos = projectPointOnRay(nodePos, v1, v2) / glm::dvec3(getCoordScales());
          /*
          Swc_Tree_Node* node = Make_Swc_Tree_Node(&(tn->node));
          node->node.x = pos.x;
          node->node.y = pos.y;
          node->node.z = pos.z;
          */
          emit addNewSwcTreeNode(pos.x, pos.y, pos.z, SwcTreeNode::radius(tn));
        }
      } else if (m_interactionMode == ExtendSwcNode && m_selectedSwcTreeNodes->size() == 1) {  // hit nothing, extend node
        Swc_Tree_Node *tn = *(m_selectedSwcTreeNodes->begin());
        glm::dvec3 v1,v2;
        get3DRayUnderScreenPoint(v1, v2, e->x(), e->y(), w, h);
        glm::dvec3 nodePos(tn->node.x * getCoordScales().x,
                           tn->node.y * getCoordScales().y,
                           tn->node.z * getCoordScales().z);
        glm::dvec3 pos = projectPointOnRay(nodePos, v1, v2) / glm::dvec3(getCoordScales());
        /*
        Swc_Tree_Node* node = Make_Swc_Tree_Node(&(tn->node));
        node->node.x = pos.x;
        node->node.y = pos.y;
        node->node.z = pos.z;
        emit extendSwcTreeNode(node, tn);
        */
        emit extendSwcTreeNode(pos.x, pos.y, pos.z);
      }
    }
    m_pressedSwc = NULL;
    m_pressedSwcTreeNode = NULL;
  }
}

void Z3DSwcFilter::updateSwcVisibleState()
{
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DSwcFilter::decompseSwcTree()
{
  m_allNodeType.clear();
  m_decompsedNodePairs.clear();
  m_decomposedNodes.clear();
  m_allNodesSet.clear();

  m_decompsedNodePairs.resize(m_swcList.size());
  m_decomposedNodes.resize(m_swcList.size());

  for (size_t i=0; i<m_swcList.size(); i++) {
    if (m_swcList[i]->isVisible()) {
      std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > allPairs;
      std::vector<Swc_Tree_Node*> allNodes;
      ZSwcTree *swcTree = m_swcList.at(i);
      swcTree->updateIterator(1);   //depth first
      for (Swc_Tree_Node *tn = swcTree->begin(); tn != swcTree->end(); tn = swcTree->next()) {
        if (!Swc_Tree_Node_Is_Virtual(tn)) {
          m_allNodeType.insert(Swc_Tree_Node_Const_Data(tn)->type);
          allNodes.push_back(tn);
          m_allNodesSet.insert(tn);
        }
        if (tn->parent != NULL && !Swc_Tree_Node_Is_Virtual(tn->parent))
          allPairs.push_back(std::pair<Swc_Tree_Node*, Swc_Tree_Node*>(tn, tn->parent));
      }
      m_decompsedNodePairs[i] = allPairs;
      m_decomposedNodes[i] = allNodes;
    }
  }
}

glm::vec4 Z3DSwcFilter::getColorByType(Swc_Tree_Node *n)
{
  if (m_colorMode.isSelected("Branch Type")) {
    if ((size_t)(n->node.type) < m_colorsForDifferentType.size() - 1) {
      return m_colorsForDifferentType[n->node.type]->get();
    } else {
      return m_colorsForDifferentType[m_colorsForDifferentType.size() - 1]->get();
    }
  } else if (m_colorMode.isSelected("Subclass")) {
    if (m_subclassTypeColorMapper.find(n->node.type) != m_subclassTypeColorMapper.end()) {
      return m_colorsForSubclassType[m_subclassTypeColorMapper[n->node.type]]->get();
    } else {
      return m_colorsForSubclassType[m_colorsForSubclassType.size() - 1]->get();
    }
  } else if (m_colorMode.isSelected("Biocytin Branch Type")) {
    return m_biocytinColorMapper[SwcTreeNode::type(n)]->get();
    //return m_colorsForBiocytinType[SwcTreeNode::type(n)]->get();
  } else  /*if (m_colorMode.get() == "ColorMap Branch Type")*/ {
    return m_colorMap.get().getMappedFColor(n->node.type);
  }
}

void Z3DSwcFilter::createColorMapperWidget(
    const std::map<ZSwcTree*, ZVec4Parameter*>& colorMapper,
    std::vector<ZWidgetsGroup*> &widgetGroup)
{
  for (size_t i=0; i<widgetGroup.size(); i++) {
    delete widgetGroup[i];
  }
  widgetGroup.clear();

  std::vector<ZParameter*> allTreeColorParas;
  for (std::map<ZSwcTree*, ZVec4Parameter*>::const_iterator it = colorMapper.begin();
       it != colorMapper.end(); ++it) {
    allTreeColorParas.push_back(it->second);
  }
  std::sort(allTreeColorParas.begin(), allTreeColorParas.end(), compareParameterName);
  for (size_t i=0; i<allTreeColorParas.size(); ++i) {
    widgetGroup.push_back(
          new ZWidgetsGroup(allTreeColorParas[i], m_widgetsGroup, 1));
  }
}

void Z3DSwcFilter::updateWidgetsGroup()
{
  if (m_widgetsGroup) {
    createColorMapperWidget(m_randomTreeColorMapper, m_randomColorWidgetGroup);
    createColorMapperWidget(m_individualTreeColorMapper,
                            m_individualColorWidgetGroup);
    /*
    for (size_t i=0; i<m_randomColorWidgetGroup.size(); i++) {
      delete m_randomColorWidgetGroup[i];
    }
    m_randomColorWidgetGroup.clear();

    std::map<ZSwcTree*, ZVec4Parameter*> *colorMapper = NULL;

    if (m_colorMode.isSelected("Individual")) {
      colorMapper = &m_individualTreeColorMapper;
    } else if (m_colorMode.isSelected("Random Tree Color")) {
      colorMapper = &m_randomTreeColorMapper;
    }

    if (colorMapper != NULL) {
      std::vector<ZParameter*> allTreeColorParas;
      for (std::map<ZSwcTree*, ZVec4Parameter*>::iterator
           it = colorMapper->begin();
           it != colorMapper->end(); ++it) {
        allTreeColorParas.push_back(it->second);
      }
      std::sort(allTreeColorParas.begin(), allTreeColorParas.end(), compareParameterName);
      for (size_t i=0; i<allTreeColorParas.size(); ++i) {
        m_randomColorWidgetGroup.push_back(
              new ZWidgetsGroup(allTreeColorParas[i], m_widgetsGroup, 1));
      }
    }
    */

    for (size_t i=0; i<m_colorsForBiocytinTypeWidgetsGroup.size(); i++) {
      delete m_colorsForBiocytinTypeWidgetsGroup[i];
    }
    m_colorsForBiocytinTypeWidgetsGroup.clear();

    for (std::map<int, ZVec4Parameter*>::iterator it = m_biocytinColorMapper.begin();
         it != m_biocytinColorMapper.end(); ++it) {
      m_colorsForBiocytinTypeWidgetsGroup.push_back(
            new ZWidgetsGroup(it->second, m_widgetsGroup, 1));
    }

    m_widgetsGroup->emitWidgetsGroupChangedSignal();
  }
}

void Z3DSwcFilter::getVisibleData()
{
  m_swcList.clear();
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    if (m_origSwcList[i]->isVisible())
      m_swcList.push_back(m_origSwcList[i]);
  }
}

glm::dvec3 Z3DSwcFilter::projectPointOnRay(glm::dvec3 pt, const glm::dvec3 &v1, const glm::dvec3 &v2)
{
  return v1 + glm::dot(pt-v1, v2-v1) * (v2-v1);
}

void Z3DSwcFilter::initialize()
{
  Z3DGeometryFilter::initialize();
  m_coneRenderer = new Z3DConeRenderer();
  m_sphereRendererForCone = new Z3DSphereRenderer();
  m_rendererBase->addRenderer(m_coneRenderer);
  m_rendererBase->addRenderer(m_sphereRendererForCone);
  m_lineRenderer = new Z3DLineRenderer();
  m_rendererBase->addRenderer(m_lineRenderer);
  m_sphereRenderer = new Z3DSphereRenderer();
  m_rendererBase->addRenderer(m_sphereRenderer);
  m_boundBoxRenderer = new Z3DLineWithFixedWidthColorRenderer();
  m_boundBoxRenderer->setUseDisplayList(false);
  m_boundBoxRenderer->setRespectRendererBaseCoordScales(false);
  m_boundBoxRenderer->setLineColorGuiName("Selection BoundBox Line Color");
  m_boundBoxRenderer->setLineWidthGuiName("Selection BoundBox Line Width");
  m_rendererBase->addRenderer(m_boundBoxRenderer);

  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
    addParameter(paras[i]);
  }

  const NeutubeConfig::Z3DWindowConfig::SwcTabConfig &config =
      NeutubeConfig::getInstance().getZ3DWindowConfig().getSwcTabConfig();
  ZFloatParameter *zscaleParam =
      dynamic_cast<ZFloatParameter*>(getParameter("Z Scale"));
  if (zscaleParam != NULL) {
    zscaleParam->set(config.getZScale());
  }
}

void Z3DSwcFilter::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //paras[i]->disconnect(this);
    removeParameter(paras[i]);
  }
  Z3DGeometryFilter::deinitialize();
}

void Z3DSwcFilter::setColorMode(const std::string &mode)
{
  m_colorMode.select(mode.c_str());
}
