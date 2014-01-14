#include "z3dvolumeslicerenderer.h"

#include "z3dtexture.h"
#include "z3dvolume.h"

Z3DVolumeSliceRenderer::Z3DVolumeSliceRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
{
  m_volumes.resize(5,NULL);
  for (size_t i=0; i<m_volumes.size(); ++i) {
    m_volumeUniformNames.push_back(QString("volume_struct_%1").arg(i+1));
  }
}

Z3DVolumeSliceRenderer::~Z3DVolumeSliceRenderer()
{
}

void Z3DVolumeSliceRenderer::setChannel1(Z3DVolume *vol)
{
  if (vol && !vol->is3DData()) {
    LERROR() << "Input is not 3D volume";
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

void Z3DVolumeSliceRenderer::setChannel2(Z3DVolume *vol)
{
  if (vol && !vol->is3DData()) {
    LERROR() << "Input is not 3D volume";
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

void Z3DVolumeSliceRenderer::setChannel3(Z3DVolume *vol)
{
  if (vol && !vol->is3DData()) {
    LERROR() << "Input is not 3D volume";
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

void Z3DVolumeSliceRenderer::setChannel4(Z3DVolume *vol)
{
  if (vol && !vol->is3DData()) {
    LERROR() << "Input is not 3D volume";
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

void Z3DVolumeSliceRenderer::setChannel5(Z3DVolume *vol)
{
  if (vol && !vol->is3DData()) {
    LERROR() << "Input is not 3D volume";
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

void Z3DVolumeSliceRenderer::setChannels(std::vector<Z3DVolume *> vols)
{
  for (size_t i=0; i<vols.size(); ++i) {
    if (vols[i] && !vols[i]->is3DData()) {
      LERROR() << "Input is not 3D volume";
      return;
    }
  }
  vols.resize(m_volumes.size(), NULL);
  if (m_volumes != vols) {
    m_volumes = vols;
    compile();
  }
}

void Z3DVolumeSliceRenderer::addQuad(const Z3DTriangleList &quad)
{
  if (quad.empty() ||
      (quad.getVertices().size() != 4 && quad.getVertices().size() != 6) ||
      quad.getVertices().size() != quad.get3DTextureCoordinates().size()) {
    LERROR() << "Input quad should be 2D slice with 3D texture coordinates";
    return;
  }
  m_quads.push_back(quad);
}

void Z3DVolumeSliceRenderer::bindVolumes(Z3DShaderProgram &shader)
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

bool Z3DVolumeSliceRenderer::hasVolume() const
{
  for (size_t i=0; i<m_volumes.size(); ++i)
    if (m_volumes[i])
      return true;
  return false;
}

void Z3DVolumeSliceRenderer::compile()
{
  m_volumeSliceShader.setHeaderAndRebuild(generateHeader());
}

void Z3DVolumeSliceRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  m_volumeSliceShader.loadFromSourceFile("transform_with_3dtexture.vert", "volume_slice.frag", generateHeader());
  m_volumeSliceShader.bindFragDataLocation(0, "FragData0");
}

void Z3DVolumeSliceRenderer::deinitialize()
{
  m_volumeSliceShader.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DVolumeSliceRenderer::generateHeader()
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

void Z3DVolumeSliceRenderer::renderUsingOpengl()
{
  //renderUsingGLSL();
}

void Z3DVolumeSliceRenderer::renderPickingUsingOpengl()
{
}

void Z3DVolumeSliceRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  bool needRender = hasVolume() && !m_quads.empty();
  if (!needRender)
    return;

  m_volumeSliceShader.bind();
  m_rendererBase->setGlobalShaderParameters(m_volumeSliceShader, eye);

  bindVolumes(m_volumeSliceShader);

  for (size_t i=0; i<m_quads.size(); ++i)
    renderTriangleList(m_volumeSliceShader, m_quads[i]);

  m_volumeSliceShader.release();
}

void Z3DVolumeSliceRenderer::renderPickingUsingGLSL(Z3DEye)
{
}


