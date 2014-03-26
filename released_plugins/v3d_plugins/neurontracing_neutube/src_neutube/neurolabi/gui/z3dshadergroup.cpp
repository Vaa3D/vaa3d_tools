#include "zglew.h"
#include "z3dshadergroup.h"

#include "z3dgpuinfo.h"

Z3DShaderGroup::Z3DShaderGroup()
  : m_base(NULL)
{
}

Z3DShaderGroup::~Z3DShaderGroup()
{
  removeAllShaders();
}

void Z3DShaderGroup::init(const QStringList &shaderFiles, const QString &header, Z3DRendererBase *base,
                          const QStringList &normalShaderFiles)
{
  m_shaderFiles = shaderFiles;
  m_header = header;
  m_base = base;
  m_normalShaderFiles = normalShaderFiles;
  m_shaders[Z3DRendererBase::Normal] = new Z3DShaderProgram();
  buildNormalShader(m_shaders[Z3DRendererBase::Normal]);
}

void Z3DShaderGroup::removeAllShaders()
{
  std::map<Z3DRendererBase::ShaderHookType, Z3DShaderProgram*>::iterator i = m_shaders.begin();
  while (i != m_shaders.end()) {
    i->second->removeAllShaders();
    delete i->second;
    ++i;
  }
  m_shaders.clear();
}

void Z3DShaderGroup::addAllSupportedPostShaders()
{
  addDualDepthPeelingShaders();
  addWeightedAverageShaders();
}

void Z3DShaderGroup::addDualDepthPeelingShaders()
{
  if (Z3DGpuInfoInstance.isDualDepthPeelingSupported()) {
    m_shaders[Z3DRendererBase::DualDepthPeelingInit] = new Z3DShaderProgram();
    m_shaders[Z3DRendererBase::DualDepthPeelingPeel] = new Z3DShaderProgram();
    buildDualDepthPeelingInitShader(m_shaders[Z3DRendererBase::DualDepthPeelingInit]);
    buildDualDepthPeelingPeelShader(m_shaders[Z3DRendererBase::DualDepthPeelingPeel]);
  }
}

void Z3DShaderGroup::addWeightedAverageShaders()
{
  if (Z3DGpuInfoInstance.isWeightedAverageSupported()) {
    m_shaders[Z3DRendererBase::WeightedAverageInit] = new Z3DShaderProgram();
    buildWeightedAverageShader(m_shaders[Z3DRendererBase::WeightedAverageInit]);
  }
}

void Z3DShaderGroup::bind()
{
  if (m_shaders.find(m_base->getShaderHookType()) != m_shaders.end()) {
    get().bind();
    if (m_base->getShaderHookType() == Z3DRendererBase::DualDepthPeelingPeel) {
      get().bindTexture("DepthBlenderTex", m_base->shaderHookPara().dualDepthPeelingDepthBlenderTexture);
      get().bindTexture("FrontBlenderTex", m_base->shaderHookPara().dualDepthPeelingFrontBlenderTexture);
    }
  } else {
    LERROR() << "not supposed to happen";
    throw Exception("Not supported post process");
  }
}

void Z3DShaderGroup::release()
{
  // if bind is ok, this should be fine
  get().release();
}

Z3DShaderProgram& Z3DShaderGroup::get()
{
  return *m_shaders[m_base->getShaderHookType()];
}

void Z3DShaderGroup::rebuild(const QString &header)
{
  m_header = header;
  std::map<Z3DRendererBase::ShaderHookType, Z3DShaderProgram*>::iterator i = m_shaders.begin();
  while (i != m_shaders.end()) {
    i->second->removeAllShaders();
    switch (i->first) {
    case Z3DRendererBase::Normal:
      buildNormalShader(i->second);
      break;
    case Z3DRendererBase::DualDepthPeelingInit:
      buildDualDepthPeelingInitShader(i->second);
      break;
    case Z3DRendererBase::DualDepthPeelingPeel:
      buildDualDepthPeelingPeelShader(i->second);
      break;
    case Z3DRendererBase::WeightedAverageInit:
      buildWeightedAverageShader(i->second);
    default:
      break;
    }
    ++i;
  }
}

void Z3DShaderGroup::buildNormalShader(Z3DShaderProgram *shader)
{
  if (m_normalShaderFiles.empty()) {
    QStringList allshaders(m_shaderFiles);
    allshaders << "common.frag";
    shader->loadFromSourceFile(allshaders, m_header);
    shader->bindFragDataLocation(0, "FragData0");
  } else {
    shader->loadFromSourceFile(m_normalShaderFiles, m_header);
    shader->bindFragDataLocation(0, "FragData0");
  }
}

void Z3DShaderGroup::buildDualDepthPeelingInitShader(Z3DShaderProgram *shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "dual_peeling_init.frag";
  QString header = m_header;
  if (GLEW_VERSION_3_0) {
    header += "out vec4 FragData1;\n";
  } else {
    header += "#define FragData1 gl_FragData[1]\n";
  }
  shader->loadFromSourceFile(allshaders, header);
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
}

//#define USE_RECT_TEX

void Z3DShaderGroup::buildDualDepthPeelingPeelShader(Z3DShaderProgram *shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "dual_peeling_peel.frag";
  QString header = m_header;
  if (GLEW_VERSION_3_0) {
    header += "out vec4 FragData1;\n";
    header += "out vec4 FragData2;\n";
  } else {
    header += "#define FragData1 gl_FragData[1]\n";
    header += "#define FragData2 gl_FragData[2]\n";
  }
#ifdef USE_RECT_TEX
  header += "#define USE_RECT_TEX\n";
#endif
  shader->loadFromSourceFile(allshaders, header);
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->bindFragDataLocation(2, "FragData2");
}

void Z3DShaderGroup::buildWeightedAverageShader(Z3DShaderProgram *shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "wavg_init.frag";
  QString header = m_header;
  if (GLEW_VERSION_3_0) {
    header += "out vec4 FragData1;\n";
  } else {
    header += "#define FragData1 gl_FragData[1]\n";
  }
  shader->loadFromSourceFile(allshaders, header);
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
}
