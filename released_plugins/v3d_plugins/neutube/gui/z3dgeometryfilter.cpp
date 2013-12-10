#include "z3dgeometryfilter.h"

Z3DGeometryFilter::Z3DGeometryFilter()
  : Z3DProcessor()
  , m_outPort("GeometryFilter")
  , m_stayOnTop("Stay On Top", false)
  , m_pickingManager(NULL)
  , m_pickingObjectsRegistered(false)
  , m_needBlending(false)
{
  addPort(m_outPort);
  m_rendererBase = new Z3DRendererBase();
  addParameter(m_stayOnTop);
}

Z3DGeometryFilter::~Z3DGeometryFilter()
{
  delete m_rendererBase;
}

void Z3DGeometryFilter::setPickingManager(Z3DPickingManager *pm)
{
  if (m_pickingManager != pm) {
    deregisterPickingObjects(m_pickingManager);
    m_pickingManager = pm;
    registerPickingObjects(m_pickingManager);
  }
}

void Z3DGeometryFilter::get3DRayUnderScreenPoint(glm::vec3 &v1, glm::vec3 &v2, int x, int y, int width, int height)
{
  glm::mat4 projection = getCamera().getProjectionMatrix(CenterEye);
  glm::mat4 modelview = getCamera().getViewMatrix(CenterEye);

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  v1 = glm::unProject(glm::vec3(x, height-y, 0.f), modelview, projection, viewport);
  v2 = glm::unProject(glm::vec3(x, height-y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2-v1) + v1;
}

void Z3DGeometryFilter::get3DRayUnderScreenPoint(glm::dvec3 &v1, glm::dvec3 &v2, int x, int y, int width, int height)
{
  glm::dmat4 projection = glm::dmat4(getCamera().getProjectionMatrix(CenterEye));
  glm::dmat4 modelview = glm::dmat4(getCamera().getViewMatrix(CenterEye));

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  v1 = glm::unProject(glm::dvec3(x, height-y, 0.f), modelview, projection, viewport);
  v2 = glm::unProject(glm::dvec3(x, height-y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2-v1) + v1;
}
