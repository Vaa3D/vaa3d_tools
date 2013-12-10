#include "z3dtexturecopyrenderer.h"

#include "z3dtexture.h"

Z3DTextureCopyRenderer::Z3DTextureCopyRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_colorTexture(NULL)
  , m_depthTexture(NULL)
  , m_colorOpAlpha(0.f)
  , m_discardTransparent(false)
{
}

void Z3DTextureCopyRenderer::compile()
{
  m_copyTextureShader.setHeaderAndRebuild(generateHeader());
}

void Z3DTextureCopyRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  m_copyTextureShader.loadFromSourceFile("pass.vert", "copyimage.frag", generateHeader());
  m_copyTextureShader.bindFragDataLocation(0, "FragData0");
}

void Z3DTextureCopyRenderer::deinitialize()
{
  m_copyTextureShader.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

void Z3DTextureCopyRenderer::renderUsingOpengl()
{
  //renderUsingGLSL();
}

void Z3DTextureCopyRenderer::renderPickingUsingOpengl()
{
}

void Z3DTextureCopyRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_colorTexture == NULL || m_depthTexture == NULL)
    return;

  m_copyTextureShader.bind();
  m_rendererBase->setGlobalShaderParameters(m_copyTextureShader, eye);
  m_copyTextureShader.setUniformValue("output_color_option", m_colorOpAlpha);
  m_copyTextureShader.setUniformValue("discard_transparent", m_discardTransparent);

  // pass texture parameters to the shader
  m_copyTextureShader.bindTexture("color_texture", m_colorTexture);
  m_copyTextureShader.bindTexture("depth_texture", m_depthTexture);

  renderScreenQuad(m_copyTextureShader);
  m_copyTextureShader.release();
}

void Z3DTextureCopyRenderer::renderPickingUsingGLSL(Z3DEye)
{
}
