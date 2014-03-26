#include "zglew.h"
#include "z3dfontrenderer.h"
#include "z3dapplication.h"
#include <QDir>
#include "z3dsdfont.h"
#include "z3dgpuinfo.h"

Z3DFontRenderer::Z3DFontRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_fontShaderGrp()
  , m_allFontNames("Font")
  , m_fontSize("Font Size", 32.f, .1f, 5000.f)
  , m_fontUseSoftEdge("Font Use Softedge", true)
  , m_fontSoftEdgeScale("Font Softedge Scale", 80.f, 70.f, 200.f)
  , m_showFontOutline("Show Font Outline", false)
  , m_fontOutlineMode("Font Outline Mode")
  , m_fontOutlineColor("Font Outline Color", glm::vec4(1.f))
  , m_showFontShadow("Show Font Shadow", false)
  , m_fontShadowColor("Font Shadow Color", glm::vec4(0.f, 0.f, 0.f, 1.f))
  , m_positionsPt(NULL)
  , m_colorsPt(NULL)
  , m_pickingColorsPt(NULL)
  , m_VBOs(4)
  , m_pickingVBOs(4)
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
{
  addParameter(&m_allFontNames);
  m_fontSize.setSingleStep(0.1);
  m_fontSize.setDecimal(1);
  addParameter(m_fontSize);
  addParameter(m_fontUseSoftEdge);
  addParameter(m_fontSoftEdgeScale);
  addParameter(m_showFontOutline);
  m_fontOutlineMode.addOptions("Glow", "Outline");
  m_fontOutlineMode.select("Glow");
  addParameter(m_fontOutlineMode);
  m_fontOutlineColor.setStyle("COLOR");
  addParameter(m_fontOutlineColor);
  addParameter(m_showFontShadow);
  m_fontShadowColor.setStyle("COLOR");
  addParameter(m_fontShadowColor);
  adjustWidgets();
  connect(&m_fontUseSoftEdge, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));
  connect(&m_fontUseSoftEdge, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_showFontOutline, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));
  connect(&m_showFontOutline, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_fontOutlineMode, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_showFontShadow, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));
  connect(&m_showFontShadow, SIGNAL(valueChanged()), this, SLOT(compile()));
}

Z3DFontRenderer::~Z3DFontRenderer()
{
}

void Z3DFontRenderer::setData(std::vector<glm::vec3> *positions, const QStringList &texts)
{
  m_positionsPt = positions;
  m_texts = texts;
  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DFontRenderer::setDataColors(std::vector<glm::vec4> *colors)
{
  m_colorsPt = colors;
  m_colors.clear();
  invalidateOpenglRenderer();
  m_dataChanged = true;
}

void Z3DFontRenderer::setDataPickingColors(std::vector<glm::vec4> *pickingColors)
{
  m_pickingColorsPt = pickingColors;
  invalidateOpenglPickingRenderer();
  m_pickingDataChanged = true;
}

void Z3DFontRenderer::adjustWidgets()
{
  m_fontSoftEdgeScale.setVisible(m_fontUseSoftEdge.get());
  m_fontOutlineColor.setVisible(m_showFontOutline.get());
  m_fontOutlineMode.setVisible(m_showFontOutline.get());
  m_fontShadowColor.setVisible(m_showFontShadow.get());
}

void Z3DFontRenderer::compile()
{
  m_fontShaderGrp.rebuild(generateHeader());
}

void Z3DFontRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList allshaders;
  allshaders << "almag.vert" << "almag_func.frag";
  QStringList normalShaders;
  normalShaders << "almag.vert" << "almag.frag";
  m_fontShaderGrp.init(allshaders, generateHeader(), m_rendererBase, normalShaders);
  m_fontShaderGrp.addAllSupportedPostShaders();

  // search for available fonts
  QDir fontDir(Z3DApplication::app()->getFontPath());
  QStringList filters;
  filters << "*.png";
  QFileInfoList list = fontDir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks);
  for (int i=0; i<list.size(); i++) {
    QFileInfo fileInfo = list.at(i);
    QFileInfo txtFileInfo(fontDir, fileInfo.completeBaseName() + ".txt");
    if (!txtFileInfo.exists())
      continue;
    Z3DSDFont *sdFont = new Z3DSDFont(fileInfo.absoluteFilePath(),
                                      txtFileInfo.absoluteFilePath());
    if (sdFont->isEmpty())
      delete sdFont;
    else {
      m_allFontNames.addOptionWithData(qMakePair(sdFont->getFontName(), m_allFonts.size()));
      m_allFonts.push_back(sdFont);
    }
  }

  glGenBuffers(m_VBOs.size(), &m_VBOs[0]);
  glGenBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);
}

void Z3DFontRenderer::deinitialize()
{
  glDeleteBuffers(m_VBOs.size(), &m_VBOs[0]);
  glDeleteBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);

  m_fontShaderGrp.removeAllShaders();
  for (size_t i=0; i<m_allFonts.size(); i++)
    delete m_allFonts[i];
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

std::vector<glm::vec4> *Z3DFontRenderer::getColors()
{
  if (!m_colorsPt) {
    m_colors.assign(m_positionsPt->size(), glm::vec4(0.f, 0.f, 0.f, 1.f));
    return &m_colors;
  } else if (m_colorsPt->size() < m_positionsPt->size()) {
    m_colors.clear();
    for (size_t i=0; i<m_colorsPt->size(); i++) {
      m_colors.push_back(m_colorsPt->at(i));
    }
    for (size_t i=m_colorsPt->size(); i<m_positionsPt->size(); i++) {
      m_colors.push_back(glm::vec4(0.f, 0.f, 0.f, 1.f));
    }
    return &m_colors;
  }

  return m_colorsPt;
}

QString Z3DFontRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();
  if (m_fontUseSoftEdge.get())
    headerSource += "#define USE_SOFTEDGE\n";
  if (m_showFontOutline.get()) {
    if (m_fontOutlineMode.isSelected("Glow"))
      headerSource += "#define SHOW_GLOW\n";
    else
      headerSource += "#define SHOW_OUTLINE\n";
  }
  if (m_showFontShadow.get())
    headerSource += "#define SHOW_SHADOW\n";
  return headerSource;
}

void Z3DFontRenderer::renderUsingOpengl()
{
  //renderUsingGLSL();
}

void Z3DFontRenderer::renderPickingUsingOpengl()
{
}

void Z3DFontRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (m_allFontNames.isEmpty()) {
    LERROR() << "Can not find any font.";
    return;
  }
  if (!m_positionsPt || m_positionsPt->empty()
      || m_positionsPt->size() != static_cast<size_t>(m_texts.size()))
    return;

  prepareFontShaderData(eye);

  Z3DSDFont* font = m_allFonts[m_allFontNames.getAssociatedData()];

  if (m_rendererBase->getShaderHookType() == Z3DRendererBase::Normal) {
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
  }

  m_fontShaderGrp.bind();
  Z3DShaderProgram &shader = m_fontShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(shader, eye);
  shader.bindTexture("tex", font->getTexture());
  if (m_fontUseSoftEdge.get())
    shader.setUniformValue("softedge_scale", m_fontSoftEdgeScale.get());
  if (m_showFontOutline.get())
    shader.setUniformValue("outline_color", m_fontOutlineColor.get());
  if (m_showFontShadow.get())
    shader.setUniformValue("shadow_color", m_fontShadowColor.get());

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      glBindVertexArray(m_VAO);
      // set vertex data
      GLint attr_vertex = shader.attributeLocation("attr_vertex");
      GLint attr_2dTexCoord0 = shader.attributeLocation("attr_2dTexCoord0");
      GLint attr_color = shader.attributeLocation("attr_color");

      glEnableVertexAttribArray(attr_vertex);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
      glBufferData(GL_ARRAY_BUFFER, m_fontPositions.size()*3*sizeof(GLfloat), &(m_fontPositions[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_2dTexCoord0);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
      glBufferData(GL_ARRAY_BUFFER, m_fontTextureCoords.size()*2*sizeof(GLfloat), &(m_fontTextureCoords[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_2dTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_color);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[2]);
      glBufferData(GL_ARRAY_BUFFER, m_fontColors.size()*4*sizeof(GLfloat), &(m_fontColors[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexs.size()*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      m_dataChanged = false;
    }

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexs.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

  } else {
    // set vertex data
    GLint attr_vertex = shader.attributeLocation("attr_vertex");
    GLint attr_2dTexCoord0 = shader.attributeLocation("attr_2dTexCoord0");
    GLint attr_color = shader.attributeLocation("attr_color");

    glEnableVertexAttribArray(attr_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, m_fontPositions.size()*3*sizeof(GLfloat), &(m_fontPositions[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_2dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, m_fontTextureCoords.size()*2*sizeof(GLfloat), &(m_fontTextureCoords[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_2dTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_color);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, m_fontColors.size()*4*sizeof(GLfloat), &(m_fontColors[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexs.size()*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, m_indexs.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(attr_vertex);
    glDisableVertexAttribArray(attr_2dTexCoord0);
    glDisableVertexAttribArray(attr_color);
  }

  m_fontShaderGrp.release();

  if (m_rendererBase->getShaderHookType() == Z3DRendererBase::Normal) {
    glPopAttrib();
  }
}

void Z3DFontRenderer::renderPickingUsingGLSL(Z3DEye)
{
  if (m_allFontNames.isEmpty()) {
    LERROR() << "Can not find any font.";
    return;
  }
  if (!m_pickingColorsPt || m_pickingColorsPt->empty()
      || m_pickingColorsPt->size() != m_positionsPt->size())
    return;
  if (!m_positionsPt || m_positionsPt->empty()
      || m_positionsPt->size() != static_cast<size_t>(m_texts.size()))
    return;


}

void Z3DFontRenderer::prepareFontShaderData(Z3DEye eye)
{
  m_fontPositions.clear();
  m_fontTextureCoords.clear();
  m_fontColors.clear();
  m_fontPickingColors.clear();
  m_indexs.clear();
  glm::mat4 viewMatrix = m_rendererBase->getViewMatrix(eye);
  glm::vec3 rightVector(viewMatrix[0][0], viewMatrix[0][1], viewMatrix[0][2]);
  glm::vec3 upVector(viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2]);
  Z3DSDFont* font = m_allFonts[m_allFontNames.getAssociatedData()];
  float scale = m_fontSize.get() / font->getMaxFontHeight();
  int indices[6] = { 0, 1, 2, 2, 1, 3 };
  int quadIdx = 0;
  for (int strIdx=0; strIdx < m_texts.size(); strIdx++) {
    QString str = m_texts[strIdx];
    if (str.isEmpty())
      continue;
    glm::vec4 color;
    if (!m_colorsPt || static_cast<size_t>(strIdx) >= m_colorsPt->size())
      color = glm::vec4(0.f, 0.f, 0.f, 1.f);
    else
      color = m_colorsPt->at(strIdx);
    glm::vec3 loc = m_positionsPt->at(strIdx);
    for (int charIdx=0; charIdx < str.size(); charIdx++) {
      Z3DSDFont::CharInfo charInfo = font->getCharInfo(str[charIdx].toLatin1());
      glm::vec3 leftUp = loc + rightVector * charInfo.xoffset * scale + upVector * charInfo.yoffset * scale;
      glm::vec3 leftDown = leftUp - upVector * static_cast<float>(charInfo.height) * scale;
      glm::vec3 rightUp = leftUp + rightVector * static_cast<float>(charInfo.width) * scale;
      glm::vec3 rightDown = leftDown + rightVector * static_cast<float>(charInfo.width) * scale;
      m_fontPositions.push_back(leftDown);
      m_fontPositions.push_back(rightDown);
      m_fontPositions.push_back(leftUp);
      m_fontPositions.push_back(rightUp);
      m_fontTextureCoords.push_back(glm::vec2(charInfo.sMin, charInfo.tMin));
      m_fontTextureCoords.push_back(glm::vec2(charInfo.sMax, charInfo.tMin));
      m_fontTextureCoords.push_back(glm::vec2(charInfo.sMin, charInfo.tMax));
      m_fontTextureCoords.push_back(glm::vec2(charInfo.sMax, charInfo.tMax));
      m_fontColors.push_back(color);
      m_fontColors.push_back(color);
      m_fontColors.push_back(color);
      m_fontColors.push_back(color);
      if (m_pickingColorsPt && m_pickingColorsPt->size() == m_positionsPt->size()) {
        m_fontPickingColors.push_back(m_pickingColorsPt->at(strIdx));
        m_fontPickingColors.push_back(m_pickingColorsPt->at(strIdx));
        m_fontPickingColors.push_back(m_pickingColorsPt->at(strIdx));
        m_fontPickingColors.push_back(m_pickingColorsPt->at(strIdx));
      }
      for (int k=0; k<6; k++) {
        m_indexs.push_back(indices[k] + 4 * quadIdx);
      }
      quadIdx++;
      loc += rightVector * charInfo.xadvance * scale;
    }
  }
}
