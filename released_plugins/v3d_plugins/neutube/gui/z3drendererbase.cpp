#include "z3drendererbase.h"

#include "z3dprimitiverenderer.h"
#include "z3dgpuinfo.h"
#define _USE_MATH_DEFINES
#include <cmath>

Z3DRendererBase::Z3DRendererBase(QObject *parent)
  : QObject(parent)
  , m_displayList(0)
  , m_pickingDisplayList(0)
  , m_coordXScale("X Scale", 1.0f, 0.1f, 50.f)
  , m_coordYScale("Y Scale", 1.0f, 0.1f, 50.f)
  , m_coordZScale("Z Scale", 1.0f, 0.1f, 50.f)
  , m_renderMethod("Rendering Method")
  , m_sizeScale("Size Scale", 1.f, .1f, 20.f)
  , m_opacity("Opacity", 1.0f, .0f, 1.f)
  , m_filterNotFrontFacing("Filter Not Front Facing", true)
  , m_materialAmbient("Material Ambient", glm::vec4(0.1f, .1f, .1f, 1.f))
  , m_materialSpecular("Material Specular", glm::vec4(1.f, 1.f, 1.f, 1.f))
  , m_materialShininess("Material Shininess", 100.f, 1.f, 200.f)
  , m_lightCount("Light Count", 5, 1, 5)
  , m_sceneAmbient("Scene Ambient", glm::vec4(0.2f, 0.2f, 0.2f, 1.f))
  , m_fogMode("Fog Mode")
  , m_fogTopColor("Fog Top Color", glm::vec3(.9f, .9f, .9f))
  , m_fogBottomColor("Fog Bottom Color", glm::vec3(.9f, .9f, .9f))
  , m_fogRange("Fog Range", glm::ivec2(100, 50000), 1, 1e5)
  , m_fogDensity("Fog Denstiy", 1.f, 0.0001f, 10.f)
  , m_camera()
  , m_viewport(-1)
  , m_hasCustomViewMatrix(false)
  , m_hasCustomProjectionMatrix(false)
  , m_shaderHookType(Normal)
{
  // lights
  QString lightname = "Key Light";
  QString name = lightname + " Position";
  m_lightPositions.push_back(new ZVec4Parameter(name, glm::vec4(0.1116f, 0.7660f, 0.6330f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  m_lightAmbients.push_back(new ZVec4Parameter(name, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
  name = lightname + " Diffuse";
  m_lightDiffuses.push_back(new ZVec4Parameter(name, glm::vec4(0.75f, 0.75f, 0.75f, 1.0f)));
  name = lightname + " Specular";
  m_lightSpeculars.push_back(new ZVec4Parameter(name, glm::vec4(0.85f, 0.85f, 0.85f, 1.0f)));
  name = lightname + " Attenuation";
  m_lightAttenuations.push_back(new ZVec3Parameter(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  m_lightSpotCutoff.push_back(new ZFloatParameter(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  m_lightSpotExponent.push_back(new ZFloatParameter(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  m_lightSpotDirection.push_back(new ZVec3Parameter(name, glm::vec3(-0.1116f, -0.7660f, -0.6330f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Head Light";
  name = lightname + " Position";
  m_lightPositions.push_back(new ZVec4Parameter(name, glm::vec4(0.f, 0.f, 1.f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  m_lightAmbients.push_back(new ZVec4Parameter(name, glm::vec4(0.1* 0.333f, 0.1* 0.333f, 0.1* 0.333f, 1.0f)));
  name = lightname + " Diffuse";
  m_lightDiffuses.push_back(new ZVec4Parameter(name, glm::vec4(0.75* 0.333f, 0.75* 0.333f, 0.75* 0.333f, 1.0f)));
  name = lightname + " Specular";
  m_lightSpeculars.push_back(new ZVec4Parameter(name, glm::vec4(0.f, 0.f, 0.f, 1.0f)));
  name = lightname + " Attenuation";
  m_lightAttenuations.push_back(new ZVec3Parameter(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  m_lightSpotCutoff.push_back(new ZFloatParameter(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  m_lightSpotExponent.push_back(new ZFloatParameter(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  m_lightSpotDirection.push_back(new ZVec3Parameter(name, glm::vec3(0.f, 0.f, -1.f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Fill Light";
  name = lightname + " Position";
  m_lightPositions.push_back(new ZVec4Parameter(name, glm::vec4(-0.0449f, -0.9659f, 0.2549f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  m_lightAmbients.push_back(new ZVec4Parameter(name, glm::vec4(0.1* 0.333f, 0.1* 0.333f, 0.1* 0.333f, 1.0f)));
  name = lightname + " Diffuse";
  m_lightDiffuses.push_back(new ZVec4Parameter(name, glm::vec4(0.75* 0.333f, 0.75* 0.333f, 0.75* 0.333f, 1.0f)));
  name = lightname + " Specular";
  m_lightSpeculars.push_back(new ZVec4Parameter(name, glm::vec4(0.85* 0.333f, 0.85* 0.333f, 0.85* 0.333f, 1.0f)));
  name = lightname + " Attenuation";
  m_lightAttenuations.push_back(new ZVec3Parameter(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  m_lightSpotCutoff.push_back(new ZFloatParameter(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  m_lightSpotExponent.push_back(new ZFloatParameter(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  m_lightSpotDirection.push_back(new ZVec3Parameter(name, glm::vec3(0.0449f, 0.9659f, -0.2549f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Back Light 1";
  name = lightname + " Position";
  m_lightPositions.push_back(new ZVec4Parameter(name, glm::vec4(0.9397f, 0.f, -0.3420f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  m_lightAmbients.push_back(new ZVec4Parameter(name, glm::vec4(0.1* 0.27f, 0.1* 0.27f, 0.1* 0.27f, 1.0f) ));
  name = lightname + " Diffuse";
  m_lightDiffuses.push_back(new ZVec4Parameter(name, glm::vec4(0.75* 0.27f, 0.75* 0.27f, 0.75* 0.27f, 1.0f) ));
  name = lightname + " Specular";
  m_lightSpeculars.push_back(new ZVec4Parameter(name, glm::vec4(0.85* 0.27f, 0.85* 0.27f, 0.85* 0.27f, 1.0f) ));
  name = lightname + " Attenuation";
  m_lightAttenuations.push_back(new ZVec3Parameter(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  m_lightSpotCutoff.push_back(new ZFloatParameter(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  m_lightSpotExponent.push_back(new ZFloatParameter(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  m_lightSpotDirection.push_back(new ZVec3Parameter(name, glm::vec3(-0.9397f, 0.f, 0.3420f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Back Light 2";
  name = lightname + " Position";
  m_lightPositions.push_back(new ZVec4Parameter(name, glm::vec4(-0.9397f, 0.f, -0.3420f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  m_lightAmbients.push_back(new ZVec4Parameter(name, glm::vec4(0.1* 0.27f, 0.1* 0.27f, 0.1* 0.27f, 1.0f) ));
  name = lightname + " Diffuse";
  m_lightDiffuses.push_back(new ZVec4Parameter(name, glm::vec4(0.75* 0.27f, 0.75* 0.27f, 0.75* 0.27f, 1.0f) ));
  name = lightname + " Specular";
  m_lightSpeculars.push_back(new ZVec4Parameter(name, glm::vec4(0.85* 0.27f, 0.85* 0.27f, 0.85* 0.27f, 1.0f) ));
  name = lightname + " Attenuation";
  m_lightAttenuations.push_back(new ZVec3Parameter(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  m_lightSpotCutoff.push_back(new ZFloatParameter(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  m_lightSpotExponent.push_back(new ZFloatParameter(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  m_lightSpotDirection.push_back(new ZVec3Parameter(name, glm::vec3(0.9397f, 0.f, 0.3420f), glm::vec3(-1.f), glm::vec3(1.f)));

  m_renderMethod.addOptions("GLSL", "Old openGL");
  m_renderMethod.select("GLSL");

  m_sizeScale.setSingleStep(0.1);
  m_sizeScale.setDecimal(1);

  addParameter(m_coordXScale);
  addParameter(m_coordYScale);
  addParameter(m_coordZScale);
  addParameter(m_sizeScale);
  //addParameter(m_renderMethod);
  addParameter(m_opacity);

  m_materialAmbient.setStyle("COLOR");
  m_materialSpecular.setStyle("COLOR");
  addParameter(m_materialAmbient);
  addParameter(m_materialSpecular);
  addParameter(m_materialShininess);

  connect(&m_lightCount, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_lightCount, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  addParameter(m_lightCount);

  connect(&m_coordXScale, SIGNAL(valueChanged()), this, SIGNAL(coordScalesChanged()));
  connect(&m_coordXScale, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  connect(&m_coordXScale, SIGNAL(valueChanged()), this, SLOT(invalidatePickingDisplayList()));
  connect(&m_coordYScale, SIGNAL(valueChanged()), this, SIGNAL(coordScalesChanged()));
  connect(&m_coordYScale, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  connect(&m_coordYScale, SIGNAL(valueChanged()), this, SLOT(invalidatePickingDisplayList()));
  connect(&m_coordZScale, SIGNAL(valueChanged()), this, SIGNAL(coordScalesChanged()));
  connect(&m_coordZScale, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  connect(&m_coordZScale, SIGNAL(valueChanged()), this, SLOT(invalidatePickingDisplayList()));
  connect(&m_sizeScale, SIGNAL(valueChanged()), this, SIGNAL(sizeScaleChanged()));
  connect(&m_sizeScale, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  connect(&m_sizeScale, SIGNAL(valueChanged()), this, SLOT(invalidatePickingDisplayList()));
  connect(&m_opacity, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  connect(&m_materialShininess, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
  connect(&m_materialSpecular, SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));

  for (size_t i=0; i<m_lightPositions.size(); i++) {
    connect(m_lightPositions[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightAmbients[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightDiffuses[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightSpeculars[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightAttenuations[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightSpotCutoff[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightSpotExponent[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));
    connect(m_lightSpotDirection[i], SIGNAL(valueChanged()), this, SLOT(invalidateDisplayList()));

    m_lightAmbients[i]->setStyle("COLOR");
    m_lightDiffuses[i]->setStyle("COLOR");
    m_lightSpeculars[i]->setStyle("COLOR");
    addParameter(m_lightPositions[i]);
    addParameter(m_lightAmbients[i]);
    addParameter(m_lightDiffuses[i]);
    addParameter(m_lightSpeculars[i]);
    addParameter(m_lightAttenuations[i]);
    addParameter(m_lightSpotCutoff[i]);
    addParameter(m_lightSpotExponent[i]);
    addParameter(m_lightSpotDirection[i]);
  }

  m_sceneAmbient.setStyle("COLOR");
  addParameter(m_sceneAmbient);

  // fog
  m_fogMode.addOptions("None", "Linear", "Exponential", "Squared Exponential");
  m_fogMode.select("None");
  connect(&m_fogMode, SIGNAL(valueChanged()), this, SLOT(compile()));
  addParameter(m_fogMode);
  m_fogTopColor.setStyle("COLOR");
  m_fogBottomColor.setStyle("COLOR");
  m_fogRange.setSingleStep(1);
  m_fogDensity.setSingleStep(0.00001);
  m_fogDensity.setDecimal(5);
  addParameter(m_fogTopColor);
  addParameter(m_fogBottomColor);
  addParameter(m_fogRange);
  addParameter(m_fogDensity);
}

Z3DRendererBase::~Z3DRendererBase()
{
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    m_renderersIt->first->deinitialize();
    delete m_renderersIt->first;
  }
  m_renderers.clear();
  if (glIsList(m_displayList))
    glDeleteLists(m_displayList, 1);
  if (glIsList(m_pickingDisplayList))
    glDeleteLists(m_pickingDisplayList, 1);
  for(size_t i=0; i<m_lightPositions.size(); i++) {
    delete m_lightPositions[i];
    delete m_lightAmbients[i];
    delete m_lightDiffuses[i];
    delete m_lightSpeculars[i];
    delete m_lightAttenuations[i];
    delete m_lightSpotCutoff[i];
    delete m_lightSpotExponent[i];
    delete m_lightSpotDirection[i];
  }
}

void Z3DRendererBase::setGlobalShaderParameters(Z3DShaderProgram &shader, Z3DEye eye)
{
  shader.setLogUniformLocationError(false);

  shader.setUniformValue("screen_dim", glm::vec2(m_viewport.z, m_viewport.w));
  shader.setUniformValue("screen_dim_RCP", 1.f / glm::vec2(m_viewport.z, m_viewport.w));

  // camera position in world coordinates, and corresponding transformation matrices
  shader.setUniformValue("camera_position", m_camera.getEye());

  shader.setUniformValue("view_matrix", getViewMatrix(eye));
  shader.setUniformValue("view_matrix_inverse", getViewMatrixInverse(eye));
  shader.setUniformValue("projection_matrix", getProjectionMatrix(eye));
  shader.setUniformValue("projection_matrix_inverse", getProjectionMatrixInverse(eye));
  shader.setUniformValue("normal_matrix", getNormalMatrix(eye));
  shader.setUniformValue("viewport_matrix", getViewportMatrix());
  shader.setUniformValue("viewport_matrix_inverse", getViewportMatrixInverse());
  shader.setUniformValue("projection_view_matrix", getProjectionMatrix(eye) * getViewMatrix(eye));

  shader.setUniformValue("gamma", 2.f);

  shader.setUniformValue("size_scale", getSizeScale());
  shader.setUniformValue("pos_scale", getCoordScales());

  for (int i=0; i<m_lightCount.get(); i++) {
    shader.setUniformValue(QString("lights[%1].position").arg(i), m_lightPositions[i]->get());
    shader.setUniformValue(QString("lights[%1].ambient").arg(i), m_lightAmbients[i]->get());
    shader.setUniformValue(QString("lights[%1].diffuse").arg(i), m_lightDiffuses[i]->get());
    shader.setUniformValue(QString("lights[%1].specular").arg(i), m_lightSpeculars[i]->get());
    shader.setUniformValue(QString("lights[%1].spotCutoff").arg(i), m_lightSpotCutoff[i]->get());
    shader.setUniformValue(QString("lights[%1].attenuation").arg(i), m_lightAttenuations[i]->get());
    shader.setUniformValue(QString("lights[%1].spotExponent").arg(i), m_lightSpotExponent[i]->get());
    shader.setUniformValue(QString("lights[%1].spotDirection").arg(i), m_lightSpotDirection[i]->get());
  }
  shader.setUniformValue("material_specular", m_materialSpecular.get());
  shader.setUniformValue("material_shininess", m_materialShininess.get());
  shader.setUniformValue("material_ambient", m_materialAmbient.get());
  shader.setUniformValue("ortho", m_camera.isPerspectiveProjection() ? 0.f : 1.f);
  shader.setUniformValue("scene_ambient", m_sceneAmbient.get());
  shader.setUniformValue("filter_not_front_facing", m_filterNotFrontFacing.get());
  shader.setUniformValue("alpha", m_opacity.get());

  if (!m_fogMode.isSelected("None")) {
    shader.setUniformValue("fog_color_top", m_fogTopColor.get());
    shader.setUniformValue("fog_color_bottom", m_fogBottomColor.get());
  }
  if (m_fogMode.isSelected("Linear")) {
    shader.setUniformValue("fog_end", static_cast<GLfloat>((m_fogRange.get().y)));
    shader.setUniformValue("fog_scale", static_cast<GLfloat>(1.f/ (m_fogRange.get().y-m_fogRange.get().x)));
  } else if (m_fogMode.isSelected("Exponential")) {
    shader.setUniformValue("fog_density_log2e", m_fogDensity.get() * (float)M_LOG2E);
  } else if (m_fogMode.isSelected("Squared Exponential")) {
    shader.setUniformValue("fog_density_density_log2e", m_fogDensity.get() * m_fogDensity.get() * (float)M_LOG2E);
  }

  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    shader.setUniformValue(QString("clip_planes[%1]").arg(i), glm::vec4(m_clipPlanes[i]));
  }

  shader.setLogUniformLocationError(true);
  CHECK_GL_ERROR;
}

void Z3DRendererBase::setGlobalShaderParameters(Z3DShaderProgram *shader, Z3DEye eye)
{
  setGlobalShaderParameters(*shader, eye);
}

QString Z3DRendererBase::generateHeader() const
{
  QString glslVer = QString("%1%2").arg(Z3DGpuInfoInstance.getGlslMajorVersion()).arg(Z3DGpuInfoInstance.getGlslMinorVersion());
  if (glslVer.length() < 3)
    glslVer += "0";

  QString header = QString("#version %1\n").arg(glslVer);

  header += QString("#define GLSL_VERSION %1\n").arg(glslVer);

  if (GLEW_VERSION_3_0) {
    header += "out vec4 FragData0;\n";
  } else {
    header += "#define FragData0 gl_FragData[0]\n";
  }

  header += QString("#define LIGHT_COUNT %1\n").arg(m_lightCount.get());

  if (!m_clipPlanes.empty()) {
    header += QString("#define HAS_CLIP_PLANE\n");
  }
  if (GLEW_VERSION_3_0)
    header += QString("#define CLIP_PLANE_COUNT %1\n").arg(m_clipPlanes.size());

  if (m_fogMode.isSelected("Linear")) {
    header += "#define USE_LINEAR_FOG\n";
  } else if (m_fogMode.isSelected("Exponential")) {
    header += "#define USE_EXPONENTIAL_FOG\n";
  } else if (m_fogMode.isSelected("Squared Exponential")) {
    header += "#define USE_SQUARED_EXPONENTIAL_FOG\n";
  }

  return header;
}

void Z3DRendererBase::addRenderer(Z3DPrimitiveRenderer *renderer)
{
  if (renderer == NULL || m_renderers.find(renderer) != m_renderers.end())
    return;

  renderer->setRendererBase(this);

  renderer->initialize();
  renderer->setInitialized(true);
  connect(renderer, SIGNAL(openglRendererInvalid()), this, SLOT(invalidateDisplayList()));
  connect(renderer, SIGNAL(openglPickingRendererInvalid()), this, SLOT(invalidatePickingDisplayList()));
  connect(this, SIGNAL(coordScalesChanged()), renderer, SLOT(coordScalesChanged()));

  m_renderers[renderer] = false;
}

void Z3DRendererBase::removeRenderer(Z3DPrimitiveRenderer *renderer)
{
  if (renderer == NULL || m_renderers.find(renderer) == m_renderers.end()) {
    LERROR() << "can not find input renderer in this rendererbase";
    return;
  }

  renderer->deinitialize();
  renderer->setInitialized(false);
  delete renderer;
  m_renderers.erase(renderer);
}

void Z3DRendererBase::deactivateAllRenderers()
{
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    m_renderersIt->second = false;
  }
}

void Z3DRendererBase::activateRenderer(Z3DPrimitiveRenderer *renderer, ActivateRendererOption option)
{
  if (option == DeactivateOthers)
    deactivateAllRenderers();
  if (m_renderers.find(renderer) == m_renderers.end()) {
    LERROR() << "can not find input renderer in this rendererbase";
    return;
  }
  m_renderers[renderer] = true;
}

void Z3DRendererBase::activateRenderer(Z3DPrimitiveRenderer *renderer1, Z3DPrimitiveRenderer *renderer2,
                                       ActivateRendererOption option)
{
  if (option == DeactivateOthers)
    deactivateAllRenderers();
  if (m_renderers.find(renderer1) == m_renderers.end() ||
      m_renderers.find(renderer2) == m_renderers.end()) {
    LERROR() << "can not find input renderer in this rendererbase";
    return;
  }
  m_renderers[renderer1] = true;
  m_renderers[renderer2] = true;
}

void Z3DRendererBase::activateRenderer(Z3DPrimitiveRenderer *renderer1, Z3DPrimitiveRenderer *renderer2,
                                       Z3DPrimitiveRenderer *renderer3, ActivateRendererOption option)
{
  if (option == DeactivateOthers)
    deactivateAllRenderers();
  if (m_renderers.find(renderer1) == m_renderers.end() ||
      m_renderers.find(renderer2) == m_renderers.end() ||
      m_renderers.find(renderer3) == m_renderers.end()) {
    LERROR() << "can not find input renderer in this rendererbase";
    return;
  }
  m_renderers[renderer1] = true;
  m_renderers[renderer2] = true;
  m_renderers[renderer3] = true;
}

void Z3DRendererBase::activateRenderer(const std::vector<Z3DPrimitiveRenderer *> &renderers,
                                       ActivateRendererOption option)
{
  if (option == DeactivateOthers)
    deactivateAllRenderers();
  for (size_t i=0; i<renderers.size(); ++i) {
    if (m_renderers.find(renderers[i]) == m_renderers.end()) {
      LERROR() << "can not find input renderer in this rendererbase";
      continue;
    }
    m_renderers[renderers[i]] = true;
  }
}

glm::mat4 Z3DRendererBase::getViewMatrix(Z3DEye eye)
{
  if (m_hasCustomViewMatrix)
    return m_customViewMatrix;
  else
    return m_camera.getViewMatrix(eye);
}

glm::mat4 Z3DRendererBase::getViewMatrixInverse(Z3DEye eye)
{
  glm::mat4 viewInvert = glm::inverse(getViewMatrix(eye));
  return viewInvert;
}

glm::mat4 Z3DRendererBase::getProjectionMatrix(Z3DEye eye)
{
  if (m_hasCustomProjectionMatrix)
    return m_customProjectionMatrix;
  else
    return m_camera.getProjectionMatrix(eye);
}

glm::mat4 Z3DRendererBase::getProjectionMatrixInverse(Z3DEye eye)
{
  glm::mat4 projInvert = glm::inverse(getProjectionMatrix(eye));
  return projInvert;
}

glm::mat3 Z3DRendererBase::getNormalMatrix(Z3DEye eye)
{
  glm::mat4 viewMatrix = getViewMatrix(eye);
  glm::mat3 ul(viewMatrix);
  glm::mat3 normalMatrix = glm::inverse(ul);
  return glm::transpose(normalMatrix);
}

glm::mat4 Z3DRendererBase::getViewportMatrix()
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  float l = viewport[0];
  float b = viewport[1];
  float r = viewport[2] + l;
  float t = viewport[3] + b;
  GLfloat depthrange[2];
  glGetFloatv(GL_DEPTH_RANGE, depthrange);
  float n = depthrange[0];
  float f = depthrange[1];
  return glm::mat4(
        glm::vec4((r-l)/2.f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, (t-b)/2.f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, (f-n)/2.f, 0.0f),
        glm::vec4((r+l)/2.f, (t+b)/2.f, (f+n)/2.f, 1.0f)
        );
}

glm::mat4 Z3DRendererBase::getViewportMatrixInverse()
{
  return glm::inverse(getViewportMatrix());
}

void Z3DRendererBase::setClipPlanes(std::vector<glm::dvec4> *clipPlanes)
{
  size_t nOldClipPlanes = m_clipPlanes.size();
  m_clipPlanes.clear();
  if (clipPlanes) {
    m_clipPlanes = *clipPlanes;
  }
  size_t nNewClipPlanes = m_clipPlanes.size();
  if (nNewClipPlanes != nOldClipPlanes)  // need to recompile shader to define or undefine HAS_CLIP_PLANE
    compile();
  invalidateDisplayList();
  invalidatePickingDisplayList();
}

void Z3DRendererBase::addParameter(ZParameter &para)
{
  addParameter(&para);
}

void Z3DRendererBase::addParameter(ZParameter *para)
{
  m_parameters.push_back(para);
}

std::vector<ZParameter*> Z3DRendererBase::getParameters() const
{
  std::vector<ZParameter*> result;
  std::map<Z3DPrimitiveRenderer*,bool>::const_iterator renderersIt;
  for (renderersIt = m_renderers.begin(); renderersIt != m_renderers.end();
       ++renderersIt) {
    std::vector<ZParameter*> others = renderersIt->first->getParameters();
    if (!others.empty())
      result.insert(result.end(), others.begin(), others.end());
  }
  result.insert(result.end(), m_parameters.begin(), m_parameters.end());
  return result;
}

std::vector<ZParameter*> Z3DRendererBase::getRendererParameters(Z3DPrimitiveRenderer *renderer) const
{
  return renderer->getParameters();
}

void Z3DRendererBase::compile()
{
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    m_renderersIt->first->compile();
  }
}

void Z3DRendererBase::render(Z3DEye eye)
{
  if (m_renderMethod.isSelected("Old openGL")) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getProjectionMatrix(eye)));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getViewMatrix(eye)));

    if (!useDisplayList()) {
      renderInstant();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      return;
    }

    // check if render state changed and we need to regenerate
    // display list
    if (m_displayList != 0 &&
        m_lastOpenglRenderingState != m_renderers) {
      invalidateDisplayList();
    }

    if (m_displayList == 0) {
      generateDisplayList();
    }

    if (glIsList(m_displayList)) {
      glCallList(m_displayList);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  } else {
    renderUsingGLSL(eye);
  }
}

void Z3DRendererBase::renderPicking(Z3DEye eye)
{
  if (m_renderMethod.isSelected("Old openGL")) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getProjectionMatrix(eye)));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getViewMatrix(eye)));

    if (!useDisplayList()) {
      renderPickingInstant();
      return;
    }

    // check if render state changed and we need to regenerate
    // display list
    if (m_pickingDisplayList != 0 &&
        m_lastOpenglPickingRenderingState != m_renderers) {
      invalidatePickingDisplayList();
    }

    if (m_pickingDisplayList == 0) {
      generatePickingDisplayList();
    }

    // render display list
    if (glIsList(m_pickingDisplayList)) {
      glCallList(m_pickingDisplayList);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  } else {
    renderPickingUsingGLSL(eye);
  }
}

void Z3DRendererBase::generateDisplayList()
{
  if (glIsList(m_displayList))
    glDeleteLists(m_displayList, 1);

  m_displayList = glGenLists(1);
  glNewList(m_displayList, GL_COMPILE);
  renderInstant();
  glEndList();
  m_lastOpenglRenderingState = m_renderers;
}

void Z3DRendererBase::generatePickingDisplayList()
{
  if (glIsList(m_pickingDisplayList))
    glDeleteLists(m_pickingDisplayList, 1);

  m_pickingDisplayList = glGenLists(1);
  glNewList(m_pickingDisplayList, GL_COMPILE);
  renderPickingInstant();
  glEndList();
  m_lastOpenglPickingRenderingState = m_renderers;
}

void Z3DRendererBase::adjustWidgets()
{
}

void Z3DRendererBase::renderInstant()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  if (needLighting()) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(m_lightAmbients[0]->get()));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(m_lightDiffuses[0]->get()));
    glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(m_lightSpeculars[0]->get()));
    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(m_lightPositions[0]->get()));
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, glm::value_ptr(m_lightSpotDirection[0]->get()));
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, m_lightSpotExponent[0]->get());
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, m_lightSpotCutoff[0]->get());
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, m_lightAttenuations[0]->get().x);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, m_lightAttenuations[0]->get().y);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, m_lightAttenuations[0]->get().z);

    if (m_lightCount.get() > 1) {
      glEnable(GL_LIGHT1);
      glLightfv(GL_LIGHT1, GL_AMBIENT, glm::value_ptr(m_lightAmbients[1]->get()));
      glLightfv(GL_LIGHT1, GL_DIFFUSE, glm::value_ptr(m_lightDiffuses[1]->get()));
      glLightfv(GL_LIGHT1, GL_SPECULAR, glm::value_ptr(m_lightSpeculars[1]->get()));
      glLightfv(GL_LIGHT1, GL_POSITION, glm::value_ptr(m_lightPositions[1]->get()));
      glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, glm::value_ptr(m_lightSpotDirection[1]->get()));
      glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, m_lightSpotExponent[1]->get());
      glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, m_lightSpotCutoff[1]->get());
      glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, m_lightAttenuations[1]->get().x);
      glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, m_lightAttenuations[1]->get().y);
      glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, m_lightAttenuations[1]->get().z);
    }

    if (m_lightCount.get() > 2) {
      glEnable(GL_LIGHT2);
      glLightfv(GL_LIGHT2, GL_AMBIENT, glm::value_ptr(m_lightAmbients[2]->get()));
      glLightfv(GL_LIGHT2, GL_DIFFUSE, glm::value_ptr(m_lightDiffuses[2]->get()));
      glLightfv(GL_LIGHT2, GL_SPECULAR, glm::value_ptr(m_lightSpeculars[2]->get()));
      glLightfv(GL_LIGHT2, GL_POSITION, glm::value_ptr(m_lightPositions[2]->get()));
      glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, glm::value_ptr(m_lightSpotDirection[2]->get()));
      glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, m_lightSpotExponent[2]->get());
      glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, m_lightSpotCutoff[2]->get());
      glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, m_lightAttenuations[2]->get().x);
      glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, m_lightAttenuations[2]->get().y);
      glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, m_lightAttenuations[2]->get().z);
    }

    if (m_lightCount.get() > 3) {
      glEnable(GL_LIGHT3);
      glLightfv(GL_LIGHT3, GL_AMBIENT, glm::value_ptr(m_lightAmbients[3]->get()));
      glLightfv(GL_LIGHT3, GL_DIFFUSE, glm::value_ptr(m_lightDiffuses[3]->get()));
      glLightfv(GL_LIGHT3, GL_SPECULAR, glm::value_ptr(m_lightSpeculars[3]->get()));
      glLightfv(GL_LIGHT3, GL_POSITION, glm::value_ptr(m_lightPositions[3]->get()));
      glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, glm::value_ptr(m_lightSpotDirection[3]->get()));
      glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, m_lightSpotExponent[3]->get());
      glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, m_lightSpotCutoff[3]->get());
      glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, m_lightAttenuations[3]->get().x);
      glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, m_lightAttenuations[3]->get().y);
      glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, m_lightAttenuations[3]->get().z);
    }

    if (m_lightCount.get() > 4) {
      glEnable(GL_LIGHT4);
      glLightfv(GL_LIGHT4, GL_AMBIENT, glm::value_ptr(m_lightAmbients[4]->get()));
      glLightfv(GL_LIGHT4, GL_DIFFUSE, glm::value_ptr(m_lightDiffuses[4]->get()));
      glLightfv(GL_LIGHT4, GL_SPECULAR, glm::value_ptr(m_lightSpeculars[4]->get()));
      glLightfv(GL_LIGHT4, GL_POSITION, glm::value_ptr(m_lightPositions[4]->get()));
      glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, glm::value_ptr(m_lightSpotDirection[4]->get()));
      glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, m_lightSpotExponent[4]->get());
      glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, m_lightSpotCutoff[4]->get());
      glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, m_lightAttenuations[4]->get().x);
      glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, m_lightAttenuations[4]->get().y);
      glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, m_lightAttenuations[4]->get().z);
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(m_materialAmbient.get()));
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, std::min(m_materialShininess.get(), 128.f));
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(m_materialSpecular.get()));
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glPopMatrix();
  }

  activateClipPlanesOpenGL();
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    if (m_renderersIt->second) {
      m_renderersIt->first->renderUsingOpengl();
    }
  }
  deactivateClipPlanesOpenGL();

  glPopAttrib();
}

void Z3DRendererBase::renderPickingInstant()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  activateClipPlanesOpenGL();
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    if (m_renderersIt->second) {
      m_renderersIt->first->renderPickingUsingOpengl();
    }
  }
  deactivateClipPlanesOpenGL();

  glPopAttrib();
}

void Z3DRendererBase::invalidateDisplayList()
{
  if (glIsList(m_displayList)) {
    glDeleteLists(m_displayList, 1);
  }
  m_displayList = 0;
}

void Z3DRendererBase::invalidatePickingDisplayList()
{
  if (glIsList(m_pickingDisplayList)) {
    glDeleteLists(m_pickingDisplayList, 1);
  }
  m_pickingDisplayList = 0;
}

void Z3DRendererBase::renderUsingGLSL(Z3DEye eye)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  activateClipPlanesGLSL();
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    if (m_renderersIt->second) {
      m_renderersIt->first->renderUsingGLSL(eye);
    }
  }
  deactivateClipPlanesGLSL();

  glPopAttrib();
}

void Z3DRendererBase::renderPickingUsingGLSL(Z3DEye eye)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  activateClipPlanesGLSL();
  for (m_renderersIt = m_renderers.begin(); m_renderersIt != m_renderers.end();
       ++m_renderersIt) {
    if (m_renderersIt->second) {
      m_renderersIt->first->renderPickingUsingGLSL(eye);
    }
  }
  deactivateClipPlanesGLSL();

  glPopAttrib();
}

bool Z3DRendererBase::needLighting() const
{
  bool needLighting = false;
  std::map<Z3DPrimitiveRenderer*,bool>::const_iterator renderersIt;
  for (renderersIt = m_renderers.begin(); renderersIt != m_renderers.end();
       ++renderersIt) {
    if (renderersIt->second) {
      needLighting = needLighting || renderersIt->first->getNeedLighting();
    }
  }
  return needLighting;
}

bool Z3DRendererBase::useDisplayList() const
{
  bool useDisplayList = false;
  std::map<Z3DPrimitiveRenderer*,bool>::const_iterator renderersIt;
  for (renderersIt = m_renderers.begin(); renderersIt != m_renderers.end();
       ++renderersIt) {
    if (renderersIt->second) {
      useDisplayList = useDisplayList || renderersIt->first->getUseDisplayList();
    }
  }
  return useDisplayList;
}

void Z3DRendererBase::activateClipPlanesOpenGL()
{
  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    glClipPlane(GL_CLIP_PLANE0+i, glm::value_ptr(m_clipPlanes[i]));
    glEnable(GL_CLIP_PLANE0+i);
    CHECK_GL_ERROR;
  }
}

void Z3DRendererBase::deactivateClipPlanesOpenGL()
{
  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    glDisable(GL_CLIP_PLANE0+i);
    CHECK_GL_ERROR;
  }
}

void Z3DRendererBase::activateClipPlanesGLSL()
{
  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    if (GLEW_VERSION_3_0) {
      glEnable(GL_CLIP_DISTANCE0+i);
    } else {
      glClipPlane(GL_CLIP_PLANE0+i, glm::value_ptr(m_clipPlanes[i]));
      glEnable(GL_CLIP_PLANE0+i);
    }
    CHECK_GL_ERROR;
  }
}

void Z3DRendererBase::deactivateClipPlanesGLSL()
{
  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    if (GLEW_VERSION_3_0) {
      glDisable(GL_CLIP_DISTANCE0+i);
    } else {
      glDisable(GL_CLIP_PLANE0+i);
    }
    CHECK_GL_ERROR;
  }
}

