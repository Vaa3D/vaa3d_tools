#include "z3dmesh.h"

#include <fstream>
#include <cassert>
#include <list>
#include "z3dutils.h"
#include <algorithm>
#include "zswctree.h"
#include "zbenchtimer.h"

Z3DTriangleList::Z3DTriangleList(GLenum type)
{
  setTriangleType(type);
}

Z3DTriangleList::~Z3DTriangleList()
{
}

void Z3DTriangleList::setTriangleType(GLenum type)
{
  m_type = type;
  if (m_type != GL_TRIANGLES && m_type != GL_TRIANGLE_FAN && m_type != GL_TRIANGLE_STRIP) {
    LERROR() << "Wrong Triangle type, should be one of GL_TRIANGLES, GL_TRIANGLE_STRIP and GL_TRIANGLE_FAN. Set to GL_TRIANGLES.";
    m_type = GL_TRIANGLES;
  }
}

std::vector<double> Z3DTriangleList::getBoundBox() const
{
  std::vector<double> result(6);
  result[0] = result[2] = result[4] = std::numeric_limits<double>::max();
  result[1] = result[3] = result[5] = -std::numeric_limits<double>::max();
  for (size_t i=0; i<m_vertices.size(); ++i) {
    result[0] = std::min(result[0], static_cast<double>(m_vertices[i].x));
    result[1] = std::max(result[1], static_cast<double>(m_vertices[i].x));
    result[2] = std::min(result[2], static_cast<double>(m_vertices[i].y));
    result[3] = std::max(result[3], static_cast<double>(m_vertices[i].y));
    result[4] = std::min(result[4], static_cast<double>(m_vertices[i].z));
    result[5] = std::max(result[5], static_cast<double>(m_vertices[i].z));
  }
  return result;
}

void Z3DTriangleList::setVertices(const std::vector<glm::dvec3> &vertices)
{
  for (size_t i=0; i<vertices.size(); i++) {
    glm::vec3 vert = glm::vec3(vertices[i]);
    m_vertices.push_back(vert);
  }
}

std::vector<glm::dvec3> Z3DTriangleList::getDoubleVertices() const
{
  std::vector<glm::dvec3> result;
  for (size_t i=0; i<m_vertices.size(); i++)
    result.push_back(glm::dvec3(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z));
  return result;
}

std::vector<glm::uvec3> Z3DTriangleList::getTriangleIndices() const
{
  std::vector<glm::uvec3> result;
  if (m_indices.empty()) {
    if (m_type == GL_TRIANGLES) {
      for (size_t i=0; i<m_vertices.size()-2; i+=3) {
        result.push_back(glm::uvec3(i,i+1,i+2));
      }
    } else if (m_type == GL_TRIANGLE_STRIP) {
      for (size_t i=0; i<m_vertices.size()-2; ++i) {
        glm::uvec3 triangle;
        if (i%2 == 0) {
          triangle[0] = i;
          triangle[1] = i+1;
        } else {
          triangle[0] = i+1;
          triangle[1] = i;
        }
        triangle[2] = i+2;
        result.push_back(triangle);
      }
    } else /*(m_type == GL_TRIANGLE_FAN)*/ {
      for (size_t i=1; i<m_vertices.size()-1; ++i) {
        result.push_back(glm::uvec3(0,i,i+1));
      }
    }
  } else {
    if (m_type == GL_TRIANGLES) {
      for (size_t i=0; i<m_indices.size()-2; i+=3) {
        result.push_back(glm::uvec3(m_indices[i],m_indices[i+1],m_indices[i+2]));
      }
    } else if (m_type == GL_TRIANGLE_STRIP) {
      for (size_t i=0; i<m_indices.size()-2; ++i) {
        glm::uvec3 triangle;
        if (i%2 == 0) {
          triangle[0] = m_indices[i];
          triangle[1] = m_indices[i+1];
        } else {
          triangle[0] = m_indices[i+1];
          triangle[1] = m_indices[i];
        }
        triangle[2] = m_indices[i+2];
        result.push_back(triangle);
      }
    } else /*(m_type == GL_TRIANGLE_FAN)*/ {
      for (size_t i=1; i<m_indices.size()-1; ++i) {
        result.push_back(glm::uvec3(m_indices[0],m_indices[i],m_indices[i+1]));
      }
    }
  }
  return result;
}

QString Z3DTriangleList::getTriangleListTypeAsString() const
{
  if (m_type == GL_TRIANGLES)
    return "GL_TRIANGLES";
  else if (m_type == GL_TRIANGLE_STRIP)
    return "GL_TRIANGLE_STRIP";
  else if (m_type == GL_TRIANGLE_FAN)
    return "GL_TRIANGLE_FAN";
  else
    return "WrongType";
}

void Z3DTriangleList::interpolate(const Z3DTriangleList &ref)
{
  std::vector<glm::uvec3> triIdxs = ref.getTriangleIndices();
  if (!ref.m_1DTextureCoordinates.empty())
    m_1DTextureCoordinates.clear();
  if (!ref.m_2DTextureCoordinates.empty())
    m_2DTextureCoordinates.clear();
  if (!ref.m_3DTextureCoordinates.empty())
    m_3DTextureCoordinates.clear();
  if (!ref.m_colors.empty())
    m_colors.clear();
  for (size_t i=0; i<m_vertices.size(); i++) {
    bool match = false;
    // first check all ref vertices
    for (size_t j=0; !match && j<ref.m_vertices.size(); j++) {
      if (glm::length(ref.m_vertices[j] - m_vertices[i]) <= 1e-6) {
        match = true;
        if (!ref.m_1DTextureCoordinates.empty())
          m_1DTextureCoordinates.push_back(ref.m_1DTextureCoordinates[j]);
        if (!ref.m_2DTextureCoordinates.empty())
          m_2DTextureCoordinates.push_back(ref.m_2DTextureCoordinates[j]);
        if (!ref.m_3DTextureCoordinates.empty())
          m_3DTextureCoordinates.push_back(ref.m_3DTextureCoordinates[j]);
        if (!ref.m_colors.empty())
          m_colors.push_back(ref.m_colors[j]);
      }
    }
    // no vertice match, interpolate
    for (size_t j=0; !match && j<triIdxs.size(); j++) {
      glm::uvec3 triIdx = triIdxs[j];
      double s, t;
      if (Z3DUtils::vertexTriangleSquaredDistance(glm::dvec3(m_vertices[i]), glm::dvec3(ref.m_vertices[triIdx[0]]),
                                                  glm::dvec3(ref.m_vertices[triIdx[1]]), glm::dvec3(ref.m_vertices[triIdx[2]]),
                                                  s, t)
          <= 1e-6) {
        match = true;
        float fs = static_cast<float>(s);
        float ft = static_cast<float>(t);
        if (!ref.m_1DTextureCoordinates.empty())
          m_1DTextureCoordinates.push_back(ref.m_1DTextureCoordinates[triIdx[0]] +
                                           (ref.m_1DTextureCoordinates[triIdx[1]] - ref.m_1DTextureCoordinates[triIdx[0]]) * fs +
                                           (ref.m_1DTextureCoordinates[triIdx[2]] - ref.m_1DTextureCoordinates[triIdx[0]]) * ft);
        if (!ref.m_2DTextureCoordinates.empty())
          m_2DTextureCoordinates.push_back(ref.m_2DTextureCoordinates[triIdx[0]] +
                                           (ref.m_2DTextureCoordinates[triIdx[1]] - ref.m_2DTextureCoordinates[triIdx[0]]) * fs +
                                           (ref.m_2DTextureCoordinates[triIdx[2]] - ref.m_2DTextureCoordinates[triIdx[0]]) * ft);
        if (!ref.m_3DTextureCoordinates.empty())
          m_3DTextureCoordinates.push_back(ref.m_3DTextureCoordinates[triIdx[0]] +
                                           (ref.m_3DTextureCoordinates[triIdx[1]] - ref.m_3DTextureCoordinates[triIdx[0]]) * fs +
                                           (ref.m_3DTextureCoordinates[triIdx[2]] - ref.m_3DTextureCoordinates[triIdx[0]]) * ft);
        if (!ref.m_colors.empty())
          m_colors.push_back(ref.m_colors[triIdx[0]] +
                             (ref.m_colors[triIdx[1]] - ref.m_colors[triIdx[0]]) * fs +
                             (ref.m_colors[triIdx[2]] - ref.m_colors[triIdx[0]]) * ft);
      }
    }
    if (!match) {
      LDEBUG() << "here";
      if (!ref.m_1DTextureCoordinates.empty())
        m_1DTextureCoordinates.push_back(0.0);
      if (!ref.m_2DTextureCoordinates.empty())
        m_2DTextureCoordinates.push_back(glm::vec2(0.0));
      if (!ref.m_3DTextureCoordinates.empty())
        m_3DTextureCoordinates.push_back(glm::vec3(0.0));
      if (!ref.m_colors.empty())
        m_colors.push_back(glm::vec4(0.0));
    }
  }
}

bool Z3DTriangleList::empty() const
{
  return m_vertices.empty();
}

void Z3DTriangleList::clear()
{
  m_vertices.clear();
  m_1DTextureCoordinates.clear();
  m_2DTextureCoordinates.clear();
  m_3DTextureCoordinates.clear();
  m_normals.clear();
  m_colors.clear();
  m_indices.clear();
  m_source.clear();
}

size_t Z3DTriangleList::getNumTriangles() const
{
  size_t n = 0;
  if (m_indices.empty())
    n = m_vertices.size();
  else
    n = m_indices.size();
  if (m_type == GL_TRIANGLES)
    return n/3;
  else if (m_type == GL_TRIANGLE_STRIP || m_type == GL_TRIANGLE_FAN)
    return n-2;
  else
    return 0;
}

std::vector<glm::vec3> Z3DTriangleList::getTriangleVertices(size_t index) const
{
  std::vector<glm::vec3> triangle;
  glm::uvec3 tIs = getTriangleIndices(index);
  triangle.push_back(m_vertices[tIs[0]]);
  triangle.push_back(m_vertices[tIs[1]]);
  triangle.push_back(m_vertices[tIs[2]]);
  return triangle;
}

glm::uvec3 Z3DTriangleList::getTriangleIndices(size_t index) const
{
  glm::uvec3 triangle;
  index = std::min(getNumTriangles()-1, index);
  if (m_indices.empty()) {
    if (m_type == GL_TRIANGLES) {
      triangle[0] = index*3;
      triangle[1] = index*3+1;
      triangle[2] = index*3+2;
    } else if (m_type == GL_TRIANGLE_STRIP) {
      if (index%2 == 0) {
        triangle[0] = index;
        triangle[1] = index+1;
      } else {
        triangle[0] = index+1;
        triangle[1] = index;
      }
      triangle[2] = index+2;
    } else if (m_type == GL_TRIANGLE_FAN) {
      triangle[0] = 0;
      triangle[1] = index+1;
      triangle[2] = index+2;
    }
  } else {
    if (m_type == GL_TRIANGLES) {
      triangle[0] = m_indices[index*3];
      triangle[1] = m_indices[index*3+1];
      triangle[2] = m_indices[index*3+2];
    } else if (m_type == GL_TRIANGLE_STRIP) {
      if (index%2 == 0) {
        triangle[0] = m_indices[index];
        triangle[1] = m_indices[index+1];
      } else {
        triangle[0] = m_indices[index+1];
        triangle[1] = m_indices[index];
      }
      triangle[2] = m_indices[index+2];
    } else if (m_type == GL_TRIANGLE_FAN) {
      triangle[0] = m_indices[0];
      triangle[1] = m_indices[index+1];
      triangle[2] = m_indices[index+2];
    }
  }
  return triangle;
}

glm::vec3 Z3DTriangleList::getTriangleVertex(size_t triangleIndex, size_t vertexIndex) const
{
  vertexIndex = std::min(static_cast<size_t>(2), vertexIndex);
  return getTriangleVertices(triangleIndex)[vertexIndex];
}

void Z3DTriangleList::transformVerticesByMatrix(const glm::mat4 &tfmat)
{
  if (tfmat == glm::mat4())
    return;
  for (size_t i=0; i<m_vertices.size(); ++i) {
    m_vertices[i] = glm::applyMatrix(tfmat, m_vertices[i]);
  }
}

std::vector<Z3DTriangleList*> Z3DTriangleList::split(size_t numTriangle) const
{
  size_t totalNumTri = getNumTriangles();
  size_t numResult = std::ceil(1.0*totalNumTri/numTriangle);
  std::vector<Z3DTriangleList*> res(numResult);
  for (size_t i=0; i<numResult; ++i) {
    res[i] = new Z3DTriangleList(GL_TRIANGLES);
    for (size_t j=i*numTriangle; j<std::min(totalNumTri, (i+1)*numTriangle); ++j) {
      res[i]->appendTriangle(*this, getTriangleIndices(j));
    }
  }
  return res;
}

void Z3DTriangleList::generateNormals(bool useAreaWeight)
{
  m_normals.resize(m_vertices.size());
  for (size_t i=0; i<m_normals.size(); ++i)
    m_normals[i] = glm::vec3(0.f);

  for (size_t i=0; i<getNumTriangles(); ++i) {
    glm::uvec3 tri = getTriangleIndices(i);
    // get the three vertices that make the faces
    glm::vec3 p1 = m_vertices[tri[0]];
    glm::vec3 p2 = m_vertices[tri[1]];
    glm::vec3 p3 = m_vertices[tri[2]];

    glm::vec3 v1 = p2 - p1;
    glm::vec3 v2 = p3 - p1;
    glm::vec3 normal = glm::cross(v1, v2);
    if (!useAreaWeight)
      normal = glm::normalize(normal);
    m_normals[tri[0]] += normal;
    m_normals[tri[1]] += normal;
    m_normals[tri[2]] += normal;
  }

  for (size_t i=0; i<m_normals.size(); ++i)
    m_normals[i] = glm::normalize(m_normals[i]);
}

Z3DTriangleList Z3DTriangleList::createCube(glm::vec3 coordLlf, glm::vec3 coordUrb, glm::vec3 texLlf, glm::vec3 texUrb)
{
  Z3DTriangleList cube(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> texCoords;
  vertices.push_back(glm::vec3(coordLlf[0], coordLlf[1], coordUrb[2]));
  vertices.push_back(glm::vec3(coordUrb[0], coordLlf[1], coordUrb[2]));
  vertices.push_back(glm::vec3(coordLlf[0], coordUrb[1], coordUrb[2]));
  vertices.push_back(glm::vec3(coordUrb[0], coordUrb[1], coordUrb[2]));
  vertices.push_back(glm::vec3(coordLlf[0], coordLlf[1], coordLlf[2]));
  vertices.push_back(glm::vec3(coordUrb[0], coordLlf[1], coordLlf[2]));
  vertices.push_back(glm::vec3(coordLlf[0], coordUrb[1], coordLlf[2]));
  vertices.push_back(glm::vec3(coordUrb[0], coordUrb[1], coordLlf[2]));

  texCoords.push_back(glm::vec3(texLlf[0], texLlf[1], texUrb[2]));
  texCoords.push_back(glm::vec3(texUrb[0], texLlf[1], texUrb[2]));
  texCoords.push_back(glm::vec3(texLlf[0], texUrb[1], texUrb[2]));
  texCoords.push_back(glm::vec3(texUrb[0], texUrb[1], texUrb[2]));
  texCoords.push_back(glm::vec3(texLlf[0], texLlf[1], texLlf[2]));
  texCoords.push_back(glm::vec3(texUrb[0], texLlf[1], texLlf[2]));
  texCoords.push_back(glm::vec3(texLlf[0], texUrb[1], texLlf[2]));
  texCoords.push_back(glm::vec3(texUrb[0], texUrb[1], texLlf[2]));

  GLuint idxes[14] = {0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1};
  std::vector<GLuint> indexes(idxes, idxes+14);
  cube.m_vertices.swap(vertices);
  cube.m_3DTextureCoordinates.swap(texCoords);
  cube.m_indices.swap(indexes);
  return cube;
}

Z3DTriangleList Z3DTriangleList::createCubeSlice(float coordIn3rdDim, float texCoordIn3rdDim, int alongDim,
                                                 glm::vec2 coordlow, glm::vec2 coordhigh,
                                                 glm::vec2 texlow, glm::vec2 texhigh)
{
  Z3DTriangleList quad(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> texCoords;
  if (alongDim == 0) {
    vertices.push_back(glm::vec3(coordIn3rdDim, coordlow[0], coordlow[1]));
    vertices.push_back(glm::vec3(coordIn3rdDim, coordhigh[0], coordlow[1]));
    vertices.push_back(glm::vec3(coordIn3rdDim, coordlow[0], coordhigh[1]));
    vertices.push_back(glm::vec3(coordIn3rdDim, coordhigh[0], coordhigh[1]));
    texCoords.push_back(glm::vec3(texCoordIn3rdDim, texlow[0], texlow[1]));
    texCoords.push_back(glm::vec3(texCoordIn3rdDim, texhigh[0], texlow[1]));
    texCoords.push_back(glm::vec3(texCoordIn3rdDim, texlow[0], texhigh[1]));
    texCoords.push_back(glm::vec3(texCoordIn3rdDim, texhigh[0], texhigh[1]));
  } else if (alongDim == 1) {
    vertices.push_back(glm::vec3(coordlow[0], coordIn3rdDim, coordlow[1]));
    vertices.push_back(glm::vec3(coordhigh[0], coordIn3rdDim, coordlow[1]));
    vertices.push_back(glm::vec3(coordlow[0], coordIn3rdDim, coordhigh[1]));
    vertices.push_back(glm::vec3(coordhigh[0], coordIn3rdDim, coordhigh[1]));
    texCoords.push_back(glm::vec3(texlow[0], texCoordIn3rdDim, texlow[1]));
    texCoords.push_back(glm::vec3(texhigh[0], texCoordIn3rdDim, texlow[1]));
    texCoords.push_back(glm::vec3(texlow[0], texCoordIn3rdDim, texhigh[1]));
    texCoords.push_back(glm::vec3(texhigh[0], texCoordIn3rdDim, texhigh[1]));
  } else if (alongDim == 2) {
    vertices.push_back(glm::vec3(coordlow[0], coordlow[1], coordIn3rdDim));
    vertices.push_back(glm::vec3(coordhigh[0], coordlow[1], coordIn3rdDim));
    vertices.push_back(glm::vec3(coordlow[0], coordhigh[1], coordIn3rdDim));
    vertices.push_back(glm::vec3(coordhigh[0], coordhigh[1], coordIn3rdDim));
    texCoords.push_back(glm::vec3(texlow[0], texlow[1], texCoordIn3rdDim));
    texCoords.push_back(glm::vec3(texhigh[0], texlow[1], texCoordIn3rdDim));
    texCoords.push_back(glm::vec3(texlow[0], texhigh[1], texCoordIn3rdDim));
    texCoords.push_back(glm::vec3(texhigh[0], texhigh[1], texCoordIn3rdDim));
  }

  quad.m_vertices.swap(vertices);
  quad.m_3DTextureCoordinates.swap(texCoords);
  return quad;
}

Z3DTriangleList Z3DTriangleList::createCubeSliceWith2DTexture(float coordIn3rdDim, int alongDim,
                                                              glm::vec2 coordlow, glm::vec2 coordhigh,
                                                              glm::vec2 texlow, glm::vec2 texhigh)
{
  Z3DTriangleList quad(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> texCoords;
  if (alongDim == 0) {
    vertices.push_back(glm::vec3(coordIn3rdDim, coordlow[0], coordlow[1]));
    vertices.push_back(glm::vec3(coordIn3rdDim, coordhigh[0], coordlow[1]));
    vertices.push_back(glm::vec3(coordIn3rdDim, coordlow[0], coordhigh[1]));
    vertices.push_back(glm::vec3(coordIn3rdDim, coordhigh[0], coordhigh[1]));
  } else if (alongDim == 1) {
    vertices.push_back(glm::vec3(coordlow[0], coordIn3rdDim, coordlow[1]));
    vertices.push_back(glm::vec3(coordhigh[0], coordIn3rdDim, coordlow[1]));
    vertices.push_back(glm::vec3(coordlow[0], coordIn3rdDim, coordhigh[1]));
    vertices.push_back(glm::vec3(coordhigh[0], coordIn3rdDim, coordhigh[1]));
  } else if (alongDim == 2) {
    vertices.push_back(glm::vec3(coordlow[0], coordlow[1], coordIn3rdDim));
    vertices.push_back(glm::vec3(coordhigh[0], coordlow[1], coordIn3rdDim));
    vertices.push_back(glm::vec3(coordlow[0], coordhigh[1], coordIn3rdDim));
    vertices.push_back(glm::vec3(coordhigh[0], coordhigh[1], coordIn3rdDim));
  }
  texCoords.push_back(glm::vec2(texlow[0], texlow[1]));
  texCoords.push_back(glm::vec2(texhigh[0], texlow[1]));
  texCoords.push_back(glm::vec2(texlow[0], texhigh[1]));
  texCoords.push_back(glm::vec2(texhigh[0], texhigh[1]));

  quad.m_vertices.swap(vertices);
  quad.m_2DTextureCoordinates.swap(texCoords);
  return quad;
}

Z3DTriangleList Z3DTriangleList::createImageSlice(float coordIn3rdDim, glm::vec2 coordlow,
                                                  glm::vec2 coordhigh, glm::vec2 texlow, glm::vec2 texhigh)
{
  Z3DTriangleList quad(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> texCoords;

  vertices.push_back(glm::vec3(coordlow[0], coordlow[1], coordIn3rdDim));
  vertices.push_back(glm::vec3(coordhigh[0], coordlow[1], coordIn3rdDim));
  vertices.push_back(glm::vec3(coordlow[0], coordhigh[1], coordIn3rdDim));
  vertices.push_back(glm::vec3(coordhigh[0], coordhigh[1], coordIn3rdDim));
  texCoords.push_back(glm::vec2(texlow[0], texlow[1]));
  texCoords.push_back(glm::vec2(texhigh[0], texlow[1]));
  texCoords.push_back(glm::vec2(texlow[0], texhigh[1]));
  texCoords.push_back(glm::vec2(texhigh[0], texhigh[1]));

  quad.m_vertices.swap(vertices);
  quad.m_2DTextureCoordinates.swap(texCoords);
  return quad;
}

Z3DTriangleList Z3DTriangleList::createCubeSerieSlices(int numSlices, int alongDim, glm::vec3 coordfirst,
                                                       glm::vec3 coordlast, glm::vec3 texfirst, glm::vec3 texlast)
{
  Z3DTriangleList quad(GL_TRIANGLES);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> texCoords;
  std::vector<GLuint> indexes;
  GLuint idx[6] = { 0, 1, 2, 2, 1, 3 };

  bool reverse = true;
  if (alongDim == 0 && coordfirst.x > coordlast.x)
    reverse = false;
  if (alongDim == 1 && coordfirst.y > coordlast.y)
    reverse = false;
  if (alongDim == 2 && coordfirst.z > coordlast.z)
    reverse = false;

  for (int i=0; i<numSlices; ++i) {
    float factor = 0.f;
    if (numSlices > 1)
      factor = i / (numSlices-1.0);
    if (alongDim == 0) {
      vertices.push_back(glm::vec3(glm::mix(coordfirst.x, coordlast.x, factor), coordfirst[1], coordfirst[2]));
      vertices.push_back(glm::vec3(glm::mix(coordfirst.x, coordlast.x, factor), coordlast[1], coordfirst[2]));
      vertices.push_back(glm::vec3(glm::mix(coordfirst.x, coordlast.x, factor), coordfirst[1], coordlast[2]));
      vertices.push_back(glm::vec3(glm::mix(coordfirst.x, coordlast.x, factor), coordlast[1], coordlast[2]));
      texCoords.push_back(glm::vec3(glm::mix(texfirst.x, texlast.x, factor), texfirst[1], texfirst[2]));
      texCoords.push_back(glm::vec3(glm::mix(texfirst.x, texlast.x, factor), texlast[1], texfirst[2]));
      texCoords.push_back(glm::vec3(glm::mix(texfirst.x, texlast.x, factor), texfirst[1], texlast[2]));
      texCoords.push_back(glm::vec3(glm::mix(texfirst.x, texlast.x, factor), texlast[1], texlast[2]));
    } else if (alongDim == 1) {
      vertices.push_back(glm::vec3(coordfirst[0], glm::mix(coordfirst.y, coordlast.y, factor), coordfirst[2]));
      vertices.push_back(glm::vec3(coordlast[0], glm::mix(coordfirst.y, coordlast.y, factor), coordfirst[2]));
      vertices.push_back(glm::vec3(coordfirst[0], glm::mix(coordfirst.y, coordlast.y, factor), coordlast[2]));
      vertices.push_back(glm::vec3(coordlast[0], glm::mix(coordfirst.y, coordlast.y, factor), coordlast[2]));
      texCoords.push_back(glm::vec3(texfirst[0], glm::mix(texfirst.y, texlast.y, factor), texfirst[2]));
      texCoords.push_back(glm::vec3(texlast[0], glm::mix(texfirst.y, texlast.y, factor), texfirst[2]));
      texCoords.push_back(glm::vec3(texfirst[0], glm::mix(texfirst.y, texlast.y, factor), texlast[2]));
      texCoords.push_back(glm::vec3(texlast[0], glm::mix(texfirst.y, texlast.y, factor), texlast[2]));
    } else {
      vertices.push_back(glm::vec3(coordfirst[0], coordfirst[1], glm::mix(coordfirst.z, coordlast.z, factor)));
      vertices.push_back(glm::vec3(coordlast[0], coordfirst[1], glm::mix(coordfirst.z, coordlast.z, factor)));
      vertices.push_back(glm::vec3(coordfirst[0], coordlast[1], glm::mix(coordfirst.z, coordlast.z, factor)));
      vertices.push_back(glm::vec3(coordlast[0], coordlast[1], glm::mix(coordfirst.z, coordlast.z, factor)));
      texCoords.push_back(glm::vec3(texfirst[0], texfirst[1], glm::mix(texfirst.z, texlast.z, factor)));
      texCoords.push_back(glm::vec3(texlast[0], texfirst[1], glm::mix(texfirst.z, texlast.z, factor)));
      texCoords.push_back(glm::vec3(texfirst[0], texlast[1], glm::mix(texfirst.z, texlast.z, factor)));
      texCoords.push_back(glm::vec3(texlast[0], texlast[1], glm::mix(texfirst.z, texlast.z, factor)));
    }
    for (int j=0; j<6; ++j) {
      if (reverse)
        indexes.push_back(idx[5-j] + i*4);
      else
        indexes.push_back(idx[j] + i*4);
    }
  }

  quad.m_vertices.swap(vertices);
  quad.m_3DTextureCoordinates.swap(texCoords);
  quad.m_indices.swap(indexes);
  return quad;
}

void Z3DTriangleList::appendTriangle(const Z3DTriangleList &mesh, glm::uvec3 triangle)
{
  if (!m_indices.empty() || m_type != GL_TRIANGLES)
    return;

  m_vertices.push_back(mesh.m_vertices[triangle[0]]);
  m_vertices.push_back(mesh.m_vertices[triangle[1]]);
  m_vertices.push_back(mesh.m_vertices[triangle[2]]);

  if (mesh.getNum1DTextureCoordinates() > 0) {
    m_1DTextureCoordinates.push_back(mesh.m_1DTextureCoordinates[triangle[0]]);
    m_1DTextureCoordinates.push_back(mesh.m_1DTextureCoordinates[triangle[1]]);
    m_1DTextureCoordinates.push_back(mesh.m_1DTextureCoordinates[triangle[2]]);
  }

  if (mesh.getNum2DTextureCoordinates() > 0) {
    m_2DTextureCoordinates.push_back(mesh.m_2DTextureCoordinates[triangle[0]]);
    m_2DTextureCoordinates.push_back(mesh.m_2DTextureCoordinates[triangle[1]]);
    m_2DTextureCoordinates.push_back(mesh.m_2DTextureCoordinates[triangle[2]]);
  }

  if (mesh.getNum3DTextureCoordinates() > 0) {
    m_3DTextureCoordinates.push_back(mesh.m_3DTextureCoordinates[triangle[0]]);
    m_3DTextureCoordinates.push_back(mesh.m_3DTextureCoordinates[triangle[1]]);
    m_3DTextureCoordinates.push_back(mesh.m_3DTextureCoordinates[triangle[2]]);
  }

  if (mesh.getNumNormals() > 0) {
    m_normals.push_back(mesh.m_normals[triangle[0]]);
    m_normals.push_back(mesh.m_normals[triangle[1]]);
    m_normals.push_back(mesh.m_normals[triangle[2]]);
  }

  if (mesh.getNumColors() > 0) {
    m_colors.push_back(mesh.m_colors[triangle[0]]);
    m_colors.push_back(mesh.m_colors[triangle[1]]);
    m_colors.push_back(mesh.m_colors[triangle[2]]);
  }
}

ZINTERFACE_DEFINE_CLASS_NAME(Z3DTriangleList)

