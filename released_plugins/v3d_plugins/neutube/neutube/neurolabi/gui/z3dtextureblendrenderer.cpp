#include "z3dtextureblendrenderer.h"

#include "z3dtexture.h"

Z3DTextureBlendRenderer::Z3DTextureBlendRenderer(const QString &mode, QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_colorTexture1(NULL)
  , m_depthTexture1(NULL)
  , m_colorTexture2(NULL)
  , m_depthTexture2(NULL)
  , m_blendMode("Blend Mode")
{
  m_blendMode.addOptionsWithData(qMakePair<QString,QString>("DepthTest", "DEPTH_TEST"),
                                 qMakePair<QString,QString>("FirstOnTop", "FIRST_ON_TOP"),
                                 qMakePair<QString,QString>("SecondOnTop", "SECOND_ON_TOP"),
                                 qMakePair<QString,QString>("DepthTestBlending", "DEPTH_TEST_BLENDING"),
                                 qMakePair<QString,QString>("FirstOnTopBlending", "FIRST_ON_TOP_BLENDING"),
                                 qMakePair<QString,QString>("SecondOnTopBlending", "SECOND_ON_TOP_BLENDING")
                                 );
  m_blendMode.select(mode);
  addParameter(m_blendMode);
  connect(&m_blendMode, SIGNAL(valueChanged()), this, SLOT(compile()));
}

void Z3DTextureBlendRenderer::compile()
{
  m_blendTextureShader.setHeaderAndRebuild(generateHeader());
}

void Z3DTextureBlendRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  m_blendTextureShader.loadFromSourceFile("pass.vert", "compositor.frag", generateHeader());
  m_blendTextureShader.bindFragDataLocation(0, "FragData0");
}

void Z3DTextureBlendRenderer::deinitialize()
{
  m_blendTextureShader.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DTextureBlendRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();
  headerSource += QString("#define %1\n").arg(m_blendMode.getAssociatedData());
  return headerSource;
}

void Z3DTextureBlendRenderer::renderUsingOpengl()
{
}

void Z3DTextureBlendRenderer::renderPickingUsingOpengl()
{
}

void Z3DTextureBlendRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_colorTexture1 == NULL || m_depthTexture1 == NULL ||
      m_colorTexture2 == NULL || m_depthTexture2 == NULL)
    return;

  m_blendTextureShader.bind();
  m_rendererBase->setGlobalShaderParameters(m_blendTextureShader, eye);

  m_blendTextureShader.bindTexture("color_texture_0", m_colorTexture1);
  m_blendTextureShader.bindTexture("depth_texture_0", m_depthTexture1);

  m_blendTextureShader.bindTexture("color_texture_1", m_colorTexture2);
  m_blendTextureShader.bindTexture("depth_texture_1", m_depthTexture2);

  renderScreenQuad(m_blendTextureShader);
  m_blendTextureShader.release();
}

void Z3DTextureBlendRenderer::renderPickingUsingGLSL(Z3DEye)
{
}
