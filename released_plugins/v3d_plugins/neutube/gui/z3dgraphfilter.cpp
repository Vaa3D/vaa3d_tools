#include "z3dgraphfilter.h"
#include "z3dlinerenderer.h"
#include "z3dsphererenderer.h"
#include "z3dconerenderer.h"

using namespace std;


Z3DGraphFilter::Z3DGraphFilter() : m_lineRenderer(NULL),
  m_coneRenderer(NULL), m_arrowRenderer(NULL), m_sphereRenderer(NULL), m_dataIsInvalid(true)
  , m_xCut("X Cut", glm::ivec2(0,0), 0, 0)
  , m_yCut("Y Cut", glm::ivec2(0,0), 0, 0)
  , m_zCut("Z Cut", glm::ivec2(0,0), 0, 0)
  , m_widgetsGroup(NULL)
  , m_showingArrow(false)
{
}

Z3DGraphFilter::~Z3DGraphFilter()
{

}

void Z3DGraphFilter::initialize()
{
  Z3DGeometryFilter::initialize();
  m_coneRenderer = new Z3DConeRenderer();
  m_coneRenderer->setNeedLighting(false);
  m_rendererBase->addRenderer(m_coneRenderer);
  m_arrowRenderer = new Z3DConeRenderer();
  m_rendererBase->addRenderer(m_arrowRenderer);
  m_lineRenderer = new Z3DLineRenderer();
  m_rendererBase->addRenderer(m_lineRenderer);
  m_sphereRenderer = new Z3DSphereRenderer();
  m_rendererBase->addRenderer(m_sphereRenderer);

  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
    addParameter(paras[i]);
  }
}

void Z3DGraphFilter::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    removeParameter(paras[i]);
  }
  Z3DGeometryFilter::deinitialize();
}

void Z3DGraphFilter::render(Z3DEye eye)
{
  m_rendererBase->activateRenderer(m_sphereRenderer);
  if (showingArrow()) {
    m_rendererBase->activateRenderer(m_arrowRenderer, Z3DRendererBase::None);
  }
  m_rendererBase->activateRenderer(
        m_lineRenderer, m_coneRenderer, Z3DRendererBase::None);
  m_rendererBase->render(eye);
}

void Z3DGraphFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DGraphFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  m_axisAndTopRadius.clear();
  m_baseAndBaseRadius.clear();
  m_arrowAxisAndTopRadius.clear();
  m_arrowBaseAndBaseRadius.clear();

  m_pointAndRadius.clear();
  m_lines.clear();
  int xMin = std::numeric_limits<int>::max();
  int xMax = std::numeric_limits<int>::min();
  int yMin = std::numeric_limits<int>::max();
  int yMax = std::numeric_limits<int>::min();
  int zMin = std::numeric_limits<int>::max();
  int zMax = std::numeric_limits<int>::min();

  vector<float> edgeWidth;

  for (size_t i = 0; i <m_graph.getEdgeNumber(); i++) {
    Z3DGraphNode n1 = m_graph.getStartNode(i);
    Z3DGraphNode n2 = m_graph.getEndNode(i);

    ZPoint startPos = ZPoint(n1.x(), n1.y(), n1.z());
    ZPoint endPos = ZPoint(n2.x(), n2.y(), n2.z());
    ZPoint vec = endPos - startPos;
    ZPoint normalizedVec = vec;
    normalizedVec.normalize();

    startPos = startPos + normalizedVec * 0.8 * m_graph.getStartNode(i).radius();
    endPos = endPos - normalizedVec * 0.8 * m_graph.getEndNode(i).radius();
    vec = endPos - startPos;

    glm::vec4 baseAndbRadius, axisAndtRadius;
    baseAndbRadius = glm::vec4(startPos.x(), startPos.y(), startPos.z(),
                               m_graph.getEdge(i).radius());
    axisAndtRadius = glm::vec4(vec.x(), vec.y(), vec.z(),
                               m_graph.getEdge(i).radius());

    if (m_graph.getEdge(i).shape() == GRAPH_CYLINDER) {
      m_baseAndBaseRadius.push_back(baseAndbRadius);
      m_axisAndTopRadius.push_back(axisAndtRadius);
    } else if (m_graph.getEdge(i).shape() == GRAPH_LINE) {
      m_lines.push_back(baseAndbRadius.xyz());
      m_lines.push_back(glm::vec3(baseAndbRadius.xyz()) +
                        glm::vec3(axisAndtRadius.xyz()));
      edgeWidth.push_back(m_graph.getEdge(i).radius());
    }

    glm::vec4 arrowBaseAndbRadius, arrowAxisAndtRadius;

    ZPoint arrowPos = startPos * 0.6 + endPos * 0.4;
    arrowBaseAndbRadius = glm::vec4(arrowPos.x(), arrowPos.y(), arrowPos.z(),
                                    m_graph.getEdge(i).radius() * 2 + 5);

    normalizedVec *= 25 + m_graph.getEdge(i).radius() * 3;
    arrowAxisAndtRadius = glm::vec4(
          normalizedVec.x(), normalizedVec.y(), normalizedVec.z(), 0);

    m_arrowBaseAndBaseRadius.push_back(arrowBaseAndbRadius);
    m_arrowAxisAndTopRadius.push_back(arrowAxisAndtRadius);
  }

  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    ZPoint nodePos = m_graph.getNode(i).center();
    if (nodePos.x() > xMax)
      xMax = static_cast<int>(std::ceil(nodePos.x()));
    if (nodePos.x() < xMin)
      xMin = static_cast<int>(std::floor(nodePos.x()));
    if (nodePos.y() > yMax)
      yMax = static_cast<int>(std::ceil(nodePos.y()));
    if (nodePos.y() < yMin)
      yMin = static_cast<int>(std::floor(nodePos.y()));
    if (nodePos.z() > zMax)
      zMax = static_cast<int>(nodePos.z());
    if (nodePos.z() < zMin)
      zMin = static_cast<int>(nodePos.z());

    m_pointAndRadius.push_back(
          glm::vec4(nodePos.x(), nodePos.y(), nodePos.z(),
                    m_graph.getNode(i).radius()));
  }

  m_xCut.setRange(xMin, xMax);
  m_xCut.set(glm::ivec2(xMin, xMax));
  m_yCut.setRange(yMin, yMax);
  m_yCut.set(glm::ivec2(yMin, yMax));
  m_zCut.setRange(zMin, zMax);
  m_zCut.set(glm::ivec2(zMin, zMax));

  m_coneRenderer->setData(&m_baseAndBaseRadius, &m_axisAndTopRadius);
  m_arrowRenderer->setData(&m_arrowBaseAndBaseRadius, &m_arrowAxisAndTopRadius);
  m_lineRenderer->setData(&m_lines);
  m_lineRenderer->setLineWidth(3.0);
  m_lineRenderer->setLineWidth(edgeWidth);
  m_sphereRenderer->setData(&m_pointAndRadius);

  prepareColor();

  m_dataIsInvalid = false;
}

void Z3DGraphFilter::prepareColor()
{
  m_pointColors.resize(m_graph.getNodeNumber());
  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    m_pointColors[i] = glm::vec4(m_graph.getNode(i).color().redF(),
                                 m_graph.getNode(i).color().greenF(),
                                 m_graph.getNode(i).color().blueF(),
                                 m_graph.getNode(i).color().alphaF());
  }

  //m_lineColors.resize(m_graph.getEdgeNumber() * 2);
  //m_lineStartColors.resize(m_graph.getEdgeNumber());
  //m_lineEndColors.resize(m_graph.getEdgeNumber());
  m_arrowStartColors.resize(m_graph.getEdgeNumber());
  m_arrowEndColors.resize(m_graph.getEdgeNumber());

  for (size_t i = 0;i < m_graph.getEdgeNumber(); i++) {
#if _DEBUG_2
    cout << "color: "
         << m_graph.getEdge(i).color().redF() << " "
         << m_graph.getEdge(i).color().greenF() << " "
         << m_graph.getEdge(i).color().blueF() << " "
         << m_graph.getEdge(i).color().alphaF()
         << endl;
#endif

    glm::vec4 startColor;
    glm::vec4 endColor;

    if (m_graph.getEdge(i).usingNodeColor()) {
      startColor = glm::vec4(
            m_graph.getStartNode(i).color().redF(),
            m_graph.getStartNode(i).color().greenF(),
            m_graph.getStartNode(i).color().blueF(),
            m_graph.getStartNode(i).color().alphaF());

      endColor = glm::vec4(
            m_graph.getEndNode(i).color().redF(),
            m_graph.getEndNode(i).color().greenF(),
            m_graph.getEndNode(i).color().blueF(),
            m_graph.getEndNode(i).color().alphaF());
    } else {
#ifdef _DEBUG_2
      cout << m_graph.getEdge(i).startColor().alphaF() << endl;
#endif

      startColor = glm::vec4(m_graph.getEdge(i).startColor().redF(),
                                      m_graph.getEdge(i).startColor().greenF(),
                                      m_graph.getEdge(i).startColor().blueF(),
                                      m_graph.getEdge(i).startColor().alphaF());
      endColor = glm::vec4(m_graph.getEdge(i).endColor().redF(),
                                          m_graph.getEdge(i).endColor().greenF(),
                                          m_graph.getEdge(i).endColor().blueF(),
                                          m_graph.getEdge(i).endColor().alphaF());
    }

    if (m_graph.getEdge(i).shape() == GRAPH_CYLINDER) {
      m_lineStartColors.push_back(startColor);
      m_lineEndColors.push_back(endColor);
    } else if (m_graph.getEdge(i).shape() == GRAPH_LINE) {
      m_lineColors.push_back(startColor);
      m_lineColors.push_back(endColor);
    }

   // m_lineStartColors[i] = startColor;
    //m_lineEndColors[i] = endColor;

    m_arrowStartColors[i] = startColor * 0.4 + endColor * 0.6;
    m_arrowStartColors[i][3] *= 0.5;
    m_arrowEndColors[i] = m_arrowStartColors[i];
  }

  m_coneRenderer->setDataColors(&m_lineStartColors, &m_lineEndColors);
  m_arrowRenderer->setDataColors(&m_arrowStartColors, &m_arrowEndColors);
  m_lineRenderer->setDataColors(&m_lineColors);
  m_sphereRenderer->setDataColors(&m_pointColors);
}

vector<double> Z3DGraphFilter::boundBox()
{
  vector<double> result(6, 0);

  for (size_t i = 0; i < 3; i++) {
    result[i * 2] = numeric_limits<double>::max();
    result[i * 2 + 1] = -numeric_limits<double>::max();
  }

  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    ZPoint pos = m_graph.getNode(i).center();
    result[0] = min(result[0], pos.x() - m_graph.getNode(i).radius() * 2.0);
    result[1] = max(result[1], pos.x() + m_graph.getNode(i).radius() * 2.0);
    result[2] = min(result[2], pos.y() - m_graph.getNode(i).radius() * 2.0);
    result[3] = max(result[3], pos.y() + m_graph.getNode(i).radius() * 2.0);
    result[4] = min(result[4], pos.z() - m_graph.getNode(i).radius() * 2.0);
    result[5] = max(result[5], pos.z() + m_graph.getNode(i).radius() * 2.0);
  }

  return result;
}

void Z3DGraphFilter::setData(const ZPointNetwork &pointNetwork,
                               ZNormColorMap *colorMap)
{
  m_graph.importPointNetwork(pointNetwork, colorMap);

  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DGraphFilter::setData(const Z3DGraph &graph)
{
  m_graph = graph;
}

ZWidgetsGroup *Z3DGraphFilter::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Graph", NULL, 1);

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
