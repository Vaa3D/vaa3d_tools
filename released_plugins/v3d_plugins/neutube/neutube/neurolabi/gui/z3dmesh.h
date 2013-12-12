#ifndef ZMESH_H
#define ZMESH_H

#include <QStringList>
#include <vector>
#include "z3dgl.h"
#include "zglmutils.h"
#include "z3dshaderprogram.h"
#include "zstackdrawable.h"

class Z3DTriangleList : public ZStackDrawable
{
public:
  // one of GL_TRIANGLES, GL_TRIANGLE_STRIP and GL_TRIANGLE_FAN
  explicit Z3DTriangleList(GLenum type = GL_TRIANGLES);
  virtual ~Z3DTriangleList();

  void setTriangleType(GLenum type);

  virtual const std::string& className() const;
  virtual void display(QPainter &, int, Display_Style) const {}

  std::vector<double> getBoundBox() const;

  void setVertices(const std::vector<glm::vec3> &vertices) { m_vertices = vertices; }
  void setVertices(const std::vector<glm::dvec3> &vertices);
  void setTextureCoordinates(const std::vector<float> &textureCoordinates) { m_1DTextureCoordinates = textureCoordinates; }
  void setTextureCoordinates(const std::vector<glm::vec2> &textureCoordinates) { m_2DTextureCoordinates = textureCoordinates; }
  void setTextureCoordinates(const std::vector<glm::vec3> &textureCoordinates) { m_3DTextureCoordinates = textureCoordinates; }
  void setNormals(const std::vector<glm::vec3> &normals) { m_normals = normals; }
  void setColors(const std::vector<glm::vec4> &colors) { m_colors = colors; }
  void setIndices(const std::vector<GLuint> &indices) { m_indices = indices; }

  std::vector<glm::dvec3> getDoubleVertices() const;

  const std::vector<glm::vec3>& getVertices() const { return m_vertices; }
  const std::vector<float>& get1DTextureCoordinates() const { return m_1DTextureCoordinates; }
  const std::vector<glm::vec2>& get2DTextureCoordinates() const { return m_2DTextureCoordinates; }
  const std::vector<glm::vec3>& get3DTextureCoordinates() const { return m_3DTextureCoordinates; }
  const std::vector<glm::vec3>& getNormals() const { return m_normals; }
  const std::vector<glm::vec4>& getColors() const { return m_colors; }
  const std::vector<GLuint>& getIndices() const { return m_indices; }
  GLenum getTriangleListType() const { return m_type; }
  QString getTriangleListTypeAsString() const;
  bool hasIndices() const { return !m_indices.empty(); }

  // use ref to interpolate texture coordinate and colors. all vertices should be on ref surface
  void interpolate(const Z3DTriangleList &ref);

  // return true if no vertex
  bool empty() const;

  void clear();

  size_t getNumVertices() const { return m_vertices.size(); }
  size_t getNumTriangles() const;
  std::vector<glm::vec3> getTriangleVertices(size_t index) const;
  std::vector<glm::svec3> getTriangleIndices() const;
  glm::svec3 getTriangleIndices(size_t index) const;
  glm::vec3 getTriangleVertex(size_t triangleIndex, size_t vertexIndex) const;
  size_t getNumColors() const { return m_colors.size(); }
  size_t getNumNormals() const { return m_normals.size(); }
  size_t getNum1DTextureCoordinates() const { return m_1DTextureCoordinates.size(); }
  size_t getNum2DTextureCoordinates() const { return m_2DTextureCoordinates.size(); }
  size_t getNum3DTextureCoordinates() const { return m_3DTextureCoordinates.size(); }

  void transformVerticesByMatrix(const glm::mat4 &tfmat);

  std::vector<Z3DTriangleList*> split(size_t numTriangle = 100000) const;

  void generateNormals(bool useAreaWeight = true);

  // a cube with six surfaces
  static Z3DTriangleList createCube(
      glm::vec3 coordLlf = glm::vec3(0.f, 0.f, 0.f),
      glm::vec3 coordUrb = glm::vec3(1.f, 1.f, 1.f),
      glm::vec3 texLlf = glm::vec3(0.f, 0.f, 0.f),
      glm::vec3 texUrb = glm::vec3(1.f, 1.f, 1.f));

  // one slice from a cube, it is a x slice if alongDim == 0, a y slice if alongDim == 1,
  // a z slice if alongDim == 2
  static Z3DTriangleList createCubeSlice(
      float coordIn3rdDim,
      float texCoordIn3rdDim,
      int alongDim = 2,     // 0, 1, or 2
      glm::vec2 coordlow = glm::vec2(0.f, 0.f),
      glm::vec2 coordhigh = glm::vec2(1.f, 1.f),
      glm::vec2 texlow = glm::vec2(0.f, 0.f),
      glm::vec2 texhigh = glm::vec2(1.f, 1.f));

  // one slice from a cube, it is a x slice if alongDim == 0, a y slice if alongDim == 1,
  // a z slice if alongDim == 2
  static Z3DTriangleList createCubeSliceWith2DTexture(
      float coordIn3rdDim,
      int alongDim = 2,     // 0, 1, or 2
      glm::vec2 coordlow = glm::vec2(0.f, 0.f),
      glm::vec2 coordhigh = glm::vec2(1.f, 1.f),
      glm::vec2 texlow = glm::vec2(0.f, 0.f),
      glm::vec2 texhigh = glm::vec2(1.f, 1.f));

  // a 2d image quad with 2d texture coordinates
  static Z3DTriangleList createImageSlice(
      float coordIn3rdDim,
      glm::vec2 coordlow = glm::vec2(0.f, 0.f),
      glm::vec2 coordhigh = glm::vec2(1.f, 1.f),
      glm::vec2 texlow = glm::vec2(0.f, 0.f),
      glm::vec2 texhigh = glm::vec2(1.f, 1.f));

  // create a serie of slices from a cube, slices are cut along a specified dimension
  // if number of slices if 1, first slice will be returned (use first coordinate)
  // if number of slices is 2, first and last slices will be returned (use first and last coordinate)
  // other slices are interpolated between first and last slice
  // assume that the last slice is nearest to camera, then created triangles will face camera if first
  // coordinate is smaller than last coordinatesin the two fixed dimensions.
  // in cut dimension, last coordinate can be smaller than first coordinate to create inverse order series
  static Z3DTriangleList createCubeSerieSlices(
      int numSlices,
      int alongDim = 2,     // 0, 1, or 2
      glm::vec3 coordfirst = glm::vec3(0.f, 0.f, 0.f),
      glm::vec3 coordlast = glm::vec3(1.f, 1.f, 1.f),
      glm::vec3 texfirst = glm::vec3(0.f, 0.f, 0.f),
      glm::vec3 texlast = glm::vec3(1.f, 1.f, 1.f));

private:
  void appendTriangle(const Z3DTriangleList &mesh, glm::svec3 triangle);

private:
  GLenum m_type;

  std::vector<glm::vec3> m_vertices;
  std::vector<float> m_1DTextureCoordinates;
  std::vector<glm::vec2> m_2DTextureCoordinates;
  std::vector<glm::vec3> m_3DTextureCoordinates;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec4> m_colors;
  std::vector<GLuint> m_indices;
  QString m_source;
};

#endif // ZMESH_H
