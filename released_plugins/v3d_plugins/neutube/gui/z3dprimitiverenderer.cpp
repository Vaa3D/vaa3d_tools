#include "z3dprimitiverenderer.h"

#include "z3dmesh.h"
#include "z3dgpuinfo.h"

Z3DPrimitiveRenderer::Z3DPrimitiveRenderer(QObject *parent)
  : QObject(parent)
  , m_rendererBase(NULL)
  , m_initialized(false)
  , m_needLighting(false)
  , m_useDisplayList(false)
  , m_respectRendererBaseCoordScale(true)
  , m_hardwareSupportVAO(Z3DGpuInfoInstance.isVAOSupported())
  , m_VAO(0)
  , m_pickingVAO(0)
  , m_privateVAO(0)
{
}

Z3DPrimitiveRenderer::~Z3DPrimitiveRenderer()
{
}

void Z3DPrimitiveRenderer::initialize()
{
  if (m_initialized) {
    LERROR() << getClassName() << "already initialized";
    return;
  }
  if (m_hardwareSupportVAO) {
    glGenVertexArrays(1, &m_VAO);
    glGenVertexArrays(1, &m_pickingVAO);
    glGenVertexArrays(1, &m_privateVAO);
  }
}

void Z3DPrimitiveRenderer::deinitialize()
{
  if (!m_initialized) {
    LERROR() << getClassName() << "not initialized";
    return;
  }
  if (m_hardwareSupportVAO) {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteVertexArrays(1, &m_pickingVAO);
    glDeleteVertexArrays(1, &m_privateVAO);
  }
}

void Z3DPrimitiveRenderer::addParameter(ZParameter &para)
{
  addParameter(&para);
}

void Z3DPrimitiveRenderer::addParameter(ZParameter *para)
{
  m_parameters.push_back(para);
}

std::vector<ZParameter*> Z3DPrimitiveRenderer::getParameters()
{
  return m_parameters;
}

QString Z3DPrimitiveRenderer::generateHeader()
{
  return m_rendererBase->generateHeader();
}

void Z3DPrimitiveRenderer::renderScreenQuad(const Z3DShaderProgram &shader, bool depthAlwaysPass)
{
  if (!shader.isLinked())
    return;

  if (depthAlwaysPass)
    glDepthFunc(GL_ALWAYS);

  if (m_hardwareSupportVAO) {
    glBindVertexArray(m_privateVAO);
  }

  GLfloat vertices[] = {-1.f, 1.f, 0.f, //top left corner
                        -1.f, -1.f, 0.f, //bottom left corner
                        1.f, 1.f, 0.f, //top right corner
                        1.f, -1.f, 0.f}; // bottom right rocner
  GLint attr_vertex = shader.attributeLocation("attr_vertex");

  GLuint bufObjects[1];
  glGenBuffers(1, bufObjects);

  glEnableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, 3*4*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, bufObjects);

  glDisableVertexAttribArray(attr_vertex);

  if (m_hardwareSupportVAO) {
    glBindVertexArray(0);
  }

  if (depthAlwaysPass)
    glDepthFunc(GL_LESS);
}

void Z3DPrimitiveRenderer::renderTriangleList(const Z3DShaderProgram &shader, const Z3DTriangleList &mesh)
{
  if (mesh.empty() || !shader.isLinked())
    return;

  const std::vector<glm::vec3>& vertices = mesh.getVertices();
  const std::vector<float>& textureCoordinates1D = mesh.get1DTextureCoordinates();
  const std::vector<glm::vec2>& textureCoordinates2D = mesh.get2DTextureCoordinates();
  const std::vector<glm::vec3>& textureCoordinates3D = mesh.get3DTextureCoordinates();
  const std::vector<glm::vec3>& normals = mesh.getNormals();
  const std::vector<glm::vec4>& colors = mesh.getColors();
  const std::vector<GLuint>& triangleIndexes = mesh.getIndices();
  GLenum type = mesh.getTriangleListType();

  if (m_hardwareSupportVAO) {
    glBindVertexArray(m_privateVAO);
  }

  GLint attr_vertex = shader.attributeLocation("attr_vertex");
  GLint attr_1dTexCoord0 = shader.attributeLocation("attr_1dTexCoord0");
  GLint attr_2dTexCoord0 = shader.attributeLocation("attr_2dTexCoord0");
  GLint attr_3dTexCoord0 = shader.attributeLocation("attr_3dTexCoord0");
  GLint attr_normal = shader.attributeLocation("attr_normal");
  GLint attr_color = shader.attributeLocation("attr_color");

  GLsizei bufObjectsSize = 1;  // vertex
  if (attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty())
    bufObjectsSize++;
  if (attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty())
    bufObjectsSize++;
  if (attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty())
    bufObjectsSize++;
  if (attr_normal != -1 && !normals.empty())
    bufObjectsSize++;
  if (attr_color != -1 && !colors.empty())
    bufObjectsSize++;
  if (!triangleIndexes.empty())
    bufObjectsSize++;

  GLuint *bufObjects = new GLuint[bufObjectsSize];
  glGenBuffers(bufObjectsSize, bufObjects);

  int bufIdx = 0;
  glEnableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat), &(vertices[0]), GL_STATIC_DRAW);
  glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

  if (attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty()) {
    glEnableVertexAttribArray(attr_1dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordinates1D.size()*1*sizeof(GLfloat), &(textureCoordinates1D[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_1dTexCoord0, 1, GL_FLOAT, GL_FALSE, 0, 0);
  }

  if (attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty()) {
    glEnableVertexAttribArray(attr_2dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordinates2D.size()*2*sizeof(GLfloat), &(textureCoordinates2D[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_2dTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  }

  if (attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty()) {
    glEnableVertexAttribArray(attr_3dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordinates3D.size()*3*sizeof(GLfloat), &(textureCoordinates3D[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  }

  if (attr_normal != -1 && !normals.empty()) {
    glEnableVertexAttribArray(attr_normal);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*3*sizeof(GLfloat), &(normals[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
  }

  if (attr_color != -1 && !colors.empty()) {
    glEnableVertexAttribArray(attr_color);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, colors.size()*4*sizeof(GLfloat), &(colors[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
  }

  if (triangleIndexes.empty()) {
    glDrawArrays(type, 0, vertices.size());
  } else {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size()*sizeof(GLuint), &(triangleIndexes[0]), GL_STATIC_DRAW);
    glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(bufObjectsSize, bufObjects);
  delete bufObjects;

  glDisableVertexAttribArray(attr_vertex);
  if (attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty())
    glDisableVertexAttribArray(attr_1dTexCoord0);
  if (attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty())
    glDisableVertexAttribArray(attr_2dTexCoord0);
  if (attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty())
    glDisableVertexAttribArray(attr_3dTexCoord0);
  if (attr_normal != -1 && !normals.empty())
    glDisableVertexAttribArray(attr_normal);
  if (attr_color != -1 && !colors.empty())
    glDisableVertexAttribArray(attr_color);

  if (m_hardwareSupportVAO) {
    glBindVertexArray(0);
  }
}

void Z3DPrimitiveRenderer::invalidateOpenglRenderer()
{
  if (m_useDisplayList)
    emit openglRendererInvalid();
}

void Z3DPrimitiveRenderer::invalidateOpenglPickingRenderer()
{
  if (m_useDisplayList)
    emit openglPickingRendererInvalid();
}

void Z3DPrimitiveRenderer::coordScalesChanged()
{
}
