#include "z3dvolume.h"
#include <algorithm>
#include <QString>
#include "QsLog.h"
#include "z3dshaderprogram.h"
#include "z3dgpuinfo.h"
#include "z3dtexture.h"

Z3DVolume::Z3DVolume(Stack *stack, const glm::vec3 &downsampleSpacing, const glm::vec3 &scaleSpacing,
                     const glm::vec3 &offset, const glm::mat4 &transformation, QObject *parent)
  : QObject(parent)
  , m_stack(stack)
  , m_histogramMaxValue(-1)
  , m_texture(NULL)
  , m_volColor(1.f,1.f,1.f)
  , m_histogramThread(NULL)
{
  m_dimensions = glm::svec3(m_stack->width, m_stack->height, m_stack->depth);
  m_detailVolumeDimensions = glm::round(glm::vec3(m_dimensions) * downsampleSpacing);
  m_parentVolumeDimensions = m_detailVolumeDimensions;
  m_parentVolumeOffset = offset;
  m_data.array = m_stack->array;
  setDownsampleSpacing(downsampleSpacing);
  setScaleSpacing(scaleSpacing);
  setOffset(offset);
  setPhysicalToWorldMatrix(transformation);
  computeMaxValue();
  computeMinValue();
}

Z3DVolume::~Z3DVolume()
{
  if (m_histogramThread) {
    if (m_histogramThread->isRunning()) {
      m_histogramThread->wait();
    }
    delete m_histogramThread;
  }
  delete m_texture;
  Kill_Stack(m_stack);
}

int Z3DVolume::getBitsStored() const
{
  if (m_stack->kind == GREY)
    return 8;
  else if (m_stack->kind == GREY16) {
    if (getMaxValue() >= 4096)
      return 12;
    else
      return 16;
  } else if (m_stack->kind == FLOAT32)
    return 32;
  return 0;
}

size_t Z3DVolume::getNumVoxels() const
{
  return (size_t)m_stack->depth * (size_t)m_stack->height * (size_t)m_stack->width;
}

QString Z3DVolume::getSamplerType() const
{
  if (m_dimensions.z > 1)
    return "sampler3D";
  else if (m_dimensions.y > 1 && m_dimensions.x > 1)
    return "sampler2D";
  else
    return "sampler1D";
}

double Z3DVolume::getFloatMinValue() const
{
  if (getBitsStored() <= 16)
    return m_minValue / ((1 << getBitsStored()) - 1);
  else
    return getMinValue();   // already float image
}

double Z3DVolume::getFloatMaxValue() const
{
  if (getBitsStored() <= 16)
    return m_maxValue / ((1 << getBitsStored()) - 1);
  else
    return getMaxValue();   // already float image
}

double Z3DVolume::getValue(int x, int y, int z) const
{
  size_t stride_x = (size_t)m_stack->kind;
  size_t stride_y = (size_t)m_stack->kind * (size_t)m_stack->width;
  size_t stride_z = (size_t)m_stack->kind * (size_t)m_stack->width * (size_t)m_stack->height;
  if (m_stack->kind == GREY) {
    return *(m_data.array8 + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (m_stack->kind == GREY16) {
    return *(m_data.array16 + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (m_stack->kind == FLOAT32) {
    return *(m_data.array32 + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  }
  return 0.0;
}

double Z3DVolume::getValue(size_t index) const
{
  if (m_stack->kind == GREY) {
    return *(m_data.array8 + index);
  } else if (m_stack->kind == GREY16) {
    return *(m_data.array16 + index);
  } else if (m_stack->kind == FLOAT32) {
    return *(m_data.array32 + index);
  }
  return 0.0;
}

void Z3DVolume::asyncGenerateHistogram()
{
  if (hasHistogram() || m_histogramThread)
    return;
  m_histogramThread = new Z3DVolumeHistogramThread(this);
  connect(m_histogramThread, SIGNAL(finished()), this, SLOT(setHistogram()));
  m_histogramThread->start();
}

size_t Z3DVolume::getHistogramBinCount() const
{
  if (m_stack->kind == GREY) {
    return 256;
  } else if (m_stack->kind == GREY16) {
    return getBitsStored() == 16 ? 65536 : 4096;
  } else if (m_stack->kind == FLOAT32) {
    return m_histogram.size();
  }
  return 0;
}

size_t Z3DVolume::getHistogramValue(size_t index) const
{
  if (index < m_histogram.size())
    return m_histogram[index];
  else
    return 0;
}

size_t Z3DVolume::getHistogramValue(double fraction) const
{
  size_t index = static_cast<size_t>(fraction * static_cast<double>(getHistogramBinCount()-1));
  return getHistogramValue(index);
}

double Z3DVolume::getNormalizedHistogramValue(size_t index) const
{
  return static_cast<double>(getHistogramValue(index)) / m_histogramMaxValue;
}

double Z3DVolume::getNormalizedHistogramValue(double fraction) const
{
  size_t index = static_cast<size_t>(fraction * static_cast<double>(getHistogramBinCount()-1));
  return getNormalizedHistogramValue(index);
}

double Z3DVolume::getLogNormalizedHistogramValue(size_t index) const
{
  return std::log(static_cast<double>(getHistogramValue(index))+1.0) / std::log(m_histogramMaxValue+1.0);
}

double Z3DVolume::getLogNormalizedHistogramValue(double fraction) const
{
  size_t index = static_cast<size_t>(fraction * static_cast<double>(getHistogramBinCount()-1));
  return getLogNormalizedHistogramValue(index);
}

Z3DTexture *Z3DVolume::getTexture()
{
  if (m_texture)
    return m_texture;
  else {
    generateTexture();
    return m_texture;   // might still be null
  }
}

void Z3DVolume::setUniform(Z3DShaderProgram &shader, const QString &uniform, const GLint texUnit) const
{
  shader.setUniformValue(uniform + ".volume", texUnit);

  shader.setLogUniformLocationError(false);

  shader.setUniformValue(uniform + ".color", m_volColor);
  // volume size
  glm::vec3 dims = glm::vec3(getDimensions());
  shader.setUniformValue(uniform + ".dimensions", dims);
  shader.setUniformValue(uniform + ".dimensions_RCP", glm::vec3(1.f) / dims);

  // volume spacing, i.e. voxel size
  glm::vec3 spacs = getSpacing();
  shader.setUniformValue(uniform + ".spacing", spacs);
  shader.setUniformValue(uniform + ".spacing_RCP", glm::vec3(1.f) / spacs);

  // volume's size in its physical coordinates
  glm::vec3 cubeSize = getCubeSize();
  shader.setUniformValue(uniform + ".cube_size", cubeSize);
  shader.setUniformValue(uniform + ".cube_size_RCP", glm::vec3(1.f) / cubeSize);

  // volume offset and texture translation for slicing
  glm::vec3 offset = getOffset();
  shader.setUniformValue(uniform + ".offset", offset);

  // volume's transformation matrix
  shader.setUniformValue(uniform + ".transformation", getPhysicalToWorldMatrix());
  glm::mat4 invTm = getWorldToPhysicalMatrix();
  shader.setUniformValue(uniform + ".transformation_inverse", invTm);

  CHECK_GL_ERROR;

  shader.setLogUniformLocationError(true);
}

std::vector<double> Z3DVolume::getPhysicalBoundBox() const
{
  glm::vec3 luf = getPhysicalLUF();
  glm::vec3 rdb = getPhysicalRDB();
  std::vector<double> res(6);
  res[0] = luf.x;
  res[1] = rdb.x;
  res[2] = luf.y;
  res[3] = rdb.y;
  res[4] = luf.z;
  res[5] = rdb.z;
  return res;
}

std::vector<double> Z3DVolume::getWorldBoundBox() const
{
  if (m_hasTransformMatrix) {
    std::vector<double> res(6);
    glm::vec3 minCoord = glm::min(getWorldLUF(), getWorldRDB());
    glm::vec3 maxCoord = glm::max(getWorldLUF(), getWorldRDB());

    minCoord = glm::min(minCoord, getWorldLDB());
    maxCoord = glm::max(maxCoord, getWorldLDB());

    minCoord = glm::min(minCoord, getWorldLDF());
    maxCoord = glm::max(maxCoord, getWorldLDF());

    minCoord = glm::min(minCoord, getWorldLUB());
    maxCoord = glm::max(maxCoord, getWorldLUB());

    minCoord = glm::min(minCoord, getWorldRUF());
    maxCoord = glm::max(maxCoord, getWorldRUF());

    minCoord = glm::min(minCoord, getWorldRDF());
    maxCoord = glm::max(maxCoord, getWorldRDF());

    minCoord = glm::min(minCoord, getWorldRUB());
    maxCoord = glm::max(maxCoord, getWorldRUB());

    res[0] = minCoord.x;
    res[1] = maxCoord.x;
    res[2] = minCoord.y;
    res[3] = maxCoord.y;
    res[4] = minCoord.z;
    res[5] = maxCoord.z;

    return res;
  } else {
    return getPhysicalBoundBox();
  }
}

void Z3DVolume::setPhysicalToWorldMatrix(const glm::mat4 &transformationMatrix)
{
  m_transformationMatrix = transformationMatrix;
  if (m_transformationMatrix != glm::mat4())
    m_hasTransformMatrix = true;
}

glm::mat4 Z3DVolume::getVoxelToWorldMatrix() const
{
  return getPhysicalToWorldMatrix() * getVoxelToPhysicalMatrix();
}

glm::mat4 Z3DVolume::getWorldToVoxelMatrix() const
{
  return glm::inverse(getVoxelToWorldMatrix());
}

glm::mat4 Z3DVolume::getWorldToTextureMatrix() const
{
  return glm::inverse(getTextureToWorldMatrix());
}

glm::mat4 Z3DVolume::getTextureToWorldMatrix() const
{
  return getVoxelToWorldMatrix() * getTextureToVoxelMatrix();
}

glm::mat4 Z3DVolume::getVoxelToPhysicalMatrix() const
{
  // 1. Multiply by spacing 2. Apply offset
  glm::mat4 scale = glm::scale(glm::mat4(), getSpacing());
  return glm::translate(scale, getOffset());
}

glm::mat4 Z3DVolume::getPhysicalToVoxelMatrix() const
{
  glm::mat4 translate = glm::translate(glm::mat4(), -getOffset());
  return glm::scale(translate, 1.f/getSpacing());
}

glm::mat4 Z3DVolume::getWorldToPhysicalMatrix() const
{
  return glm::inverse(getPhysicalToWorldMatrix());
}

glm::mat4 Z3DVolume::getTextureToPhysicalMatrix() const
{
  return getVoxelToPhysicalMatrix() * getTextureToVoxelMatrix();
}

glm::mat4 Z3DVolume::getPhysicalToTextureMatrix() const
{
  return getVoxelToTextureMatrix() * getPhysicalToVoxelMatrix();
}

glm::mat4 Z3DVolume::getTextureToVoxelMatrix() const
{
  return glm::scale(glm::mat4(), glm::vec3(getDimensions()));
}

glm::mat4 Z3DVolume::getVoxelToTextureMatrix() const
{
  return glm::scale(glm::mat4(), 1.0f/glm::vec3(getDimensions()));
}

void Z3DVolume::setHistogram()
{
  m_histogram.swap(m_histogramThread->getHistogram());
  computeHistogramMaxValue();
  emit histogramFinished();
}

void Z3DVolume::generateTexture()
{
  if (getDimensions().x == 0 || getDimensions().y == 0 || getDimensions().z == 0) {
    QString message = QString("OpenGL volumes must have a size greater than 0 in all dimensions. Actual size: (%1, %2, %3)")
        .arg(m_stack->width).arg(m_stack->height).arg(m_stack->depth);
    LERROR() << message;
    return;
  }

  GLint format;
  GLint internalFormat;
  GLenum dataType;
  if (m_stack->kind == GREY) {
    format = GL_RED;
    internalFormat = GL_R8;
    dataType = GL_UNSIGNED_BYTE;
  } else if (m_stack->kind == GREY16) {
    format = GL_RED;
    internalFormat = GL_R16;
    dataType = GL_UNSIGNED_SHORT;
  } else if (m_stack->kind == FLOAT32) {
    format = GL_RED;
    internalFormat = GL_R32F;
    dataType = GL_FLOAT;
  } else {
    LERROR() << "Only GREY, GREY16 or FLOAT32 stack formats are supported";
    return;
  }

  // Create texture
  Z3DTexture* tex = new Z3DTexture(glm::ivec3(getDimensions()), format, internalFormat, dataType);

  CHECK_GL_ERROR;
  tex->setData(m_stack->array);
  tex->uploadTexture();

  CHECK_GL_ERROR;

  // append to internal data structure
  m_texture = tex;
}

void Z3DVolume::computeMinValue()
{
  if (m_stack->kind == GREY)
    m_minValue = *std::min_element(m_data.array8, m_data.array8 + getNumVoxels());
  else if (m_stack->kind == GREY16)
    m_minValue = *std::min_element(m_data.array16, m_data.array16 + getNumVoxels());
  else if (m_stack->kind == FLOAT32)
    m_minValue = *std::min_element(m_data.array32, m_data.array32 + getNumVoxels());
}

void Z3DVolume::computeMaxValue()
{
  if (m_stack->kind == GREY)
    m_maxValue = *std::max_element(m_data.array8, m_data.array8 + getNumVoxels());
  else if (m_stack->kind == GREY16)
    m_maxValue = *std::max_element(m_data.array16, m_data.array16 + getNumVoxels());
  else if (m_stack->kind == FLOAT32)
    m_maxValue = *std::max_element(m_data.array32, m_data.array32 + getNumVoxels());
}

void Z3DVolume::computeHistogramMaxValue()
{
  m_histogramMaxValue = *std::max_element(m_histogram.begin(), m_histogram.end());
}

//-----------------------------------------------------------------------------------
Z3DVolumeHistogramThread::Z3DVolumeHistogramThread(Z3DVolume *volume, QObject* parent)
  : QThread(parent)
  , m_volume(volume)
{
}

void Z3DVolumeHistogramThread::run()
{
  if (m_volume->getType() == GREY) {
    m_histogram.assign(static_cast<size_t>(m_volume->getMaxValue()+1), 0);
    for (size_t i=0; i<m_volume->getNumVoxels(); i++) {
      m_histogram[static_cast<size_t>(m_volume->getValue(i))]++;
    }
  } else if (m_volume->getType() == GREY16) {
    m_histogram.assign(static_cast<size_t>(m_volume->getMaxValue()+1), 0);
    for (size_t i=0; i<m_volume->getNumVoxels(); i++) {
      m_histogram[static_cast<size_t>(m_volume->getValue(i))]++;
    }
  } else if (m_volume->getType() == FLOAT32) {
    size_t binCount = 256;   // cover range [0.0f, 1.0f]
    m_histogram.assign(binCount, 0);
    for (size_t i=0; i<m_volume->getNumVoxels(); i++) {
      m_histogram[static_cast<size_t>(m_volume->getValue(i) * (binCount-1))]++;
    }
  }
}

void Z3DVolume::translate(double dx, double dy, double dz)
{
  m_transformationMatrix[3][0] += dx;
  m_transformationMatrix[3][1] += dy;
  m_transformationMatrix[3][2] += dz;

  if (m_transformationMatrix != glm::mat4())
    m_hasTransformMatrix = true;
}
