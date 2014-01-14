#include "z3dimage2drenderer.h"

#include "z3dtexture.h"
#include "z3dvolume.h"

Z3DImage2DRenderer::Z3DImage2DRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
{
  m_volumes.resize(5,NULL);
  for (size_t i=0; i<m_volumes.size(); ++i) {
    m_volumeUniformNames.push_back(QString("volume_struct_%1").arg(i+1));
  }
}

Z3DImage2DRenderer::~Z3DImage2DRenderer()
{
}

void Z3DImage2DRenderer::setChannel1(Z3DVolume *vol)
{
  if (vol && !vol->is2DData()) {
    LERROR() << "Input is not 2D image";
    return;
  }
  if (m_volumes[0] != vol) {
    Z3DVolume *oldvol = m_volumes[0];
    m_volumes[0] = vol;
    if ((oldvol && vol == NULL) ||
        (oldvol == NULL && vol))
      compile();
  }
}

void Z3DImage2DRenderer::setChannel2(Z3DVolume *vol)
{
  if (vol && !vol->is2DData()) {
    LERROR() << "Input is not 2D image";
    return;
  }
  if (m_volumes[1] != vol) {
    Z3DVolume *oldvol = m_volumes[1];
    m_volumes[1] = vol;
    if ((oldvol && vol == NULL) ||
        (oldvol == NULL && vol))
      compile();
  }
}

void Z3DImage2DRenderer::setChannel3(Z3DVolume *vol)
{
  if (vol && !vol->is2DData()) {
    LERROR() << "Input is not 2D image";
    return;
  }
  if (m_volumes[2] != vol) {
    Z3DVolume *oldvol = m_volumes[2];
    m_volumes[2] = vol;
    if ((oldvol && vol == NULL) ||
        (oldvol == NULL && vol))
      compile();
  }
}

void Z3DImage2DRenderer::setChannel4(Z3DVolume *vol)
{
  if (vol && !vol->is2DData()) {
    LERROR() << "Input is not 2D image";
    return;
  }
  if (m_volumes[3] != vol) {
    Z3DVolume *oldvol = m_volumes[3];
    m_volumes[3] = vol;
    if ((oldvol && vol == NULL) ||
        (oldvol == NULL && vol))
      compile();
  }
}

void Z3DImage2DRenderer::setChannel5(Z3DVolume *vol)
{
  if (vol && !vol->is2DData()) {
    LERROR() << "Input is not 2D image";
    return;
  }
  if (m_volumes[4] != vol) {
    Z3DVolume *oldvol = m_volumes[4];
    m_volumes[4] = vol;
    if ((oldvol && vol == NULL) ||
        (oldvol == NULL && vol))
      compile();
  }
}

void Z3DImage2DRenderer::setChannels(std::vector<Z3DVolume*> vols)
{
  for (size_t i=0; i<vols.size(); ++i) {
    if (vols[i] && !vols[i]->is2DData()) {
      LERROR() << "Input is not 2D image";
      return;
    }
  }
  vols.resize(m_volumes.size(), NULL);
  if (m_volumes != vols) {
    m_volumes = vols;
    compile();
  }
}

void Z3DImage2DRenderer::addQuad(const Z3DTriangleList &quad)
{
  if (quad.empty() ||
      (quad.getVertices().size() != 4 && quad.getVertices().size() != 6) ||
      quad.getVertices().size() != quad.get2DTextureCoordinates().size()) {
    LERROR() << "Input quad should be 2D slice with 2D texture coordinates";
    return;
  }
  m_quads.push_back(quad);
}

void Z3DImage2DRenderer::bindVolumes(Z3DShaderProgram &shader)
{
  for (size_t i=0; i < m_volumes.size(); ++i) {
    Z3DVolume *volume = m_volumes[i];
    if (!volume)
      continue;

    // volumes
    shader.bindVolume(m_volumeUniformNames[i], volume, GL_NEAREST, GL_NEAREST);

    CHECK_GL_ERROR;
  }
}

bool Z3DImage2DRenderer::hasVolume() const
{
  for (size_t i=0; i<m_volumes.size(); ++i)
    if (m_volumes[i])
      return true;
  return false;
}

void Z3DImage2DRenderer::compile()
{
  m_image2DShader.setHeaderAndRebuild(generateHeader());
}

void Z3DImage2DRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  m_image2DShader.loadFromSourceFile("transform_with_2dtexture.vert", "image2d.frag", generateHeader());
  m_image2DShader.bindFragDataLocation(0, "FragData0");
}

void Z3DImage2DRenderer::deinitialize()
{
  m_image2DShader.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DImage2DRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();

  if (hasVolume()) {
    if (m_volumes[0]) {
      headerSource += "#define VOLUME_1_EXIST\n";
    }
    if (m_volumes[1]) {
      headerSource += "#define VOLUME_2_EXIST\n";
    }
    if (m_volumes[2]) {
      headerSource += "#define VOLUME_3_EXIST\n";
    }
    if (m_volumes[3]) {
      headerSource += "#define VOLUME_4_EXIST\n";
    }
    if (m_volumes[4]) {
      headerSource += "#define VOLUME_5_EXIST\n";
    }
  } else {
    headerSource += "#define DISABLE_TEXTURE_COORD_OUTPUT\n";
  }

  return headerSource;
}

void Z3DImage2DRenderer::renderUsingOpengl()
{
  //renderUsingGLSL();
}

void Z3DImage2DRenderer::renderPickingUsingOpengl()
{
}

void Z3DImage2DRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  bool needRender = hasVolume() && !m_quads.empty();
  if (!needRender)
    return;

  m_image2DShader.bind();
  m_rendererBase->setGlobalShaderParameters(m_image2DShader, eye);

  bindVolumes(m_image2DShader);

  for (size_t i=0; i<m_quads.size(); ++i)
    renderTriangleList(m_image2DShader, m_quads[i]);

  m_image2DShader.release();
}

void Z3DImage2DRenderer::renderPickingUsingGLSL(Z3DEye)
{
}


