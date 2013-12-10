#include "z3dconerenderer.h"
#include "z3dgpuinfo.h"

Z3DConeRenderer::Z3DConeRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_coneShaderGrp()
  , m_coneCapStyle("Cone Cap Style")
  , m_cylinderSubdivisionAroundZ("Cylinder Subdivisions Around Z", 36, 20, 100)
  , m_cylinderSubdivisionAlongZ("Cylinder Subdivisions Along Z", 1, 1, 100)
  , m_sameColorForBaseAndTop(false)
  , m_useConeShader2(true)
  , m_VBOs(6)
  , m_pickingVBOs(5)
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
{
  setNeedLighting(true);
  setUseDisplayList(true);

  m_coneCapStyle.addOptionsWithData(qMakePair<QString,QString>("Flat Caps", "FLAT_CAPS"),
                                    qMakePair<QString,QString>("Round Caps", "ROUND_CAPS"),
                                    qMakePair<QString,QString>("No Caps", "NO_CAPS"),
                                    qMakePair<QString,QString>("Round Base Flat Top", "ROUND_BASE_CAP_FLAT_TOP_CAP"),
                                    qMakePair<QString,QString>("Flat Base Round Top", "FLAT_BASE_CAP_ROUND_TOP_CAP"));
  m_coneCapStyle.select("Flat Caps");
  connect(&m_coneCapStyle, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  connect(&m_coneCapStyle, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglPickingRenderer()));
  connect(&m_coneCapStyle, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_cylinderSubdivisionAroundZ, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  connect(&m_cylinderSubdivisionAlongZ, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  addParameter(m_coneCapStyle);
  addParameter(m_cylinderSubdivisionAroundZ);
  addParameter(m_cylinderSubdivisionAlongZ);
}

Z3DConeRenderer::~Z3DConeRenderer()
{
}

void Z3DConeRenderer::setData(std::vector<glm::vec4> *baseAndBaseRadius, std::vector<glm::vec4> *axisAndTopRadius)
{
  m_baseAndBaseRadius.clear();
  m_axisAndTopRadius.clear();
  m_allFlags.clear();
  m_indexs.clear();
  if (m_useConeShader2) {
    int indices[6] = { 0, 1, 2, 2, 1, 3 };
    int quadIdx = 0;
    for (size_t i=0; i<baseAndBaseRadius->size(); i++) {
      m_baseAndBaseRadius.push_back(baseAndBaseRadius->at(i));
      m_baseAndBaseRadius.push_back(baseAndBaseRadius->at(i));
      m_baseAndBaseRadius.push_back(baseAndBaseRadius->at(i));
      m_baseAndBaseRadius.push_back(baseAndBaseRadius->at(i));
      m_axisAndTopRadius.push_back(axisAndTopRadius->at(i));
      m_axisAndTopRadius.push_back(axisAndTopRadius->at(i));
      m_axisAndTopRadius.push_back(axisAndTopRadius->at(i));
      m_axisAndTopRadius.push_back(axisAndTopRadius->at(i));
      for (int k=0; k<6; k++) {
        m_indexs.push_back(indices[k] + 4 * quadIdx);
      }
      quadIdx++;
    }
    size_t rightUpSize = m_allFlags.size();
    float cornerFlags[4] = {0 << 4 | 0,      // (0, 0) left down
                            1 << 4 | 0,      // (1, 0) right down
                            0 << 4 | 1,      // (0, 1) left up
                            1 << 4 | 1};     // (1, 1) right up

    if (rightUpSize > m_baseAndBaseRadius.size()) {
      m_allFlags.resize(m_baseAndBaseRadius.size());
    } else if (rightUpSize < m_baseAndBaseRadius.size()) {
      m_allFlags.resize(m_baseAndBaseRadius.size());
      for (size_t i=rightUpSize; i<m_allFlags.size(); i+=4) {
        m_allFlags[i] = cornerFlags[0];
        m_allFlags[i+1] = cornerFlags[1];
        m_allFlags[i+2] = cornerFlags[2];
        m_allFlags[i+3] = cornerFlags[3];
      }
    }
  } else {
    int indices[6 * 2 * 3] = { 0, 2, 1, 2, 0, 3, 1, 6, 5, 6, 1, 2, 0, 1, 5, 5, 4, 0,
                               0, 7, 3, 7, 0, 4, 3, 6, 2, 6, 3, 7, 4, 5, 6, 6, 7, 4 };
    int rightIdx[8] =  { 0, 1, 1, 0, 0, 1, 1, 0 };
    int upIdx[8] =     { 0, 0, 1, 1, 0, 0, 1, 1 };
    int outIdx[8] =    { 0, 0, 0, 0, 1, 1, 1, 1 };
    int coneIdx = 0;
    for (size_t i=0; i<baseAndBaseRadius->size(); i++) {
      for (int k=0; k<8; k++) {
        m_baseAndBaseRadius.push_back(baseAndBaseRadius->at(i));
        m_axisAndTopRadius.push_back(axisAndTopRadius->at(i));
        m_allFlags.push_back(rightIdx[k] << 8 | upIdx[k] << 4 | outIdx[k]);
      }
      for (int k=0; k<36; k++) {
        m_indexs.push_back(indices[k] + 8 * coneIdx);
      }
      coneIdx++;
    }
  }

  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DConeRenderer::setDataColors(std::vector<glm::vec4> *coneColors)
{
  m_coneBaseColors.clear();
  m_coneTopColors.clear();
  m_sameColorForBaseAndTop = true;
  int dup = m_useConeShader2 ? 4 : 8;
  for (size_t i=0; i<coneColors->size(); i++) {
    for (int k=0; k<dup; k++)
      m_coneBaseColors.push_back(coneColors->at(i));
  }
  invalidateOpenglRenderer();
  m_dataChanged = true;
}

void Z3DConeRenderer::setDataColors(std::vector<glm::vec4> *coneBaseColors, std::vector<glm::vec4> *coneTopColors)
{
  m_coneBaseColors.clear();
  m_coneTopColors.clear();
  m_sameColorForBaseAndTop = false;
  int dup = m_useConeShader2 ? 4 : 8;
  for (size_t i=0; i<coneBaseColors->size(); i++) {
    for (int k=0; k<dup; k++) {
      m_coneBaseColors.push_back(coneBaseColors->at(i));
      m_coneTopColors.push_back(coneTopColors->at(i));
    }
  }
  invalidateOpenglRenderer();
  m_dataChanged = true;
}

void Z3DConeRenderer::setDataPickingColors(std::vector<glm::vec4> *conePickingColors)
{
  m_conePickingColors.clear();
  if (conePickingColors == NULL)
    return;
  int dup = m_useConeShader2 ? 4 : 8;
  for (size_t i=0; i<conePickingColors->size(); i++) {
    for (int k=0; k<dup; k++)
      m_conePickingColors.push_back(conePickingColors->at(i));
  }
  invalidateOpenglPickingRenderer();
  m_pickingDataChanged = true;
}

void Z3DConeRenderer::compile()
{
  m_coneShaderGrp.rebuild(generateHeader());
}

void Z3DConeRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList allshaders;
  if (m_useConeShader2)
    allshaders << "cone_2.vert" << "cone_func_2.frag" << "lighting.frag";
  m_coneShaderGrp.init(allshaders, generateHeader(), m_rendererBase);
  m_coneShaderGrp.addAllSupportedPostShaders();

  glGenBuffers(m_VBOs.size(), &m_VBOs[0]);
  glGenBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);
}

void Z3DConeRenderer::deinitialize()
{
  if (!m_VBOs.empty()) {
    glDeleteBuffers(m_VBOs.size(), &m_VBOs[0]);
  }

  if (!m_pickingVBOs.empty()) {
    glDeleteBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);
  }

  m_coneShaderGrp.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DConeRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();
  headerSource += QString("#define %1\n").arg(m_coneCapStyle.getAssociatedData());
  return headerSource;
}

void Z3DConeRenderer::renderUsingOpengl()
{
  if (m_baseAndBaseRadius.empty())
    return;
  appendDefaultColors();

  GLUquadricObj* quadric = gluNewQuadric();
  int dup = m_useConeShader2 ? 4 : 8;
  for (size_t i=0; i<m_baseAndBaseRadius.size(); i+=dup) {
    glColor4fv(glm::value_ptr(glm::vec4(m_coneBaseColors[i].rgb(), m_coneBaseColors[i].a * getOpacity())));
    glm::vec3 bottomPos = m_baseAndBaseRadius[i].xyz();
    glm::vec3 topPos = m_axisAndTopRadius[i].xyz();
    topPos += bottomPos;
    bottomPos *= getCoordScales();
    topPos *= getCoordScales();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glm::vec3 C = topPos - bottomPos;
    float height = glm::length(C);
    glm::vec3 A, B;
    C = glm::normalize(C);
    glm::getOrthogonalVectors(C, A, B);
    glm::mat4 m(glm::vec4(A, 0.f),
                glm::vec4(B, 0.f),
                glm::vec4(C, 0.f),
                glm::vec4(bottomPos, 1.f));
    glMultMatrixf(&m[0][0]);

    gluCylinder(quadric, getSizeScale()*m_baseAndBaseRadius[i].w, getSizeScale()*m_axisAndTopRadius[i].w, height,
                m_cylinderSubdivisionAroundZ.get(), m_cylinderSubdivisionAlongZ.get());

    if (m_coneCapStyle.isSelected("Round Caps") || m_coneCapStyle.isSelected("Round Base Flat Top")) {
      gluSphere(quadric, getSizeScale()*m_baseAndBaseRadius[i].w, m_cylinderSubdivisionAroundZ.get(), m_cylinderSubdivisionAroundZ.get());
    } else if (m_coneCapStyle.isSelected("Flat Caps") || m_coneCapStyle.isSelected("Flat Base Round Top")) {
      gluQuadricOrientation(quadric, GLU_INSIDE);
      gluDisk(quadric, 0.0, getSizeScale()*m_baseAndBaseRadius[i].w, m_cylinderSubdivisionAroundZ.get(), 1);
      gluQuadricOrientation(quadric, GLU_OUTSIDE);
    }

    if (m_coneCapStyle.isSelected("Round Caps") || m_coneCapStyle.isSelected("Flat Base Round Top")) {
      glTranslatef(0, 0, height);
      if (!m_sameColorForBaseAndTop)
        glColor4fv(glm::value_ptr(glm::vec4(m_coneTopColors[i].rgb(), m_coneTopColors[i].a * getOpacity())));
      gluSphere(quadric, getSizeScale()*m_axisAndTopRadius[i].w, m_cylinderSubdivisionAroundZ.get(), m_cylinderSubdivisionAroundZ.get());
    } else if (m_coneCapStyle.isSelected("Flat Caps") || m_coneCapStyle.isSelected("Round Base Flat Top")) {
      glTranslatef(0, 0, height);
      gluDisk(quadric, 0.0, getSizeScale()*m_axisAndTopRadius[i].w, m_cylinderSubdivisionAroundZ.get(), 1);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  gluDeleteQuadric(quadric);
}

void Z3DConeRenderer::renderPickingUsingOpengl()
{
  if (m_baseAndBaseRadius.empty())
    return;
  if (m_conePickingColors.empty() || m_conePickingColors.size() != m_baseAndBaseRadius.size())
    return;

  GLUquadricObj* quadric = gluNewQuadric();
  int dup = m_useConeShader2 ? 4 : 8;
  for (size_t i=0; i<m_baseAndBaseRadius.size(); i+=dup) {
    glColor4fv(glm::value_ptr(m_conePickingColors[i]));
    glm::vec3 bottomPos = m_baseAndBaseRadius[i].xyz();
    glm::vec3 topPos = m_axisAndTopRadius[i].xyz();
    topPos += bottomPos;
    bottomPos *= getCoordScales();
    topPos *= getCoordScales();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glm::vec3 C = topPos - bottomPos;
    float height = glm::length(C);
    glm::vec3 A, B;
    C = glm::normalize(C);
    glm::getOrthogonalVectors(C, A, B);
    glm::mat4 m(glm::vec4(A, 0.f),
                glm::vec4(B, 0.f),
                glm::vec4(C, 0.f),
                glm::vec4(bottomPos, 1.f));
    glMultMatrixf(&m[0][0]);

    gluCylinder(quadric, getSizeScale()*m_baseAndBaseRadius[i].w, getSizeScale()*m_axisAndTopRadius[i].w, height,
                m_cylinderSubdivisionAroundZ.get(), m_cylinderSubdivisionAlongZ.get());

    if (m_coneCapStyle.isSelected("Round Caps") || m_coneCapStyle.isSelected("Round Base Flat Top")) {
      gluSphere(quadric, getSizeScale()*m_baseAndBaseRadius[i].w, 12, 12);
    } else if (m_coneCapStyle.isSelected("Flat Caps") || m_coneCapStyle.isSelected("Flat Base Round Top")) {
      gluQuadricOrientation(quadric, GLU_INSIDE);
      gluDisk(quadric, 0.0, getSizeScale()*m_baseAndBaseRadius[i].w, 12, 1);
      gluQuadricOrientation(quadric, GLU_OUTSIDE);
    }

    if (m_coneCapStyle.isSelected("Round Caps") || m_coneCapStyle.isSelected("Flat Base Round Top")) {
      glTranslatef(0, 0, height);
      gluSphere(quadric, getSizeScale()*m_axisAndTopRadius[i].w, 12, 12);
    } else if (m_coneCapStyle.isSelected("Flat Caps") || m_coneCapStyle.isSelected("Round Base Flat Top")) {
      glTranslatef(0, 0, height);
      gluDisk(quadric, 0.0, getSizeScale()*m_axisAndTopRadius[i].w, 12, 1);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  gluDeleteQuadric(quadric);
}

void Z3DConeRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_baseAndBaseRadius.empty())
    return;
  appendDefaultColors();

  m_coneShaderGrp.bind();
  Z3DShaderProgram &shader = m_coneShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(shader, eye);

  shader.setUniformValue("lighting_enabled", m_needLighting);
  shader.setUniformValue("pos_scale", getCoordScales());

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      glBindVertexArray(m_VAO);
      // set vertex data
      GLint attr_origin = shader.attributeLocation("attr_origin");
      GLint attr_axis = shader.attributeLocation("attr_axis");
      GLint attr_flags = shader.attributeLocation("attr_flags");
      GLint attr_colors = shader.attributeLocation("attr_colors");
      GLint attr_colors2 = shader.attributeLocation("attr_colors2");

      glEnableVertexAttribArray(attr_origin);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
      glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_baseAndBaseRadius[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_origin, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_axis);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
      glBufferData(GL_ARRAY_BUFFER, m_axisAndTopRadius.size()*4*sizeof(GLfloat), &(m_axisAndTopRadius[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_axis, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_flags);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[2]);
      glBufferData(GL_ARRAY_BUFFER, m_allFlags.size()*sizeof(GLfloat), &(m_allFlags[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_colors);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[3]);
      glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_coneBaseColors[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_colors, 4, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_sameColorForBaseAndTop) {
        glEnableVertexAttribArray(attr_colors2);
        glVertexAttribPointer(attr_colors2, 4, GL_FLOAT, GL_FALSE, 0, 0);
      } else {
        glEnableVertexAttribArray(attr_colors2);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[4]);
        glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_coneTopColors[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_colors2, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[5]);
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
    GLint attr_origin = shader.attributeLocation("attr_origin");
    GLint attr_axis = shader.attributeLocation("attr_axis");
    GLint attr_flags = shader.attributeLocation("attr_flags");
    GLint attr_colors = shader.attributeLocation("attr_colors");
    GLint attr_colors2 = shader.attributeLocation("attr_colors2");

    glEnableVertexAttribArray(attr_origin);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_baseAndBaseRadius[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_origin, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_axis);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_axisAndTopRadius.size()*4*sizeof(GLfloat), &(m_axisAndTopRadius[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_axis, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_flags);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[2]);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_allFlags.size()*sizeof(GLfloat), &(m_allFlags[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_colors);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[3]);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_coneBaseColors[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_colors, 4, GL_FLOAT, GL_FALSE, 0, 0);

    if (m_sameColorForBaseAndTop) {
      glEnableVertexAttribArray(attr_colors2);
      glVertexAttribPointer(attr_colors2, 4, GL_FLOAT, GL_FALSE, 0, 0);
    } else {
      glEnableVertexAttribArray(attr_colors2);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[4]);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_coneTopColors[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_colors2, 4, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[5]);
    if (m_dataChanged)
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexs.size()*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, m_indexs.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(attr_origin);
    glDisableVertexAttribArray(attr_axis);
    glDisableVertexAttribArray(attr_flags);
    glDisableVertexAttribArray(attr_colors);
    glDisableVertexAttribArray(attr_colors2);

    m_dataChanged = false;
  }

  m_coneShaderGrp.release();
}

void Z3DConeRenderer::renderPickingUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_baseAndBaseRadius.empty())
    return;

  if (m_conePickingColors.empty() || m_conePickingColors.size() != m_baseAndBaseRadius.size())
    return;

  m_coneShaderGrp.bind();
  Z3DShaderProgram &shader = m_coneShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(shader, eye);

  shader.setUniformValue("lighting_enabled", false);
  shader.setUniformValue("pos_scale", getCoordScales());

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      glBindVertexArray(m_pickingVAO);
      // set vertex data
      GLint attr_origin = shader.attributeLocation("attr_origin");
      GLint attr_axis = shader.attributeLocation("attr_axis");
      GLint attr_flags = shader.attributeLocation("attr_flags");
      GLint attr_colors = shader.attributeLocation("attr_colors");
      GLint attr_colors2 = shader.attributeLocation("attr_colors2");

      glEnableVertexAttribArray(attr_origin);
      if (m_dataChanged) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[0]);
        glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_baseAndBaseRadius[0]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
      }
      glVertexAttribPointer(attr_origin, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_axis);
      if (m_dataChanged) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[1]);
        glBufferData(GL_ARRAY_BUFFER, m_axisAndTopRadius.size()*4*sizeof(GLfloat), &(m_axisAndTopRadius[0]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
      }
      glVertexAttribPointer(attr_axis, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_flags);
      if (m_dataChanged) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[2]);
        glBufferData(GL_ARRAY_BUFFER, m_allFlags.size()*sizeof(GLfloat), &(m_allFlags[0]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[2]);
      }
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_colors);
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[3]);
      glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_conePickingColors[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_colors, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_colors2);
      glVertexAttribPointer(attr_colors2, 4, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_dataChanged) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pickingVBOs[4]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexs.size()*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[5]);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      m_pickingDataChanged = false;
    }

    glBindVertexArray(m_pickingVAO);
    glDrawElements(GL_TRIANGLES, m_indexs.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

  } else {
    // set vertex data
    GLint attr_origin = shader.attributeLocation("attr_origin");
    GLint attr_axis = shader.attributeLocation("attr_axis");
    GLint attr_flags = shader.attributeLocation("attr_flags");
    GLint attr_colors = shader.attributeLocation("attr_colors");
    GLint attr_colors2 = shader.attributeLocation("attr_colors2");

    glEnableVertexAttribArray(attr_origin);
    if (m_dataChanged) {
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[0]);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_baseAndBaseRadius[0]), GL_STATIC_DRAW);
    } else {
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
    }
    glVertexAttribPointer(attr_origin, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_axis);
    if (m_dataChanged) {
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[1]);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, m_axisAndTopRadius.size()*4*sizeof(GLfloat), &(m_axisAndTopRadius[0]), GL_STATIC_DRAW);
    } else {
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
    }
    glVertexAttribPointer(attr_axis, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_flags);
    if (m_dataChanged) {
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[2]);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, m_allFlags.size()*sizeof(GLfloat), &(m_allFlags[0]), GL_STATIC_DRAW);
    } else {
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[2]);
    }
    glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_colors);
    glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[3]);
    if (m_pickingDataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_baseAndBaseRadius.size()*4*sizeof(GLfloat), &(m_conePickingColors[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_colors, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_colors2);
    glVertexAttribPointer(attr_colors2, 4, GL_FLOAT, GL_FALSE, 0, 0);

    if (m_dataChanged) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pickingVBOs[4]);
      if (m_pickingDataChanged)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexs.size()*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);
    } else {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[5]);
    }

    glDrawElements(GL_TRIANGLES, m_indexs.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(attr_origin);
    glDisableVertexAttribArray(attr_axis);
    glDisableVertexAttribArray(attr_flags);
    glDisableVertexAttribArray(attr_colors);
    glDisableVertexAttribArray(attr_colors2);

    m_pickingDataChanged = false;
  }

  m_coneShaderGrp.release();
}

void Z3DConeRenderer::appendDefaultColors()
{
  if (m_coneBaseColors.size() < m_baseAndBaseRadius.size()) {
    if (m_sameColorForBaseAndTop) {
      for (size_t i=m_coneBaseColors.size(); i<m_baseAndBaseRadius.size(); i++)
        m_coneBaseColors.push_back(glm::vec4(0.f, 0.f, 0.f, 1.f));
    } else {
      for (size_t i=m_coneBaseColors.size(); i<m_baseAndBaseRadius.size(); i++) {
        m_coneBaseColors.push_back(glm::vec4(0.f, 0.f, 0.f, 1.f));
        m_coneTopColors.push_back(glm::vec4(0.f, 0.f, 0.f, 1.f));
      }
    }
  }
}
