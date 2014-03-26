#include "z3dtransferfunction.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"
#include "z3dtransferfunctionwidgetwitheditorwindow.h"
#include "z3dvolume.h"
#include "z3dtexture.h"
#include <QLabel>

Z3DTransferFunction::Z3DTransferFunction(double min, double max, const glm::col4 &minColor,
                                         const glm::col4 &maxColor, int width, QObject *parent)
  : ZColorMap(min, max, minColor, maxColor, parent)
  , m_texture(NULL)
  , m_dimensions(width, 1, 1)
  , m_textureFormat(GL_BGRA)
  , m_textureDataType(GL_UNSIGNED_INT_8_8_8_8_REV)
  , m_textureIsInvalid(true)
{
  connect(this, SIGNAL(changed()), this, SLOT(invalidateTexture()));
}

Z3DTransferFunction::Z3DTransferFunction(const Z3DTransferFunction& tf)
  : ZColorMap(tf)
  , m_texture(NULL)
  , m_dimensions(tf.m_dimensions)
  , m_textureFormat(tf.m_textureFormat)
  , m_textureDataType(tf.m_textureDataType)
  , m_textureIsInvalid(true)
{
  connect(this, SIGNAL(changed()), this, SLOT(invalidateTexture()));
}

Z3DTransferFunction::~Z3DTransferFunction()
{
  delete m_texture;
}

void Z3DTransferFunction::swap(Z3DTransferFunction &other)
{
  ZColorMap::swap(other);
  std::swap(m_texture, other.m_texture);
  std::swap(m_dimensions, other.m_dimensions);
  std::swap(m_textureFormat, other.m_textureFormat);
  std::swap(m_textureDataType, other.m_textureDataType);
  std::swap(m_textureIsInvalid, other.m_textureIsInvalid);
}

Z3DTransferFunction &Z3DTransferFunction::operator =(Z3DTransferFunction other)
{
  swap(other);
  return *this;
}

bool Z3DTransferFunction::operator==(const Z3DTransferFunction& tf) const
{
  if (!ZColorMap::equalTo(tf))
    return false;
  if (m_dimensions != tf.m_dimensions)
    return false;
  if (m_textureDataType != tf.m_textureDataType)
    return false;
  if (m_textureFormat != tf.m_textureFormat)
    return false;

  return true;
}

bool Z3DTransferFunction::operator!=(const Z3DTransferFunction& tf) const
{
  return !(*this == tf);
}

void Z3DTransferFunction::resetToDefault()
{
  reset(0., 1., glm::col4(0,0,0,0), glm::col4(255,255,255,255));
  emit changed();
}

void Z3DTransferFunction::createTexture()
{
  delete m_texture;

  m_texture = new Z3DTexture(m_dimensions, m_textureFormat, GL_RGBA8, m_textureDataType);
  CHECK_GL_ERROR;
}

Z3DTexture* Z3DTransferFunction::getTexture()
{
  if (m_textureIsInvalid)
    updateTexture();

  return m_texture;
}

QString Z3DTransferFunction::getSamplerType() const
{
  if (m_dimensions.z > 1)
    return "sampler3D";
  else if (m_dimensions.y > 1)
    return "sampler2D";
  else
    return "sampler1D";
}

void Z3DTransferFunction::resize(int width)
{
  fitDimensions(width, m_dimensions.y, m_dimensions.z);

  if (width != m_dimensions.x) {
    m_dimensions.x = width;
    emit changed();
  }
}

void Z3DTransferFunction::fitDimensions(int& width, int& height, int& depth) const
{
  int maxTexSize;
  if (depth == 1)
    maxTexSize = Z3DGpuInfoInstance.getMaxTextureSize();
  else
    maxTexSize = Z3DGpuInfoInstance.getMax3DTextureSize();

  if (maxTexSize < width)
    width = maxTexSize;

  if (maxTexSize < height)
    height = maxTexSize;

  if (maxTexSize < depth)
    depth = maxTexSize;
}

void Z3DTransferFunction::updateTexture()
{
  if (!m_texture || (m_texture->getDimensions() != m_dimensions))
    createTexture();
  assert(m_texture);

  std::vector<glm::col4> tfData(m_dimensions.x);
  for (size_t x = 0; x < tfData.size(); ++x)
    tfData[x] = getMappedColorBGRA(static_cast<double>(x) / (tfData.size()-1));
  m_texture->setData(&tfData[0]);

  m_texture->uploadTexture();
  CHECK_GL_ERROR;

  m_textureIsInvalid = false;
}

bool Z3DTransferFunction::isValidDomainMin(double min) const
{
  if (min < getDomainMax() && min >= 0.0 && min < 1.0)
    return true;
  else
    return false;
}

bool Z3DTransferFunction::isValidDomainMax(double max) const
{
  if (max > getDomainMin() && max > 0.0 && max <= 1.0)
    return true;
  else
    return false;
}

Z3DTransferFunctionParameter::Z3DTransferFunctionParameter(const QString &name, double min, double max, const glm::col4 &minColor,
                                                           const glm::col4 &maxColor, int width, QObject *parent)
  : ZSingleValueParameter<Z3DTransferFunction>(name, parent)
  , m_volume(NULL)
{
  m_value = Z3DTransferFunction(min, max, minColor, maxColor, width);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

void Z3DTransferFunctionParameter::setVolume(Z3DVolume *volume)
{
  if (m_volume != volume) {
    m_volume = volume;
    if (m_volume) {
      // Resize texture of tf according to bitdepth of volume
      int bits = m_volume->getBitsStored();
      if (bits > 16)
        bits = 16; // handle float data as if it was 16 bit to prevent overflow

      int max = 1 << bits;
      m_value.resize(max);
    }
    emit valueChanged();
  }
}

QWidget *Z3DTransferFunctionParameter::actualCreateWidget(QWidget *parent)
{
  return new Z3DTransferFunctionWidgetWithEditorWindow(this, m_mainWin, parent);
}
