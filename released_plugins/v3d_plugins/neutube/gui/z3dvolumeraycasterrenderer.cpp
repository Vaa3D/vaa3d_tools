#include "z3dvolumeraycasterrenderer.h"

#include "z3dtexture.h"
#include "z3dvolume.h"

Z3DVolumeRaycasterRenderer::Z3DVolumeRaycasterRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_samplingRate("Sampling Rate", 2.f, 0.01f, 20.f)
  , m_isoValue("ISO Value", 0.5f, 0.0f, 1.0f)
  , m_localMIPThreshold("Local MIP Threshold", 0.8f, 0.01f, 1.f)
  , m_gradientMode("Gradient Calculation")
  , m_compositingMode("Compositing")
  , m_channel1Visible("Show Channel 1", true)
  , m_channel2Visible("Show Channel 2", true)
  , m_channel3Visible("Show Channel 3", true)
  , m_channel4Visible("Show Channel 4", true)
  , m_channel5Visible("Show Channel 5", true)
  , m_transferFunc1("Transfer Function 1")
  , m_transferFunc2("Transfer Function 2")
  , m_transferFunc3("Transfer Function 3")
  , m_transferFunc4("Transfer Function 4")
  , m_transferFunc5("Transfer Function 5")
  , m_texFilterMode1("Texture Filtering 1")
  , m_texFilterMode2("Texture Filtering 2")
  , m_texFilterMode3("Texture Filtering 3")
  , m_texFilterMode4("Texture Filtering 4")
  , m_texFilterMode5("Texture Filtering 5")
  , m_nChannel(0)
  , m_is2DImage(false)
  , m_entryCoordTexture(NULL)
  , m_entryDepthTexture(NULL)
  , m_exitCoordTexture(NULL)
  , m_exitDepthTexture(NULL)
  , m_opaque(false)
  , m_alpha(1.0)
{
  addParameter(m_samplingRate);

  addParameter(m_channel1Visible);
  addParameter(m_channel2Visible);
  addParameter(m_channel3Visible);
  addParameter(m_channel4Visible);
  addParameter(m_channel5Visible);

  addParameter(m_transferFunc1);
  addParameter(m_transferFunc2);
  addParameter(m_transferFunc3);
  addParameter(m_transferFunc4);
  addParameter(m_transferFunc5);

  // compositing modes
  m_compositingMode.addOptions("Direct Volume Rendering", "Maximum Intensity Projection",
                               "MIP Opaque", "Local MIP", "Local MIP Opaque", "ISO Surface", "X Ray");
  m_compositingMode.select("MIP Opaque");
  addParameter(m_compositingMode);
  addParameter(m_isoValue);
  addParameter(m_localMIPThreshold);

  // volume texture filtering
  m_texFilterMode1.addOptionsWithData(qMakePair(QString("Nearest"), GL_NEAREST),
                                      qMakePair(QString("Linear"), GL_LINEAR));
  m_texFilterMode1.select("Linear");
  addParameter(m_texFilterMode1);
  m_texFilterMode2.addOptionsWithData(qMakePair(QString("Nearest"), GL_NEAREST),
                                      qMakePair(QString("Linear"), GL_LINEAR));
  m_texFilterMode2.select("Linear");
  addParameter(m_texFilterMode2);
  m_texFilterMode3.addOptionsWithData(qMakePair(QString("Nearest"), GL_NEAREST),
                                      qMakePair(QString("Linear"), GL_LINEAR));
  m_texFilterMode3.select("Linear");
  addParameter(m_texFilterMode3);
  m_texFilterMode4.addOptionsWithData(qMakePair(QString("Nearest"), GL_NEAREST),
                                      qMakePair(QString("Linear"), GL_LINEAR));
  m_texFilterMode4.select("Linear");
  addParameter(m_texFilterMode4);
  m_texFilterMode5.addOptionsWithData(qMakePair(QString("Nearest"), GL_NEAREST),
                                      qMakePair(QString("Linear"), GL_LINEAR));
  m_texFilterMode5.select("Linear");
  addParameter(m_texFilterMode5);

  connect(&m_compositingMode, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));
  connect(&m_compositingMode, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_channel1Visible, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_channel2Visible, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_channel3Visible, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_channel4Visible, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_channel5Visible, SIGNAL(valueChanged()), this, SLOT(compile()));

  m_volumes.resize(5,NULL);
  for (size_t i=0; i<m_volumes.size(); ++i) {
    m_volumeUniformNames.push_back(QString("volume_struct_%1").arg(i+1));
    m_transferFuncUniformNames.push_back(QString("transfer_function_%1").arg(i+1));
  }
  m_channelVisibleParas.push_back(&m_channel1Visible);
  m_channelVisibleParas.push_back(&m_channel2Visible);
  m_channelVisibleParas.push_back(&m_channel3Visible);
  m_channelVisibleParas.push_back(&m_channel4Visible);
  m_channelVisibleParas.push_back(&m_channel5Visible);
  m_transferFuncParas.push_back(&m_transferFunc1);
  m_transferFuncParas.push_back(&m_transferFunc2);
  m_transferFuncParas.push_back(&m_transferFunc3);
  m_transferFuncParas.push_back(&m_transferFunc4);
  m_transferFuncParas.push_back(&m_transferFunc5);
  m_texFilterModeParas.push_back(&m_texFilterMode1);
  m_texFilterModeParas.push_back(&m_texFilterMode2);
  m_texFilterModeParas.push_back(&m_texFilterMode3);
  m_texFilterModeParas.push_back(&m_texFilterMode4);
  m_texFilterModeParas.push_back(&m_texFilterMode5);

  adjustWidgets();
  updateChannel();
}

Z3DVolumeRaycasterRenderer::~Z3DVolumeRaycasterRenderer()
{
}

void Z3DVolumeRaycasterRenderer::setCompositeMode(const QString &option)
{
  m_compositingMode.select(option);
}

void Z3DVolumeRaycasterRenderer::setTextureFilterMode(const QString &option)
{
  m_texFilterMode1.select(option);
  m_texFilterMode2.select(option);
  m_texFilterMode3.select(option);
  m_texFilterMode4.select(option);
  m_texFilterMode5.select(option);
}

void Z3DVolumeRaycasterRenderer::setChannel1(Z3DVolume *vol)
{
  if (m_volumes[0] != vol) {
    m_volumes[0] = vol;
    m_transferFuncParas[0]->setVolume(m_volumes[0]);
    updateChannel();
  }
}

void Z3DVolumeRaycasterRenderer::setChannel2(Z3DVolume *vol)
{
  if (m_volumes[1] != vol) {
    m_volumes[1] = vol;
    m_transferFuncParas[1]->setVolume(m_volumes[1]);
    updateChannel();
  }
}

void Z3DVolumeRaycasterRenderer::setChannel3(Z3DVolume *vol)
{
  if (m_volumes[2] != vol) {
    m_volumes[2] = vol;
    m_transferFuncParas[2]->setVolume(m_volumes[2]);
    updateChannel();
  }
}

void Z3DVolumeRaycasterRenderer::setChannel4(Z3DVolume *vol)
{
  if (m_volumes[3] != vol) {
    m_volumes[3] = vol;
    m_transferFuncParas[3]->setVolume(m_volumes[3]);
    updateChannel();
  }
}

void Z3DVolumeRaycasterRenderer::setChannel5(Z3DVolume *vol)
{
  if (m_volumes[4] != vol) {
    m_volumes[4] = vol;
    m_transferFuncParas[4]->setVolume(m_volumes[4]);
    updateChannel();
  }
}

void Z3DVolumeRaycasterRenderer::setChannels(std::vector<Z3DVolume *> vols)
{
  vols.resize(m_volumes.size(), NULL);
  if (m_volumes != vols) {
    m_volumes = vols;
    for (size_t i=0; i<m_volumes.size(); ++i)
      m_transferFuncParas[i]->setVolume(m_volumes[i]);
    updateChannel();
  }
}

void Z3DVolumeRaycasterRenderer::addQuad(const Z3DTriangleList &quad)
{
  if (quad.empty() ||
      (quad.getVertices().size() != 4 && quad.getVertices().size() != 6) ||
      (quad.getVertices().size() != quad.get3DTextureCoordinates().size() &&
       quad.getVertices().size() != quad.get2DTextureCoordinates().size())) {
    LERROR() << "Input quad should be 2D slice with either 2D or 3D texture coordinates";
    return;
  }
  m_quads.push_back(quad);
  m_entryCoordTexture = NULL;
  m_entryDepthTexture = NULL;
  m_exitCoordTexture = NULL;
  m_exitDepthTexture = NULL;
}

void Z3DVolumeRaycasterRenderer::setEntryExitCoordTextures(const Z3DTexture *entryCoordTexture, const Z3DTexture *entryDepthTexture,
                                                           const Z3DTexture *exitCoordTexture, const Z3DTexture *exitDepthTexture)
{
  m_entryCoordTexture = entryCoordTexture;
  m_entryDepthTexture = entryDepthTexture;
  m_exitCoordTexture = exitCoordTexture;
  m_exitDepthTexture = exitDepthTexture;
  m_quads.clear();
}


void Z3DVolumeRaycasterRenderer::adjustWidgets()
{
  m_isoValue.setVisible(m_compositingMode.isSelected("ISO Surface"));
  m_localMIPThreshold.setVisible(m_compositingMode.isSelected("Local MIP") ||
                                 m_compositingMode.isSelected("Local MIP Opaque"));
}

void Z3DVolumeRaycasterRenderer::bindVolumesAndTransferFuncs(Z3DShaderProgram &shader)
{
  shader.setLogUniformLocationError(false);

  for (size_t i=0; i < m_nChannel; ++i) {
    Z3DVolume *volume = m_volumes[i];
    if (!volume || !m_channelVisibleParas[i]->get())
      continue;

    // volumes
    shader.bindVolume(m_volumeUniformNames[i], volume, m_texFilterModeParas[i]->getAssociatedData(),
                      m_texFilterModeParas[i]->getAssociatedData());

    // transfer functions
    shader.bindTexture(m_transferFuncUniformNames[i], m_transferFuncParas[i]->get().getTexture());

    CHECK_GL_ERROR;
  }

  shader.setLogUniformLocationError(true);
}

void Z3DVolumeRaycasterRenderer::compile()
{
  m_raycasterShader.setHeaderAndRebuild(generateHeader());
  m_2dImageShader.setHeaderAndRebuild(generateHeader());
  m_volumeSliceWithTransferfunShader.setHeaderAndRebuild(generateHeader());
}

void Z3DVolumeRaycasterRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  m_raycasterShader.loadFromSourceFile("pass.vert", "volume_raycaster.frag", generateHeader());
  m_raycasterShader.bindFragDataLocation(0, "FragData0");
  m_2dImageShader.loadFromSourceFile("transform_with_2dtexture.vert", "image2d_with_transfun.frag", generateHeader());
  m_2dImageShader.bindFragDataLocation(0, "FragData0");
  m_volumeSliceWithTransferfunShader.loadFromSourceFile("transform_with_3dtexture.vert", "volume_slice_with_transfun.frag", generateHeader());
  m_volumeSliceWithTransferfunShader.bindFragDataLocation(0, "FragData0");
}

void Z3DVolumeRaycasterRenderer::deinitialize()
{
  m_raycasterShader.removeAllShaders();
  m_2dImageShader.removeAllShaders();
  m_volumeSliceWithTransferfunShader.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DVolumeRaycasterRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();

  if (hasVisibleRendering()) {
    if(m_volumes[0] && m_channel1Visible.get()){
      headerSource += "#define VOLUME_1_ACTIVE\n";
    }
    if(m_volumes[1] && m_channel2Visible.get()){
      headerSource += "#define VOLUME_2_ACTIVE\n";
    }
    if(m_volumes[2] && m_channel3Visible.get()){
      headerSource += "#define VOLUME_3_ACTIVE\n";
    }
    if(m_volumes[3] && m_channel4Visible.get()){
      headerSource += "#define VOLUME_4_ACTIVE\n";
    }
    if(m_volumes[4] && m_channel5Visible.get()){
      headerSource += "#define VOLUME_5_ACTIVE\n";
    }
  } else {
    headerSource += "#define DISABLE_TEXTURE_COORD_OUTPUT\n";
  }

  headerSource += "#define TF_SAMPLER_TYPE_1 " + m_transferFunc1.get().getSamplerType() + "\n";
  headerSource += "#define TF_SAMPLER_TYPE_2 " + m_transferFunc2.get().getSamplerType() + "\n";
  headerSource += "#define TF_SAMPLER_TYPE_3 " + m_transferFunc3.get().getSamplerType() + "\n";
  headerSource += "#define TF_SAMPLER_TYPE_4 " + m_transferFunc4.get().getSamplerType() + "\n";
  headerSource += "#define TF_SAMPLER_TYPE_5 " + m_transferFunc5.get().getSamplerType() + "\n";

  if (m_compositingMode.isSelected("Direct Volume Rendering")) {
    headerSource += "#define COMPOSITING(result, color, currentRayLength, rayDepth) ";
    headerSource += "compositeDVR(result, color, currentRayLength, rayDepth);\n";
  } else if (m_compositingMode.isSelected("ISO Surface")) {
    headerSource += "#define ISO\n";
    headerSource += "#define COMPOSITING(result, color, currentRayLength, rayDepth) ";
    headerSource += "compositeISO(result, color, currentRayLength, rayDepth, iso_value);\n";
  } else if (m_compositingMode.isSelected("Maximum Intensity Projection")) {
    headerSource += "#define MIP\n";
  } else if (m_compositingMode.isSelected("Local MIP")) {
    headerSource += "#define MIP\n";
    headerSource += "#define LOCAL_MIP\n";
  } else if (m_compositingMode.isSelected("X Ray")) {
    headerSource += "#define COMPOSITING(result, color, currentRayLength, rayDepth) ";
    headerSource += "compositeXRay(result, color, currentRayLength, rayDepth);\n";
  } else if (m_compositingMode.isSelected("MIP Opaque")) {
    headerSource += "#define MIP\n";
    headerSource += "#define RESULT_OPAQUE\n";
  } else if (m_compositingMode.isSelected("Local MIP Opaque")) {
    headerSource += "#define MIP\n";
    headerSource += "#define LOCAL_MIP\n";
    headerSource += "#define RESULT_OPAQUE\n";
  }

  return headerSource;
}

void Z3DVolumeRaycasterRenderer::renderUsingOpengl()
{
  //renderUsingGLSL();
}

void Z3DVolumeRaycasterRenderer::renderPickingUsingOpengl()
{
}

void Z3DVolumeRaycasterRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (!hasVisibleRendering())
    return;

  if (m_quads.empty()) {
    if (m_entryCoordTexture == NULL || m_entryDepthTexture == NULL ||
        m_exitCoordTexture == NULL || m_exitDepthTexture == NULL)
      return;
  } else {
    for (size_t i=0; i<m_quads.size(); ++i) {
      if (m_is2DImage && m_quads[i].getVertices().size() != m_quads[i].get2DTextureCoordinates().size())
        return;
      if (!m_is2DImage && m_quads[i].getVertices().size() != m_quads[i].get3DTextureCoordinates().size())
        return;
    }
  }

  if (!m_quads.empty()) { // 2d image or slice from 3d volume
    if (m_is2DImage) {   // image is 2D
      m_2dImageShader.bind();
      m_rendererBase->setGlobalShaderParameters(m_2dImageShader, eye);

      bindVolumesAndTransferFuncs(m_2dImageShader);

      for (size_t i=0; i<m_quads.size(); ++i)
        renderTriangleList(m_2dImageShader, m_quads[i]);

      m_2dImageShader.release();
    } else {   // image is 3D, but a 2D slice will be shown
      m_volumeSliceWithTransferfunShader.bind();
      m_rendererBase->setGlobalShaderParameters(m_volumeSliceWithTransferfunShader, eye);

      bindVolumesAndTransferFuncs(m_volumeSliceWithTransferfunShader);

      for (size_t i=0; i<m_quads.size(); ++i)
        renderTriangleList(m_volumeSliceWithTransferfunShader, m_quads[i]);

      m_volumeSliceWithTransferfunShader.release();
    }
  } else {  // 3d volume raycasting
    m_raycasterShader.bind();

    m_rendererBase->setGlobalShaderParameters(m_raycasterShader, eye);

    bindVolumesAndTransferFuncs(m_raycasterShader);

    float n = getCamera().getNearDist();
    float f = getCamera().getFarDist();
    //http://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
    // zw = a/ze + b;  ze = a/(zw - b);  a = f*n/(f-n);  b = 0.5*(f+n)/(f-n) + 0.5;
    float a = f*n/(f-n);
    float b = 0.5f * (f+n)/(f-n) + 0.5f;
    m_raycasterShader.setUniformValue("ze_to_zw_b", b);
    m_raycasterShader.setUniformValue("ze_to_zw_a", a);

    // entry exit points
    m_raycasterShader.bindTexture("ray_entry_points", m_entryCoordTexture);
    m_raycasterShader.bindTexture("ray_entry_points_depth", m_entryDepthTexture);
    m_raycasterShader.bindTexture("ray_exit_points",  m_exitCoordTexture);
    m_raycasterShader.bindTexture("ray_exit_points_depth", m_exitDepthTexture);

    if (m_compositingMode.get() ==  "ISO Surface")
      m_raycasterShader.setUniformValue("iso_value", m_isoValue.get());

    if (m_compositingMode.get() ==  "Local MIP" || m_compositingMode.get() ==  "Local MIP Opaque")
      m_raycasterShader.setUniformValue("local_MIP_threshold", m_localMIPThreshold.get());

    m_raycasterShader.setUniformValue("sampling_rate", m_samplingRate.get());

    renderScreenQuad(m_raycasterShader, false);
    m_raycasterShader.release();
  }
  CHECK_GL_ERROR;
}

void Z3DVolumeRaycasterRenderer::renderPickingUsingGLSL(Z3DEye)
{
}

void Z3DVolumeRaycasterRenderer::updateChannel()
{
  size_t oldNumChannel = m_nChannel;
  if (m_volumes[4]) {
    m_nChannel = 5;
    m_is2DImage = (m_volumes[4]->is2DData());
  } else if (m_volumes[3]) {
    m_nChannel = 4;
    m_is2DImage = (m_volumes[3]->is2DData());
  } else if (m_volumes[2]) {
    m_nChannel = 3;
    m_is2DImage = (m_volumes[2]->is2DData());
  } else if (m_volumes[1]) {
    m_nChannel = 2;
    m_is2DImage = (m_volumes[1]->is2DData());
  } else if (m_volumes[0]) {
    m_nChannel = 1;
    m_is2DImage = (m_volumes[0]->is2DData());
  } else {
    m_nChannel = 0;
    m_is2DImage = false;
  }

  for (size_t i=0; i<5; ++i) {
    if (i<m_nChannel) {
      m_channelVisibleParas[i]->setVisible(true);
      m_transferFuncParas[i]->setVisible(true);
      m_texFilterModeParas[i]->setVisible(true);
    } else {
      m_channelVisibleParas[i]->setVisible(false);
      m_transferFuncParas[i]->setVisible(false);
      m_texFilterModeParas[i]->setVisible(false);
    }
  }
  if (oldNumChannel != m_nChannel) {
    compile();
    //if (oldNumChannel == 1 || m_nChannel == 1)
    resetTransferFunctions();
  }
}

void Z3DVolumeRaycasterRenderer::resetTransferFunctions()
{
#if 1
  for (size_t i=0; i<m_nChannel; ++i) {
    if (m_opaque) {
      m_transferFuncParas[i]->get().reset(
            0.0, 1.0, glm::vec4(0.f),
            glm::vec4(m_volumes[i]->getVolColor(), 1.0));
      m_transferFuncParas[i]->get().addKey(
            ZColorMapKey(0.001, glm::vec4(0.01f, 0.01f, 0.01f,0.0)));
      m_transferFuncParas[i]->get().addKey(
            ZColorMapKey(0.01, glm::vec4(0.01f, 0.01f, 0.01f,1.0)));
    } else {
      m_transferFuncParas[i]->get().reset(
            0.0, 1.0, glm::vec4(0.f),
            glm::vec4(m_volumes[i]->getVolColor(), 1.f));
      //m_transferFuncParas[i]->get().addKey(ZColorMapKey(0.1, glm::vec4(m_volumes[i]->getVolColor(), 1.f) *
      //                                                  glm::vec4(.1f,.1f,.1f,0.f)));
    }
  }
#else
  if (m_nChannel == 1) {
    m_transferFunc1.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(255, 255, 255, 255));
    m_transferFunc1.get().addKey(ZColorMapKey(0.1, glm::col4(25,25,25,0)));
  } else {
    m_transferFunc1.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(255, 0, 0, 255));
    m_transferFunc2.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(0, 255, 0, 255));
    m_transferFunc3.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(0, 0, 255, 255));
    m_transferFunc1.get().addKey(ZColorMapKey(0.1, glm::col4(25,0,0,0)));
    m_transferFunc2.get().addKey(ZColorMapKey(0.1, glm::col4(0,25,0,0)));
    m_transferFunc3.get().addKey(ZColorMapKey(0.1, glm::col4(0,0,25,0)));
  }
  m_transferFunc4.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(255, 255, 255, 255));
#endif
}

void Z3DVolumeRaycasterRenderer::translate(double dx, double dy, double dz)
{
  for (std::vector<Z3DVolume *>::iterator iter = m_volumes.begin();
       iter != m_volumes.end(); ++iter) {
    if (*iter != NULL) {
      (*iter)->translate(dx, dy, dz);
    }
  }
}
