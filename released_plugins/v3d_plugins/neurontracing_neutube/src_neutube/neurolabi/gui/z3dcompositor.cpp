#include "zglew.h"
#include "z3dcompositor.h"

#include "z3dtexture.h"
#include "z3dgpuinfo.h"
#include "z3dtextureblendrenderer.h"
#include "z3dtexturecopyrenderer.h"
#include "z3drendertarget.h"
#include "z3dinteractionhandler.h"

#include "zbenchtimer.h"

Z3DCompositor::Z3DCompositor()
  : Z3DRenderProcessor()
  , m_alphaBlendRenderer(NULL)
  , m_firstOnTopBlendRenderer(NULL)
  , m_textureCopyRenderer(NULL)
  , m_backgroundRenderer(NULL)
  , m_showBackground("Show Background", true)
  , m_renderGeometries("Render Geometries", true)
  , m_camera("Camera", Z3DCamera())
  , m_geometriesMultisampleMode("Multisample Anti-Aliasing")
  , m_transparencyMethod("Transparency")
  , m_inport("Image", false, InvalidMonoViewResult)
  , m_leftEyeInport("LeftEyeImage", false, InvalidLeftEyeResult)
  , m_rightEyeInport("RightEyeImage", false, InvalidRightEyeResult)
  , m_outport("Image", true, InvalidMonoViewResult)
  , m_leftEyeOutport("LeftEyeImage", true, InvalidLeftEyeResult)
  , m_rightEyeOutport("RightEyeImage", true, InvalidRightEyeResult)
  , m_tempPort("ImageTemp")
  , m_tempPort2("ImageTemp2")
  , m_tempPort3("ImageTemp3")
  , m_tempPort4("ImageTemp4")
  , m_pickingPort("PickingTarget", true, Z3DProcessor::InvalidAllResult, GL_RGBA8)
  , m_pPort("GeometryFilters", true)
  , m_ddpRT(NULL)
  , m_ddpBlendShader(NULL)
  , m_ddpFinalShader(NULL)
  , m_waRT(NULL)
  , m_waFinalShader(NULL)
  , m_queryId(0)
  , m_backgroundWidgetsGroup(NULL)
{
  addParameter(m_showBackground);
  addParameter(m_renderGeometries);
  addParameter(m_camera);

  m_interactionHandler = new Z3DTrackballInteractionHandler("Interaction Handler", &m_camera);
  addInteractionHandler(m_interactionHandler);

  m_geometriesMultisampleMode.addOptions("None", "2x2");
  m_geometriesMultisampleMode.select("2x2");
  addParameter(m_geometriesMultisampleMode);

  m_transparencyMethod.addOption("Blend Delayed");
  m_transparencyMethod.addOption("Blend No Depth Mask");
  m_transparencyMethod.select("Blend Delayed");

  if (Z3DGpuInfoInstance.isWeightedAverageSupported())
    m_transparencyMethod.addOption("Weighted Average");

  if (Z3DGpuInfoInstance.isDualDepthPeelingSupported())
    m_transparencyMethod.addOption("Dual Depth Peeling");

  //  if (Z3DGpuInfoInstance.isLinkedListSupported())
  //    m_transparencyMethod.addOption("Linked List");

  addParameter(m_transparencyMethod);

  addPort(m_inport);
  addPort(m_leftEyeInport);
  addPort(m_rightEyeInport);
  addPort(m_outport);
  addPort(m_leftEyeOutport);
  addPort(m_rightEyeOutport);
  addPrivateRenderPort(m_tempPort);
  addPrivateRenderPort(m_tempPort2);
  addPrivateRenderPort(m_tempPort3);
  addPrivateRenderPort(m_tempPort4);
  addPrivateRenderPort(m_pickingPort);
  addPort(m_pPort);
}

Z3DCompositor::~Z3DCompositor()
{
  delete m_interactionHandler;
}

//#define USE_RECT_TEX

void Z3DCompositor::initialize()
{
  Z3DRenderProcessor::initialize();

  m_pickingManager.setRenderTarget(m_pickingPort.getRenderTarget());

  m_alphaBlendRenderer = new Z3DTextureBlendRenderer("DepthTestBlending");
  m_rendererBase->addRenderer(m_alphaBlendRenderer);
  m_firstOnTopBlendRenderer = new Z3DTextureBlendRenderer("FirstOnTopBlending");
  m_rendererBase->addRenderer(m_firstOnTopBlendRenderer);
  m_textureCopyRenderer = new Z3DTextureCopyRenderer();
  m_rendererBase->addRenderer(m_textureCopyRenderer);
  m_textureCopyRenderer->setDiscardTransparent(true);
  m_backgroundRenderer = new Z3DBackgroundRenderer();
  m_rendererBase->addRenderer(m_backgroundRenderer);
  std::vector<ZParameter*> paras = m_rendererBase->getRendererParameters(m_backgroundRenderer);
  for (size_t i=0; i<paras.size(); i++) {
    //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
    addParameter(paras[i]);
  }

  if (Z3DGpuInfoInstance.isDualDepthPeelingSupported()) {
    m_ddpBlendShader = new Z3DShaderProgram();
    m_ddpFinalShader = new Z3DShaderProgram();
#ifdef USE_RECT_TEX
    m_ddpBlendShader->loadFromSourceFile("pass.vert", "dual_peeling_blend.frag",
                                         m_rendererBase->generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_ddpBlendShader->loadFromSourceFile("pass.vert", "dual_peeling_blend.frag", m_rendererBase->generateHeader());
#endif
    m_ddpBlendShader->bindFragDataLocation(0, "FragData0");
#ifdef USE_RECT_TEX
    m_ddpFinalShader->loadFromSourceFile("pass.vert", "dual_peeling_final.frag",
                                         m_rendererBase->generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_ddpFinalShader->loadFromSourceFile("pass.vert", "dual_peeling_final.frag", m_rendererBase->generateHeader());
#endif
    m_ddpFinalShader->bindFragDataLocation(0, "FragData0");
    glGenQueries(1, &m_queryId);
  }

  if (Z3DGpuInfoInstance.isWeightedAverageSupported()) {
    m_waFinalShader = new Z3DShaderProgram();
#ifdef USE_RECT_TEX
    m_waFinalShader->loadFromSourceFile("pass.vert", "wavg_final.frag",
                                        m_rendererBase->generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_waFinalShader->loadFromSourceFile("pass.vert", "wavg_final.frag", m_rendererBase->generateHeader());
#endif
    m_waFinalShader->bindFragDataLocation(0, "FragData0");
  }

  CHECK_GL_ERROR;
}

void Z3DCompositor::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getRendererParameters(m_backgroundRenderer);
  for (size_t i=0; i<paras.size(); i++) {
    //paras[i]->disconnect(this);
    removeParameter(paras[i]);
  }

  if (Z3DGpuInfoInstance.isDualDepthPeelingSupported()) {
    if (m_ddpRT) {
      delete m_ddpRT;
      m_ddpRT = NULL;
    }
    m_ddpBlendShader->removeAllShaders();
    m_ddpFinalShader->removeAllShaders();
    delete m_ddpBlendShader;
    m_ddpBlendShader = NULL;
    delete m_ddpFinalShader;
    m_ddpFinalShader = NULL;
    glDeleteQueries(1, &m_queryId);
  }

  if (Z3DGpuInfoInstance.isWeightedAverageSupported()) {
    if (m_waRT) {
      delete m_waRT;
      m_waRT = NULL;
    }
    m_waFinalShader->removeAllShaders();
    delete m_waFinalShader;
    m_waFinalShader = NULL;
  }

  Z3DRenderProcessor::deinitialize();
}

bool Z3DCompositor::isReady(Z3DEye eye) const
{
  if (eye == CenterEye && m_outport.isReady())
    return true;
  else if (eye == LeftEye && m_leftEyeOutport.isReady())
    return true;
  else if (eye == RightEye && m_rightEyeOutport.isReady())
    return true;
  return false;
}

ZWidgetsGroup *Z3DCompositor::getBackgroundWidgetsGroup()
{
  if (!m_backgroundWidgetsGroup) {
    m_backgroundWidgetsGroup = new ZWidgetsGroup("Background", NULL, 1);
    new ZWidgetsGroup(&m_showBackground, m_backgroundWidgetsGroup, 1);
    std::vector<ZParameter*> paras = m_rendererBase->getRendererParameters(m_backgroundRenderer);
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      new ZWidgetsGroup(para, m_backgroundWidgetsGroup, 1);
    }
    paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); ++i) {
      ZParameter *para = paras[i];
      if (para->getName() == "Rendering Method") {
        new ZWidgetsGroup(para, m_backgroundWidgetsGroup, 1);
        break;
      }
    }

    m_backgroundWidgetsGroup->setBasicAdvancedCutoff(4);
  }
  return m_backgroundWidgetsGroup;
}

void Z3DCompositor::savePickingBufferToImage(const QString &filename)
{
  Z3DTexture *tex = m_pickingManager.getRenderTarget()->getAttachment(GL_COLOR_ATTACHMENT0);
  saveTextureAsImage(tex, filename);
}

void Z3DCompositor::process(Z3DEye eye)
{
  std::vector<Z3DGeometryFilter*> filters = m_pPort.getAllConnectedProcessors();
  std::vector<Z3DGeometryFilter*> onTopFilters;
  std::vector<Z3DGeometryFilter*> normalFilters;
  if (m_renderGeometries.get()) {
    for (size_t i=0; i<filters.size(); ++i) {
      Z3DGeometryFilter* geomFilter = filters.at(i);
      if (geomFilter->isReady(eye) && geomFilter->getOpacity() > 0.0) {
        if (geomFilter->isStayOnTop())
          onTopFilters.push_back(geomFilter);
        else
          normalFilters.push_back(geomFilter);
      }
    }
  }

  Z3DRenderOutputPort &currentOutport = (eye == CenterEye) ?
        m_outport : (eye == LeftEye) ? m_leftEyeOutport : m_rightEyeOutport;
  Z3DRenderInputPort &currentInport = (eye == CenterEye) ?
        m_inport : (eye == LeftEye) ? m_leftEyeInport : m_rightEyeInport;

  if (!currentInport.isReady()) {  // no volume, only geometrys to render
    if (normalFilters.empty() || onTopFilters.empty()) {
      if (m_geometriesMultisampleMode.isSelected("2x2")) { // render to tempport (twice larger than outport) then copy to outport
        m_tempPort.resize(currentOutport.getSize() * 2);
      } else {  // render to tempport then copy to outport
        m_tempPort.resize(currentOutport.getSize());
      }

      if (onTopFilters.empty())
        renderGeometries(normalFilters, m_tempPort, eye);
      else
        renderGeometries(onTopFilters, m_tempPort, eye);

      // copy to outport
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase->setViewport(currentOutport.getSize());

      if (m_showBackground.get()) {
        m_rendererBase->activateRenderer(m_backgroundRenderer);
        m_rendererBase->render(eye);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      }
      m_textureCopyRenderer->setColorTexture(m_tempPort.getColorTexture());
      m_textureCopyRenderer->setDepthTexture(m_tempPort.getDepthTexture());
      m_rendererBase->activateRenderer(m_textureCopyRenderer);
      m_rendererBase->render(eye);
      if (m_showBackground.get()) {
        glBlendFunc(GL_ONE,GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
      CHECK_GL_ERROR;
    } else {
      if (m_geometriesMultisampleMode.isSelected("2x2")) {
        m_tempPort.resize(currentOutport.getSize() * 2);
        m_tempPort2.resize(currentOutport.getSize() * 2);
      } else {
        m_tempPort.resize(currentOutport.getSize());
        m_tempPort2.resize(currentOutport.getSize());
      }

      // render normal geometries to tempport
      renderGeometries(normalFilters, m_tempPort, eye);

      // render on top geometries to tempport2
      renderGeometries(onTopFilters, m_tempPort2, eye);

      // blend to output
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase->setViewport(currentOutport.getSize());

      if (m_showBackground.get()) {
        m_rendererBase->activateRenderer(m_backgroundRenderer);
        m_rendererBase->render(eye);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      }
      m_firstOnTopBlendRenderer->setColorTexture1(m_tempPort2.getColorTexture());
      m_firstOnTopBlendRenderer->setDepthTexture1(m_tempPort2.getDepthTexture());
      m_firstOnTopBlendRenderer->setColorTexture2(m_tempPort.getColorTexture());
      m_firstOnTopBlendRenderer->setDepthTexture2(m_tempPort.getDepthTexture());
      m_rendererBase->activateRenderer(m_firstOnTopBlendRenderer);
      m_rendererBase->render(eye);
      if (m_showBackground.get()) {
        glBlendFunc(GL_ONE,GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
      CHECK_GL_ERROR;
    }
  } else {      // with volume
    if (normalFilters.empty() && onTopFilters.empty()) {  // directly copy inport image to outport
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase->setViewport(currentOutport.getSize());

      if (m_showBackground.get()) {
        m_rendererBase->activateRenderer(m_backgroundRenderer);
        m_rendererBase->render(eye);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      }
      m_textureCopyRenderer->setColorTexture(currentInport.getColorTexture());
      m_textureCopyRenderer->setDepthTexture(currentInport.getDepthTexture());
      m_rendererBase->activateRenderer(m_textureCopyRenderer);
      m_rendererBase->render(eye);
      if (m_showBackground.get()) {
        glBlendFunc(GL_ONE,GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
      CHECK_GL_ERROR;
    } else if (normalFilters.empty() || onTopFilters.empty()) {  // render geometries into one temp port then blend with volume
      if (m_geometriesMultisampleMode.isSelected("2x2")) {
        m_tempPort.resize(currentOutport.getSize() * 2);
      } else {
        m_tempPort.resize(currentOutport.getSize());
      }

      // render geometries into one temp port
      if (onTopFilters.empty())
        renderGeometries(normalFilters, m_tempPort, eye);
      else
        renderGeometries(onTopFilters, m_tempPort, eye);

      // blend tempPort with volume
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase->setViewport(currentOutport.getSize());

      if (m_showBackground.get()) {
        m_rendererBase->activateRenderer(m_backgroundRenderer);
        m_rendererBase->render(eye);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      }
      if (onTopFilters.empty()) {
        m_alphaBlendRenderer->setColorTexture1(m_tempPort.getColorTexture());
        m_alphaBlendRenderer->setDepthTexture1(m_tempPort.getDepthTexture());
        m_alphaBlendRenderer->setColorTexture2(currentInport.getColorTexture());
        m_alphaBlendRenderer->setDepthTexture2(currentInport.getDepthTexture());
        m_rendererBase->activateRenderer(m_alphaBlendRenderer);
        m_rendererBase->render(eye);
      } else {
        m_firstOnTopBlendRenderer->setColorTexture1(m_tempPort.getColorTexture());
        m_firstOnTopBlendRenderer->setDepthTexture1(m_tempPort.getDepthTexture());
        m_firstOnTopBlendRenderer->setColorTexture2(currentInport.getColorTexture());
        m_firstOnTopBlendRenderer->setDepthTexture2(currentInport.getDepthTexture());
        m_rendererBase->activateRenderer(m_firstOnTopBlendRenderer);
        m_rendererBase->render(eye);
      }
      if (m_showBackground.get()) {
        glBlendFunc(GL_ONE,GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
      CHECK_GL_ERROR;
    } else { // render normal geometries into tempport, then blend inport and tempport into tempport2, then render on top geometries into tempport, then
      // blend temport and temport2 into outport
      if (m_geometriesMultisampleMode.isSelected("2x2")) {
        m_tempPort.resize(currentOutport.getSize() * 2);
        m_tempPort2.resize(currentOutport.getSize() * 2);
      } else {
        m_tempPort.resize(currentOutport.getSize());
        m_tempPort2.resize(currentOutport.getSize());
      }

      // render normal geometries into tempport
      renderGeometries(normalFilters, m_tempPort, eye);

      // blend inport and tempport into tempport2
      m_tempPort2.bindTarget();
      CHECK_GL_ERROR;

      m_rendererBase->setViewport(m_tempPort2.getSize());
      m_alphaBlendRenderer->setColorTexture1(m_tempPort.getColorTexture());
      m_alphaBlendRenderer->setDepthTexture1(m_tempPort.getDepthTexture());
      m_alphaBlendRenderer->setColorTexture2(currentInport.getColorTexture());
      m_alphaBlendRenderer->setDepthTexture2(currentInport.getDepthTexture());
      m_rendererBase->activateRenderer(m_alphaBlendRenderer);
      m_rendererBase->render(eye);

      m_tempPort2.releaseTarget();
      CHECK_GL_ERROR;

      // render on top geometries into tempport
      renderGeometries(onTopFilters, m_tempPort, eye);

      // blend temport and temport2 into outport
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase->setViewport(currentOutport.getSize());

      if (m_showBackground.get()) {
        m_rendererBase->activateRenderer(m_backgroundRenderer);
        m_rendererBase->render(eye);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      }
      m_firstOnTopBlendRenderer->setColorTexture1(m_tempPort.getColorTexture());
      m_firstOnTopBlendRenderer->setDepthTexture1(m_tempPort.getDepthTexture());
      m_firstOnTopBlendRenderer->setColorTexture2(m_tempPort2.getColorTexture());
      m_firstOnTopBlendRenderer->setDepthTexture2(m_tempPort2.getDepthTexture());
      m_rendererBase->activateRenderer(m_firstOnTopBlendRenderer);
      m_rendererBase->render(eye);
      if (m_showBackground.get()) {
        glBlendFunc(GL_ONE,GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
      CHECK_GL_ERROR;
    }
  }

  // render picking objects
  if (m_pickingManager.hasRenderTarget() && m_renderGeometries.get() && !filters.empty()) {
    glEnable(GL_DEPTH_TEST);
    m_pickingManager.bindTarget();
    m_pickingManager.clearTarget();
    for (size_t i=0; i<filters.size(); i++) {
      Z3DGeometryFilter* geomFilter = filters.at(i);
      if (geomFilter->isReady(eye)) {
        geomFilter->setPickingManager(&m_pickingManager);
        geomFilter->renderPicking(eye);
        CHECK_GL_ERROR;
      }
    }
    m_pickingManager.releaseTarget();
    glDisable(GL_DEPTH_TEST);
  }

  CHECK_GL_ERROR;
}

void Z3DCompositor::renderGeometries(const std::vector<Z3DGeometryFilter*> &filters,
                                     Z3DRenderOutputPort &port, Z3DEye eye)
{
  glEnable(GL_DEPTH_TEST);
  if (m_transparencyMethod.isSelected("Blend No Depth Mask"))
    renderGeomsBlendNoDepthMask(filters, port, eye);
  else if (m_transparencyMethod.isSelected("Blend Delayed"))
    renderGeomsBlendDelayed(filters, port, eye);
  else
    renderGeomsOIT(filters, port, eye, m_transparencyMethod.get());
  glDisable(GL_DEPTH_TEST);
}

void Z3DCompositor::renderGeomsBlendDelayed(const std::vector<Z3DGeometryFilter *> &filters,
                                            Z3DRenderOutputPort &port, Z3DEye eye)
{
  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  CHECK_GL_ERROR;

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    if(!geomFilter->needBlending()) {
      geomFilter->setCamera(m_camera.get());
      geomFilter->setViewport(port.getSize());
      geomFilter->render(eye);
      CHECK_GL_ERROR;
    }
  }

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    if(geomFilter->needBlending()) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      geomFilter->setCamera(m_camera.get());
      geomFilter->setViewport(port.getSize());
      geomFilter->render(eye);
      glBlendFunc(GL_ONE,GL_ZERO);
      glDisable(GL_BLEND);
      CHECK_GL_ERROR;
    }
  }

  port.releaseTarget();
  CHECK_GL_ERROR;
}

void Z3DCompositor::renderGeomsBlendNoDepthMask(const std::vector<Z3DGeometryFilter *> &filters,
                                                Z3DRenderOutputPort &port, Z3DEye eye)
{
  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  CHECK_GL_ERROR;

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    if(!geomFilter->needBlending()) {
      geomFilter->setCamera(m_camera.get());
      geomFilter->setViewport(port.getSize());
      geomFilter->render(eye);
      CHECK_GL_ERROR;
    }
  }

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    if(geomFilter->needBlending()) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);
      geomFilter->setCamera(m_camera.get());
      geomFilter->setViewport(port.getSize());
      geomFilter->render(eye);
      glBlendFunc(GL_ONE,GL_ZERO);
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
      CHECK_GL_ERROR;
    }
  }

  port.releaseTarget();
  CHECK_GL_ERROR;
}

void Z3DCompositor::renderGeomsOIT(const std::vector<Z3DGeometryFilter *> &filters,
                                   Z3DRenderOutputPort &port, Z3DEye eye, const QString &method)
{
  std::vector<Z3DGeometryFilter*> opaqueFilters;
  std::vector<Z3DGeometryFilter*> transparentFilters;
  for (size_t i=0; i<filters.size(); ++i) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    if (geomFilter->getRendererMethod() != "GLSL") {
      QString error = QString("Transparency method '%1' can only be used with 'GLSL' render mode").arg(method);
      LERROR() << error;
      port.bindTarget();
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      port.releaseTarget();
      return;
    }
    if (geomFilter->needBlending())
      transparentFilters.push_back(geomFilter);
    else
      opaqueFilters.push_back(geomFilter);
  }

  if (transparentFilters.empty()) {
    renderOpaque(opaqueFilters, port, eye);
  }
  //  else {
  //    if (method == "Dual Depth Peeling") {
  //      renderTransparentDDP(renderers, port, eye);
  //    }
  //  }
  else if (opaqueFilters.empty()) {
    if (method == "Dual Depth Peeling") {
      renderTransparentDDP(transparentFilters, port, eye);
    } else if (method == "Weighted Average") {
      renderTransparentWA(transparentFilters, port, eye);
    }
  } else {
    m_tempPort3.resize(port.getSize());
    renderOpaque(opaqueFilters, m_tempPort3, eye);

    m_tempPort4.resize(port.getSize());
    if (method == "Dual Depth Peeling") {
      renderTransparentDDP(transparentFilters, m_tempPort4, eye);
    } else if (method == "Weighted Average") {
      renderTransparentWA(transparentFilters, m_tempPort4, eye);
    }

    // blend temport3 and temport4 into outport
    port.bindTarget();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    m_rendererBase->setViewport(port.getSize());
    m_alphaBlendRenderer->setColorTexture1(m_tempPort3.getColorTexture());
    m_alphaBlendRenderer->setDepthTexture1(m_tempPort3.getDepthTexture());
    m_alphaBlendRenderer->setColorTexture2(m_tempPort4.getColorTexture());
    m_alphaBlendRenderer->setDepthTexture2(m_tempPort4.getDepthTexture());
    m_rendererBase->activateRenderer(m_alphaBlendRenderer);
    m_rendererBase->render(eye);
    port.releaseTarget();
  }
  CHECK_GL_ERROR;
}

void Z3DCompositor::renderOpaque(const std::vector<Z3DGeometryFilter *> &filters,
                                 Z3DRenderOutputPort &port, Z3DEye eye)
{
  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  CHECK_GL_ERROR;
  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    geomFilter->setCamera(m_camera.get());
    geomFilter->setViewport(port.getSize());
    geomFilter->render(eye);
    CHECK_GL_ERROR;
  }
  port.releaseTarget();
  CHECK_GL_ERROR;
}

void Z3DCompositor::renderTransparentDDP(const std::vector<Z3DGeometryFilter *> &filters,
                                         Z3DRenderOutputPort &port, Z3DEye eye)
{
  if (!m_ddpRT)
    if (!createDDPRenderTarget(port.getSize())) {
      LERROR() << "Can not create fbo for dual depth peeling rendering";
      return;
    }
  m_ddpRT->resize(port.getSize());

  Z3DTexture* g_dualDepthTexId[2];
  g_dualDepthTexId[0] = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT0);
  g_dualDepthTexId[1] = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT3);
  Z3DTexture* g_dualFrontBlenderTexId[2];
  g_dualFrontBlenderTexId[0] = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT1);
  g_dualFrontBlenderTexId[1] = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT4);
  Z3DTexture* g_dualBackTempTexId[2];
  g_dualBackTempTexId[0] = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT2);
  g_dualBackTempTexId[1] = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT5);
  Z3DTexture* g_dualBackBlenderTexId = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT6);
  Z3DTexture* g_depthTex = m_ddpRT->getAttachment(GL_COLOR_ATTACHMENT7);
  GLenum g_drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                            GL_COLOR_ATTACHMENT1,
                            GL_COLOR_ATTACHMENT2,
                            GL_COLOR_ATTACHMENT3,
                            GL_COLOR_ATTACHMENT4,
                            GL_COLOR_ATTACHMENT5,
                            GL_COLOR_ATTACHMENT6
                           };

  GLenum g_db[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT7};

  bool g_useOQ = true;
  int g_numPasses = 100;

#define MAX_DEPTH 1.0

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // ---------------------------------------------------------------------
  // 1. Initialize Min-Max Depth Buffer
  // ---------------------------------------------------------------------

  m_ddpRT->bind();

  // Render targets 1 and 2 store the front and back colors
  // Clear to 0.0 and use MAX blending to filter written color
  // At most one front color and one back color can be written every pass
  glDrawBuffers(2, &g_drawBuffers[1]);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Render target 0 stores (-minDepth, maxDepth, alphaMultiplier)
  glDrawBuffers(2, g_db);
  glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glBlendEquation(GL_MAX);

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    geomFilter->setCamera(m_camera.get());
    geomFilter->setViewport(m_ddpRT->getSize());
    geomFilter->setShaderHookType(Z3DRendererBase::DualDepthPeelingInit);
    geomFilter->render(eye);
    CHECK_GL_ERROR;
  }


  // ---------------------------------------------------------------------
  // 2. Dual Depth Peeling + Blending
  // ---------------------------------------------------------------------

  // Since we cannot blend the back colors in the geometry passes,
  // we use another render target to do the alpha blending
  glDrawBuffer(g_drawBuffers[6]);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  int currId = 0;

  for (int pass = 1; g_useOQ && pass < g_numPasses; pass++) {
    currId = pass % 2;
    int prevId = 1 - currId;
    int bufId = currId * 3;

    glDrawBuffers(2, &g_drawBuffers[bufId+1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawBuffer(g_drawBuffers[bufId+0]);
    glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render target 0: RG32F MAX blending
    // Render target 1: RGBA MAX blending
    // Render target 2: RGBA MAX blending
    glDrawBuffers(3, &g_drawBuffers[bufId+0]);
    glBlendEquation(GL_MAX);

    for (size_t i=0; i<filters.size(); i++) {
      Z3DGeometryFilter* geomFilter = filters.at(i);
      geomFilter->setCamera(m_camera.get());
      geomFilter->setViewport(m_ddpRT->getSize());
      geomFilter->setShaderHookType(Z3DRendererBase::DualDepthPeelingPeel);
      geomFilter->shaderHookPara().dualDepthPeelingDepthBlenderTexture = g_dualDepthTexId[prevId];
      geomFilter->shaderHookPara().dualDepthPeelingFrontBlenderTexture = g_dualFrontBlenderTexId[prevId];
      geomFilter->render(eye);
      CHECK_GL_ERROR;
    }

    // Full screen pass to alpha-blend the back color
    glDrawBuffer(g_drawBuffers[6]);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (g_useOQ) {
      glBeginQuery(GL_SAMPLES_PASSED, m_queryId);
    }

    m_ddpBlendShader->bind();
    m_ddpBlendShader->bindTexture("TempTex", g_dualBackTempTexId[currId]);
#if !defined(USE_RECT_TEX)
    m_rendererBase->setViewport(m_ddpRT->getSize());
    m_rendererBase->setGlobalShaderParameters(m_ddpBlendShader, eye);
#endif
    renderScreenQuad(*m_ddpBlendShader);
    m_ddpBlendShader->release();

    CHECK_GL_ERROR;

    if (g_useOQ) {
      glEndQuery(GL_SAMPLES_PASSED);
      GLuint sample_count;
      glGetQueryObjectuiv(m_queryId, GL_QUERY_RESULT, &sample_count);
      if (sample_count == 0) {
        break;
      }
    }
    CHECK_GL_ERROR;
  }

  m_ddpRT->release();

  glPopAttrib();

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    geomFilter->setShaderHookType(Z3DRendererBase::Normal);
  }
  CHECK_GL_ERROR;

  // ---------------------------------------------------------------------
  // 3. Final Pass
  // ---------------------------------------------------------------------

  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  m_ddpFinalShader->bind();
  m_ddpFinalShader->bindTexture("DepthTex", g_depthTex);
  m_ddpFinalShader->bindTexture("FrontBlenderTex", g_dualFrontBlenderTexId[currId]);
  m_ddpFinalShader->bindTexture("BackBlenderTex", g_dualBackBlenderTexId);
#if !defined(USE_RECT_TEX)
  m_rendererBase->setViewport(m_ddpRT->getSize());
  m_rendererBase->setGlobalShaderParameters(m_ddpFinalShader, eye);
#endif
  renderScreenQuad(*m_ddpFinalShader);
  m_ddpFinalShader->release();
  port.releaseTarget();

  CHECK_GL_ERROR;
}

bool Z3DCompositor::createDDPRenderTarget(glm::ivec2 size)
{
  m_ddpRT = new Z3DRenderTarget(size);
  Z3DTexture* g_dualDepthTexId[2];
  Z3DTexture* g_dualFrontBlenderTexId[2];
  Z3DTexture* g_dualBackTempTexId[2];
  Z3DTexture* g_dualBackBlenderTexId;
  Z3DTexture* g_depthTex;

#ifdef USE_RECT_TEX
  for (int i = 0; i < 2; i++)
  {
    g_dualDepthTexId[i] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                         GL_RG, GL_RG32F, GL_FLOAT,
                                         GL_NEAREST, GL_NEAREST, GL_CLAMP);
    g_dualDepthTexId[i]->uploadTexture();

    g_dualFrontBlenderTexId[i] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                                GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                                GL_NEAREST, GL_NEAREST, GL_CLAMP);
    g_dualFrontBlenderTexId[i]->uploadTexture();

    g_dualBackTempTexId[i] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                            GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                            GL_NEAREST, GL_NEAREST, GL_CLAMP);
    g_dualBackTempTexId[i]->uploadTexture();
  }

  g_dualBackBlenderTexId = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                          GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_dualBackBlenderTexId->uploadTexture();

  g_depthTex = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                              GL_RED, GL_R32F, GL_FLOAT,
                              GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_depthTex->uploadTexture();
#else
  for (int i = 0; i < 2; i++)
  {
    g_dualDepthTexId[i] = new Z3DTexture(glm::ivec3(size, 1),
                                         GL_RG, GL_RG32F, GL_FLOAT,
                                         GL_NEAREST, GL_NEAREST, GL_CLAMP);
    g_dualDepthTexId[i]->uploadTexture();

    g_dualFrontBlenderTexId[i] = new Z3DTexture(glm::ivec3(size, 1),
                                                GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                                GL_NEAREST, GL_NEAREST, GL_CLAMP);
    g_dualFrontBlenderTexId[i]->uploadTexture();

    g_dualBackTempTexId[i] = new Z3DTexture(glm::ivec3(size, 1),
                                            GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                            GL_NEAREST, GL_NEAREST, GL_CLAMP);
    g_dualBackTempTexId[i]->uploadTexture();
  }

  g_dualBackBlenderTexId = new Z3DTexture(glm::ivec3(size, 1),
                                          GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_dualBackBlenderTexId->uploadTexture();

  g_depthTex = new Z3DTexture(glm::ivec3(size, 1),
                              GL_RED, GL_R32F, GL_FLOAT,
                              GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_depthTex->uploadTexture();
#endif

  int j = 0;
  m_ddpRT->attachTextureToFBO(g_dualDepthTexId[j], GL_COLOR_ATTACHMENT0);
  m_ddpRT->attachTextureToFBO(g_dualFrontBlenderTexId[j], GL_COLOR_ATTACHMENT1);
  m_ddpRT->attachTextureToFBO(g_dualBackTempTexId[j], GL_COLOR_ATTACHMENT2);

  j = 1;
  m_ddpRT->attachTextureToFBO(g_dualDepthTexId[j], GL_COLOR_ATTACHMENT3);
  m_ddpRT->attachTextureToFBO(g_dualFrontBlenderTexId[j], GL_COLOR_ATTACHMENT4);
  m_ddpRT->attachTextureToFBO(g_dualBackTempTexId[j], GL_COLOR_ATTACHMENT5);

  m_ddpRT->attachTextureToFBO(g_dualBackBlenderTexId, GL_COLOR_ATTACHMENT6);
  m_ddpRT->attachTextureToFBO(g_depthTex, GL_COLOR_ATTACHMENT7);
  bool comp =  m_ddpRT->isFBOComplete();
  if (!comp) {
    delete m_ddpRT;
    m_ddpRT = NULL;
  }
  return comp;
}

void Z3DCompositor::renderTransparentWA(const std::vector<Z3DGeometryFilter *> &filters,
                                        Z3DRenderOutputPort &port, Z3DEye eye)
{
  if (!m_waRT)
    if (!createWARenderTarget(port.getSize())) {
      LERROR() << "Can not create fbo for weighted average rendering";
      return;
    }
  m_waRT->resize(port.getSize());

  Z3DTexture* g_accumulationTexId[2];
  g_accumulationTexId[0] = m_waRT->getAttachment(GL_COLOR_ATTACHMENT0);
  g_accumulationTexId[1] = m_waRT->getAttachment(GL_COLOR_ATTACHMENT1);
  GLenum g_drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                            GL_COLOR_ATTACHMENT1
                           };

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  // ---------------------------------------------------------------------
  // 1. Accumulate Colors and Depth Complexity
  // ---------------------------------------------------------------------

  m_waRT->bind();

  glDrawBuffers(2, g_drawBuffers);

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    geomFilter->setCamera(m_camera.get());
    geomFilter->setViewport(m_waRT->getSize());
    geomFilter->setShaderHookType(Z3DRendererBase::WeightedAverageInit);
    geomFilter->render(eye);
    CHECK_GL_ERROR;
  }

  m_waRT->release();

  glPopAttrib();
  for (size_t i=0; i<filters.size(); i++) {
    Z3DGeometryFilter* geomFilter = filters.at(i);
    geomFilter->setShaderHookType(Z3DRendererBase::Normal);
  }
  CHECK_GL_ERROR;

  // ---------------------------------------------------------------------
  // 2. Approximate Blending
  // ---------------------------------------------------------------------

  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  m_waFinalShader->bind();
  m_waFinalShader->bindTexture("ColorTex0", g_accumulationTexId[0]);
  m_waFinalShader->bindTexture("ColorTex1", g_accumulationTexId[1]);
#if !defined(USE_RECT_TEX)
  m_rendererBase->setViewport(m_waRT->getSize());
  m_rendererBase->setGlobalShaderParameters(m_waFinalShader, eye);
#endif
  renderScreenQuad(*m_waFinalShader);
  m_waFinalShader->release();
  port.releaseTarget();

  CHECK_GL_ERROR;
}

bool Z3DCompositor::createWARenderTarget(glm::ivec2 size)
{
  m_waRT = new Z3DRenderTarget(size);
  Z3DTexture* g_accumulationTexId[2];

#ifdef USE_RECT_TEX
  g_accumulationTexId[0] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                          GL_RGBA, GL_RGBA32F, GL_FLOAT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_accumulationTexId[0]->uploadTexture();
  g_accumulationTexId[1] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                          GL_RG, GL_RG32F, GL_FLOAT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_accumulationTexId[1]->uploadTexture();
#else
  g_accumulationTexId[0] = new Z3DTexture(glm::ivec3(size, 1),
                                          GL_RGBA, GL_RGBA32F, GL_FLOAT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_accumulationTexId[0]->uploadTexture();
  g_accumulationTexId[1] = new Z3DTexture(glm::ivec3(size, 1),
                                          GL_RG, GL_RG32F, GL_FLOAT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP);
  g_accumulationTexId[1]->uploadTexture();
#endif

  m_waRT->attachTextureToFBO(g_accumulationTexId[0], GL_COLOR_ATTACHMENT0);
  m_waRT->attachTextureToFBO(g_accumulationTexId[1], GL_COLOR_ATTACHMENT1);
  bool comp = m_waRT->isFBOComplete();
  if (!comp) {
    delete m_waRT;
    m_waRT = NULL;
  }
  return comp;
}
