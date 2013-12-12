#include "z3daxis.h"

#include "z3dlinerenderer.h"
#include "z3darrowrenderer.h"
#include "z3dfontrenderer.h"
#include "zwidgetsgroup.h"

Z3DAxis::Z3DAxis()
  : Z3DGeometryFilter()
  , m_showAxis("Show Axis", true)
  , m_XAxisColor("X Axis Color", glm::vec4(1.f, 0.f, 0.f, 1.0f))
  , m_YAxisColor("Y Axis Color", glm::vec4(0.f, 1.f, 0.f, 1.0f))
  , m_ZAxisColor("Z Axis Color", glm::vec4(0.f, 0.f, 1.f, 1.0f))
  , m_axisRegionRatio("Axis Region Ratio", .2f, .1f, 1.f)
  , m_mode("mode")
  , m_widgetsGroup(NULL)
{
  m_XAxisColor.setStyle("COLOR");
  m_YAxisColor.setStyle("COLOR");
  m_ZAxisColor.setStyle("COLOR");
  m_mode.addOptions("Arrow", "Line");
  m_mode.select("Arrow");
  addParameter(m_showAxis);
  addParameter(m_XAxisColor);
  addParameter(m_YAxisColor);
  addParameter(m_ZAxisColor);
  addParameter(m_axisRegionRatio);
  addParameter(m_mode);
  setNeedBlending(true);
}

Z3DAxis::~Z3DAxis()
{
}

bool Z3DAxis::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && m_showAxis.get();
}

ZWidgetsGroup *Z3DAxis::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Axis", NULL, 1);
    new ZWidgetsGroup(&m_showAxis, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_mode, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_axisRegionRatio, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_XAxisColor, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_YAxisColor, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_ZAxisColor, m_widgetsGroup, 1);
    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      if (para->getName() == "Size Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 1);
      else if (para->getName() == "Rendering Method")
        new ZWidgetsGroup(para, m_widgetsGroup, 3);
      else if (para->getName() == "Opacity")
        new ZWidgetsGroup(para, m_widgetsGroup, 3);
      else if (para->getName().contains("Font"))
        new ZWidgetsGroup(para, m_widgetsGroup, 3);
    }
    m_widgetsGroup->setBasicAdvancedCutoff(4);
  }
  return m_widgetsGroup;
}

void Z3DAxis::initialize()
{
  Z3DGeometryFilter::initialize();
  m_arrowRenderer = new Z3DArrowRenderer();
  m_arrowRenderer->setUseDisplayList(false);
  m_arrowRenderer->setRespectRendererBaseCoordScales(false);
  m_rendererBase->addRenderer(m_arrowRenderer);
  m_lineRenderer = new Z3DLineRenderer();
  m_lineRenderer->setUseDisplayList(false);
  m_lineRenderer->setRespectRendererBaseCoordScales(false);
  m_rendererBase->addRenderer(m_lineRenderer);
  m_fontRenderer = new Z3DFontRenderer();
  m_fontRenderer->setRespectRendererBaseCoordScales(false);
  m_rendererBase->addRenderer(m_fontRenderer);
  setupCamera();

  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
    addParameter(paras[i]);
  }
}

void Z3DAxis::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //paras[i]->disconnect(this);
    removeParameter(paras[i]);
  }
  Z3DGeometryFilter::deinitialize();
}

void Z3DAxis::render(Z3DEye eye)
{
  if (!m_showAxis.get())
    return;
  prepareData(eye);

  if (m_mode.get() == "Arrow")
    m_rendererBase->activateRenderer(m_arrowRenderer, m_fontRenderer);
  else
    m_rendererBase->activateRenderer(m_lineRenderer, m_fontRenderer);
  glm::ivec4 viewport = m_rendererBase->getViewport();
  int size = std::min(viewport.z, viewport.w) * m_axisRegionRatio.get();
  glViewport(viewport.x, viewport.y, size, size);
  m_rendererBase->render(eye);

  glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

void Z3DAxis::prepareData(Z3DEye eye)
{
  m_tailPosAndTailRadius.clear();
  m_headPosAndHeadRadius.clear();
  m_lineColors.clear();
  m_lines.clear();
  m_textColors.clear();
  m_textPositions.clear();
  glm::mat3 rotMatrix = m_rendererBase->getCamera().getRotateMatrix(eye);
  m_XEnd = rotMatrix * glm::vec3(256.f, 0.f, 0.f);
  m_YEnd = rotMatrix * glm::vec3(0.f, 256.f, 0.f);
  m_ZEnd = rotMatrix * glm::vec3(0.f, 0.f, 256.f);
  glm::vec3 origin(0.f);
  m_lines.push_back(origin);
  m_lineColors.push_back(m_XAxisColor.get());
  m_lines.push_back(m_XEnd*glm::vec3(0.88));
  m_lineColors.push_back(m_XAxisColor.get());
  m_lines.push_back(origin);
  m_lineColors.push_back(m_YAxisColor.get());
  m_lines.push_back(m_YEnd*glm::vec3(0.88));
  m_lineColors.push_back(m_YAxisColor.get());
  m_lines.push_back(origin);
  m_lineColors.push_back(m_ZAxisColor.get());
  m_lines.push_back(m_ZEnd*glm::vec3(0.88));
  m_lineColors.push_back(m_ZAxisColor.get());

  m_textPositions.push_back(m_XEnd*glm::vec3(0.93));
  m_textPositions.push_back(m_YEnd*glm::vec3(0.93));
  m_textPositions.push_back(m_ZEnd*glm::vec3(0.93));
  m_textColors.push_back(m_XAxisColor.get());
  m_textColors.push_back(m_YAxisColor.get());
  m_textColors.push_back(m_ZAxisColor.get());
  QStringList texts;
  texts.push_back("X");
  texts.push_back("Y");
  texts.push_back("Z");

  float tailRadius = 5.f;
  float headRadius = 10.f;

  m_tailPosAndTailRadius.push_back(glm::vec4(origin, tailRadius));
  m_headPosAndHeadRadius.push_back(glm::vec4(m_XEnd*glm::vec3(0.88), headRadius));

  m_tailPosAndTailRadius.push_back(glm::vec4(origin, tailRadius));
  m_headPosAndHeadRadius.push_back(glm::vec4(m_YEnd*glm::vec3(0.88), headRadius));

  m_tailPosAndTailRadius.push_back(glm::vec4(origin, tailRadius));
  m_headPosAndHeadRadius.push_back(glm::vec4(m_ZEnd*glm::vec3(0.88), headRadius));

  m_lineRenderer->setData(&m_lines);
  m_lineRenderer->setDataColors(&m_lineColors);
  m_arrowRenderer->setArrowData(&m_tailPosAndTailRadius, &m_headPosAndHeadRadius, .1f);
  m_arrowRenderer->setArrowColors(&m_textColors);
  m_fontRenderer->setData(&m_textPositions, texts);
  m_fontRenderer->setDataColors(&m_textColors);
}

void Z3DAxis::setupCamera()
{
  Z3DCamera camera;
  glm::vec3 center(0.f);
  camera.setFieldOfView(10.f);

  float radius = 300.f;

  float distance = radius/std::sin(glm::radians(camera.getFieldOfView())*0.5);
  glm::vec3 vn(0, 0, 1);     //plane normal
  glm::vec3 position = center + vn * distance;
  camera.setCamera(position, center, glm::vec3(0.0, 1.0, 0.0));
  camera.setNearDist(distance - radius - 1);
  camera.setFarDist(distance + radius);

  m_rendererBase->setViewMatrix(camera.getViewMatrix(CenterEye));
  m_rendererBase->setProjectionMatrix(camera.getProjectionMatrix(CenterEye));
}
