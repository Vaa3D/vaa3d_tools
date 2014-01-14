#include "zglew.h"
#include "z3dbackgroundrenderer.h"
#include "z3dgpuinfo.h"

Z3DBackgroundRenderer::Z3DBackgroundRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_backgroundShaderGrp()
  , m_firstColor("First Color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
  , m_secondColor("Second Color", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f))
  , m_gradientOrientation("Gradient Orientation")
  , m_mode("mode")
  , m_VBO(0)
{
  m_firstColor.setStyle("COLOR");
  m_secondColor.setStyle("COLOR");
  m_mode.addOptions("Uniform", "Gradient");
  m_mode.select("Gradient");
  m_gradientOrientation.addOptions("LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop");
  m_gradientOrientation.select("BottomToTop");
  addParameter(m_firstColor);
  addParameter(m_secondColor);
  addParameter(m_mode);
  addParameter(m_gradientOrientation);
  connect(&m_mode, SIGNAL(valueChanged()), this, SLOT(adjustWidgets()));
  connect(&m_mode, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_gradientOrientation, SIGNAL(valueChanged()), this, SLOT(compile()));
}

void Z3DBackgroundRenderer::compile()
{
  m_backgroundShaderGrp.rebuild(generateHeader());
}

void Z3DBackgroundRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList allshaders;
  allshaders << "pass.vert" << "background_func.frag";
  QStringList normalShaders;
  normalShaders << "pass.vert" << "background.frag";
  m_backgroundShaderGrp.init(allshaders, generateHeader(), m_rendererBase, normalShaders);
  m_backgroundShaderGrp.addAllSupportedPostShaders();

  glGenBuffers(1, &m_VBO);

  if (m_hardwareSupportVAO) {
    glBindVertexArray(m_VAO);
    GLfloat vertices[] = {-1.f, 1.f, 1.0f-1e-5, //top left corner
                          -1.f, -1.f, 1.0f-1e-5, //bottom left corner
                          1.f, 1.f, 1.0f-1e-5, //top right corner
                          1.f, -1.f, 1.0f-1e-5}; // bottom right rocner
    GLint attr_vertex = m_backgroundShaderGrp.get().attributeLocation("attr_vertex");

    glEnableVertexAttribArray(attr_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 3*4*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
  }
}

void Z3DBackgroundRenderer::deinitialize()
{
  glDeleteBuffers(1, &m_VBO);

  m_backgroundShaderGrp.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DBackgroundRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();
  if (m_mode.get() == "Uniform") {
    headerSource += "#define UNIFORM\n";
  } else {
    if (m_gradientOrientation.isSelected("LeftToRight"))
      headerSource += "#define GRADIENT_LEFT_TO_RIGHT\n";
    else if (m_gradientOrientation.isSelected("RightToLeft"))
      headerSource += "#define GRADIENT_RIGHT_TO_LEFT\n";
    else if (m_gradientOrientation.isSelected("TopToBottom"))
      headerSource += "#define GRADIENT_TOP_TO_BOTTOM\n";
    else if (m_gradientOrientation.isSelected("BottomToTop"))
      headerSource += "#define GRADIENT_BOTTOM_TO_TOP\n";
  }
  return headerSource;
}

void Z3DBackgroundRenderer::renderUsingOpengl()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if (m_mode.get() == "Gradient") {
    if (m_gradientOrientation.isSelected("LeftToRight"))
      glRotatef(270.f, 0.0f, 0.0f, 1.0f);
    else if (m_gradientOrientation.isSelected("RightToLeft"))
      glRotatef(90.f, 0.0f, 0.0f, 1.0f);
    else if (m_gradientOrientation.isSelected("TopToBottom"))
      glRotatef(180.f, 0.0f, 0.0f, 1.0f);
    else if (m_gradientOrientation.isSelected("BottomToTop"))
      glRotatef(0.f, 0.0f, 0.0f, 1.0f);
  }

  glBegin(GL_QUADS);
  glColor4fv(&m_firstColor.get()[0]);
  glVertex3f(-1.0, -1.0, 1.0-1e-5);
  glVertex3f( 1.0, -1.0, 1.0-1e-5);
  if (m_mode.get() == "Gradient")
    glColor4fv(&m_secondColor.get()[0]);
  glVertex3f( 1.0, 1.0, 1.0-1e-5);
  glVertex3f(-1.0, 1.0, 1.0-1e-5);
  glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  CHECK_GL_ERROR;
}

void Z3DBackgroundRenderer::renderPickingUsingOpengl()
{
  // do nothing
}

void Z3DBackgroundRenderer::renderUsingGLSL(Z3DEye eye)
{
  m_backgroundShaderGrp.bind();
  Z3DShaderProgram &shader = m_backgroundShaderGrp.get();
  m_rendererBase->setGlobalShaderParameters(shader, eye);

  shader.setUniformValue("color1", m_firstColor.get());
  if (m_mode.get() != "Uniform")
    shader.setUniformValue("color2", m_secondColor.get());

  if (m_hardwareSupportVAO) {
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
  } else {
    GLfloat vertices[] = {-1.f, 1.f, 1.0f-1e-5, //top left corner
                          -1.f, -1.f, 1.0f-1e-5, //bottom left corner
                          1.f, 1.f, 1.0f-1e-5, //top right corner
                          1.f, -1.f, 1.0f-1e-5}; // bottom right rocner
    GLint attr_vertex = shader.attributeLocation("attr_vertex");

    glEnableVertexAttribArray(attr_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 3*4*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(attr_vertex);
  }

  m_backgroundShaderGrp.release();
}

void Z3DBackgroundRenderer::renderPickingUsingGLSL(Z3DEye)
{
  // do nothing
}

void Z3DBackgroundRenderer::adjustWidgets()
{
  if (m_mode.get() == "Gradient") {
    m_firstColor.setName("First Color");
    m_firstColor.setVisible(true);
    m_secondColor.setVisible(true);
    m_gradientOrientation.setVisible(true);
  } else if (m_mode.get() == "Uniform") {
    m_firstColor.setName("Color");
    m_firstColor.setVisible(true);
    m_secondColor.setVisible(false);
    m_gradientOrientation.setVisible(false);
  }
}
