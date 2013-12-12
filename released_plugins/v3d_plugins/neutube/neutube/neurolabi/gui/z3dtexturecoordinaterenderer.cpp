#include "z3dtexturecoordinaterenderer.h"

#include "z3dmesh.h"

Z3DTextureCoordinateRenderer::Z3DTextureCoordinateRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_mesh(NULL)
  , m_VBOs(3)
  , m_dataChanged(false)
{
}

void Z3DTextureCoordinateRenderer::compile()
{
  m_renderTextureCoordinateShader.setHeaderAndRebuild(generateHeader());
}

void Z3DTextureCoordinateRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  m_renderTextureCoordinateShader.loadFromSourceFile("transform_with_3dtexture.vert",
                                                     "render_3dtexture_coordinate.frag", generateHeader());
  m_renderTextureCoordinateShader.bindFragDataLocation(0, "FragData0");

  glGenBuffers(m_VBOs.size(), &m_VBOs[0]);
}

void Z3DTextureCoordinateRenderer::deinitialize()
{
  glDeleteBuffers(m_VBOs.size(), &m_VBOs[0]);

  m_renderTextureCoordinateShader.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

void Z3DTextureCoordinateRenderer::renderUsingOpengl()
{
  //renderUsingGLSL();
}

void Z3DTextureCoordinateRenderer::renderPickingUsingOpengl()
{
}

void Z3DTextureCoordinateRenderer::renderUsingGLSL(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_mesh == NULL || m_mesh->getVertices().empty() ||
      m_mesh->getVertices().size() != m_mesh->get3DTextureCoordinates().size())
    return;

  const std::vector<glm::vec3> &vertices = m_mesh->getVertices();
  const std::vector<glm::vec3> &texCoords = m_mesh->get3DTextureCoordinates();
  const std::vector<GLuint> &triangleIndexes = m_mesh->getIndices();

  m_renderTextureCoordinateShader.bind();
  m_rendererBase->setGlobalShaderParameters(m_renderTextureCoordinateShader, eye);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      glBindVertexArray(m_VAO);
      GLint attr_vertex = m_renderTextureCoordinateShader.attributeLocation("attr_vertex");
      GLint attr_3dTexCoord0 = m_renderTextureCoordinateShader.attributeLocation("attr_3dTexCoord0");

      int bufIdx = 0;
      glEnableVertexAttribArray(attr_vertex);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[bufIdx++]);
      glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat), &(vertices[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_3dTexCoord0);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[bufIdx++]);
      glBufferData(GL_ARRAY_BUFFER, texCoords.size()*3*sizeof(GLfloat), &(texCoords[0]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      if (!triangleIndexes.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[bufIdx++]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size()*sizeof(GLuint), &(triangleIndexes[0]), GL_STATIC_DRAW);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      m_dataChanged = false;
    }

    glBindVertexArray(m_VAO);
    if (triangleIndexes.empty()) {
      glDrawArrays(m_mesh->getTriangleListType(), 0, vertices.size());
    } else {
      glDrawElements(m_mesh->getTriangleListType(), triangleIndexes.size(), GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);

  } else {
    GLint attr_vertex = m_renderTextureCoordinateShader.attributeLocation("attr_vertex");
    GLint attr_3dTexCoord0 = m_renderTextureCoordinateShader.attributeLocation("attr_3dTexCoord0");

    int bufIdx = 0;
    glEnableVertexAttribArray(attr_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat), &(vertices[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_3dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size()*3*sizeof(GLfloat), &(texCoords[0]), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if (triangleIndexes.empty()) {
      glDrawArrays(m_mesh->getTriangleListType(), 0, vertices.size());
    } else {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[bufIdx++]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size()*sizeof(GLuint), &(triangleIndexes[0]), GL_STATIC_DRAW);
      glDrawElements(m_mesh->getTriangleListType(), triangleIndexes.size(), GL_UNSIGNED_INT, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(attr_vertex);
    glDisableVertexAttribArray(attr_3dTexCoord0);
  }

  m_renderTextureCoordinateShader.release();
}

void Z3DTextureCoordinateRenderer::renderPickingUsingGLSL(Z3DEye)
{
}
