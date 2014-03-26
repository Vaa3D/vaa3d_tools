#include "zglew.h"
#include "z3dlinerenderer.h"
#include "z3dgpuinfo.h"

Z3DLineRenderer::Z3DLineRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_lineShaderGrp()
  , m_linesPt(NULL)
  , m_lineColorsPt(NULL)
  , m_linePickingColorsPt(NULL)
  , m_lineWidth(1.0)
  , m_VBOs(2)
  , m_pickingVBOs(2)
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
{
  setNeedLighting(false);
  setUseDisplayList(true);
}

Z3DLineRenderer::~Z3DLineRenderer()
{
}

void Z3DLineRenderer::setData(std::vector<glm::vec3> *linesInput)
{
  m_linesPt = linesInput;

  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DLineRenderer::setDataColors(std::vector<glm::vec4> *lineColorsInput)
{
  m_lineColorsPt = lineColorsInput;
  invalidateOpenglRenderer();
  m_dataChanged = true;
}

void Z3DLineRenderer::setDataPickingColors(std::vector<glm::vec4> *linePickingColorsInput)
{
  m_linePickingColorsPt = linePickingColorsInput;
  invalidateOpenglPickingRenderer();
  m_pickingDataChanged = true;
}

void Z3DLineRenderer::compile()
{
  m_lineShaderGrp.rebuild(generateHeader());
}

void Z3DLineRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList allshaders;
  allshaders << "line.vert" << "line_func.frag";
  QStringList normalShaders;
  normalShaders << "line.vert" << "line.frag";
  m_lineShaderGrp.init(allshaders, generateHeader(), m_rendererBase,
                       normalShaders);
  m_lineShaderGrp.addAllSupportedPostShaders();

  glGenBuffers(m_VBOs.size(), &m_VBOs[0]);
  glGenBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);
}

void Z3DLineRenderer::deinitialize()
{
  glDeleteBuffers(m_VBOs.size(), &m_VBOs[0]);
  glDeleteBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);

  m_lineShaderGrp.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

float Z3DLineRenderer::getLineWidth() const
{
  return getSizeScale() * m_lineWidth;
}

std::vector<glm::vec4> * Z3DLineRenderer::getLineColors()
{
  if (!m_lineColorsPt) {
    m_lineColors.assign(m_linesPt->size(), glm::vec4(0.f, 0.f, 0.f, 1.f));
    return &m_lineColors;
  } else if (m_lineColorsPt->size() < m_linesPt->size()) {
    m_lineColors.clear();
    for (size_t i=0; i<m_lineColorsPt->size(); i++) {
      m_lineColors.push_back(m_lineColorsPt->at(i));
    }
    for (size_t i=m_lineColorsPt->size(); i<m_linesPt->size(); i++) {
      m_lineColors.push_back(glm::vec4(0.f, 0.f, 0.f, 1.f));
    }
    return &m_lineColors;
  }

  return m_lineColorsPt;
}

void Z3DLineRenderer::renderUsingOpengl()
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  std::vector<glm::vec4> * colors = getLineColors();

  if (colors->at(0).a != getOpacity()) {
    for (size_t i=0; i<colors->size(); i++)
      colors->at(i).a = getOpacity();
  }

  glLineWidth(getLineWidth());
  glPointSize(getLineWidth());

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef(getCoordScales().x, getCoordScales().y, getCoordScales().z);

  GLuint bufObjects[2];
  glGenBuffers(2, bufObjects);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), &((*m_linesPt)[0]), GL_STATIC_DRAW);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
  glBufferData(GL_ARRAY_BUFFER, colors->size()*4*sizeof(GLfloat), &((*colors)[0]), GL_STATIC_DRAW);
  glColorPointer(4, GL_FLOAT, 0, 0);

  glDrawArrays(GL_LINES, 0, m_linesPt->size());
  glDrawArrays(GL_POINTS, 0, m_linesPt->size());

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, bufObjects);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glLineWidth(1.0);
  glPointSize(1.0);
}

void Z3DLineRenderer::renderPickingUsingOpengl()
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  if (!m_linePickingColorsPt || m_linePickingColorsPt->empty()
      || m_linePickingColorsPt->size() != m_linesPt->size())
    return;

  glLineWidth(getLineWidth());
  glPointSize(getLineWidth());

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef(getCoordScales().x, getCoordScales().y, getCoordScales().z);

  GLuint bufObjects[2];
  glGenBuffers(2, bufObjects);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), &((*m_linesPt)[0]), GL_STATIC_DRAW);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
  glBufferData(GL_ARRAY_BUFFER, m_linePickingColorsPt->size()*4*sizeof(GLfloat), &((*m_linePickingColorsPt)[0]), GL_STATIC_DRAW);
  glColorPointer(4, GL_FLOAT, 0, 0);

  glDrawArrays(GL_LINES, 0, m_linesPt->size());
  glDrawArrays(GL_POINTS, 0, m_linesPt->size());

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, bufObjects);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glLineWidth(1.0);
  glPointSize(1.0);
}

void Z3DLineRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (!m_linesPt || m_linesPt->empty())
    return;

  const std::vector<glm::vec4> * colors = getLineColors();

  glLineWidth(getLineWidth());
  glPointSize(getLineWidth());

  m_lineShaderGrp.bind();
  Z3DShaderProgram &shader = m_lineShaderGrp.get();
  m_rendererBase->setGlobalShaderParameters(shader, eye);
  shader.setUniformValue("pos_scale", getCoordScales());
  shader.setUniformValue("no_alpha", false);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      glBindVertexArray(m_VAO);
      GLint attr_vertex = shader.attributeLocation("attr_vertex");
      GLint attr_color = shader.attributeLocation("attr_color");

      glEnableVertexAttribArray(attr_vertex);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
      glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), &((*m_linesPt)[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_color);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
      glBufferData(GL_ARRAY_BUFFER, colors->size()*4*sizeof(GLfloat), &((*colors)[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      m_dataChanged = false;
    }

    glBindVertexArray(m_VAO);
    if (!m_lineWidthArray.empty()) {
      for (size_t i = 0; i < m_lineWidthArray.size(); ++i) {
        glLineWidth(m_lineWidthArray[i]);
        glDrawArrays(GL_LINES, i * 2, 2);
      }
    } else {
      glDrawArrays(GL_LINES, 0, m_linesPt->size());
    }

#ifndef _FLYEM_
    glDrawArrays(GL_POINTS, 0, m_linesPt->size());
#endif
    glBindVertexArray(0);

  } else {
    GLint attr_vertex = shader.attributeLocation("attr_vertex");
    GLint attr_color = shader.attributeLocation("attr_color");

    glEnableVertexAttribArray(attr_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), &((*m_linesPt)[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

#ifdef _DEBUG_2
    std::cout << "alpha: " << colors[0][3] << std::endl;
#endif

    glEnableVertexAttribArray(attr_color);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, colors->size()*4*sizeof(GLfloat), &((*colors)[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

    if (!m_lineWidthArray.empty()) {
      for (size_t i = 0; i < m_lineWidthArray.size(); ++i) {
        glLineWidth(m_lineWidthArray[i]);
        glDrawArrays(GL_LINES, i * 2, 2);
      }
    } else {
      glDrawArrays(GL_LINES, 0, m_linesPt->size());
    }

#ifndef _FLYEM_
    glDrawArrays(GL_POINTS, 0, m_linesPt->size());
#endif

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(attr_color);
    glDisableVertexAttribArray(attr_vertex);

    m_dataChanged = false;
  }

  glLineWidth(1.0);
  glPointSize(1.0);

  m_lineShaderGrp.release();
}

void Z3DLineRenderer::renderPickingUsingGLSL(Z3DEye eye)
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  if (!m_linePickingColorsPt || m_linePickingColorsPt->empty()
      || m_linePickingColorsPt->size() != m_linesPt->size())
    return;

  glLineWidth(getLineWidth());
  glPointSize(getLineWidth());

  m_lineShaderGrp.bind();
  Z3DShaderProgram &shader = m_lineShaderGrp.get();
  m_rendererBase->setGlobalShaderParameters(shader, eye);
  shader.setUniformValue("pos_scale", getCoordScales());
  shader.setUniformValue("no_alpha", true);

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      glBindVertexArray(m_pickingVAO);
      GLint attr_vertex = shader.attributeLocation("attr_vertex");
      GLint attr_color = shader.attributeLocation("attr_color");

      glEnableVertexAttribArray(attr_vertex);
      if (m_dataChanged) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[0]);
        glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), &((*m_linesPt)[0]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
      }
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_color);
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[1]);
      glBufferData(GL_ARRAY_BUFFER, m_linePickingColorsPt->size()*4*sizeof(GLfloat), &((*m_linePickingColorsPt)[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      m_pickingDataChanged = false;
    }

    glBindVertexArray(m_pickingVAO);
    glDrawArrays(GL_LINES, 0, m_linesPt->size());
    glDrawArrays(GL_POINTS, 0, m_linesPt->size());
    glBindVertexArray(0);

  } else {
    GLint attr_vertex = shader.attributeLocation("attr_vertex");
    GLint attr_color = shader.attributeLocation("attr_color");

    glEnableVertexAttribArray(attr_vertex);
    if (m_dataChanged) {
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[0]);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), &((*m_linesPt)[0]), GL_STATIC_DRAW);
    } else {
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
    }
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_color);
    glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[1]);
    if (m_pickingDataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_linePickingColorsPt->size()*4*sizeof(GLfloat), &((*m_linePickingColorsPt)[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_LINES, 0, m_linesPt->size());
    glDrawArrays(GL_POINTS, 0, m_linesPt->size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(attr_color);
    glDisableVertexAttribArray(attr_vertex);

    m_pickingDataChanged = false;
  }

  glLineWidth(1.0);
  glPointSize(1.0);

  m_lineShaderGrp.release();
}

void Z3DLineRenderer::enableLineSmooth()
{
#if defined(_WIN32) || defined(_WIN64)
  if (Z3DGpuInfoInstance.getGpuVendor() == Z3DGpuInfo::GPU_VENDOR_ATI) {
    return;
  }
#endif
  return;
  if (m_rendererBase->getShaderHookType() == Z3DRendererBase::Normal) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glPushAttrib(GL_LINE_BIT);
    glDisable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
  }
}

void Z3DLineRenderer::disableLineSmooth()
{
#if defined(_WIN32) || defined(_WIN64)
  if (Z3DGpuInfoInstance.getGpuVendor() == Z3DGpuInfo::GPU_VENDOR_ATI) {
    return;
  }
#endif
  return;
  glPopAttrib();
}
