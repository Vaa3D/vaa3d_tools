#include "z3dtexture.h"
#include "QsLog.h"
#include "z3dgpuinfo.h"


Z3DTexture::Z3DTexture(const glm::ivec3 &dimensions, GLenum dataFormat, GLint internalformat, GLenum dataType)
  : m_dimensions(dimensions)
  , m_dataFormat(dataFormat)
  , m_internalFormat(internalformat)
  , m_dataType(dataType)
  , m_minFilter(GL_LINEAR)
  , m_magFilter(GL_LINEAR)
  , m_wrap(GL_CLAMP_TO_EDGE)
  , m_id(0)
  , m_data(NULL)
{
  deriveTextureTarget();
  init();
}

Z3DTexture::Z3DTexture(const glm::ivec3 &dimensions, GLenum dataFormat, GLint internalformat, GLenum dataType,
                       GLint minFilter, GLint magFilter, GLint wrap)
  : m_dimensions(dimensions)
  , m_dataFormat(dataFormat)
  , m_internalFormat(internalformat)
  , m_dataType(dataType)
  , m_minFilter(minFilter)
  , m_magFilter(magFilter)
  , m_wrap(wrap)
  , m_id(0)
  , m_data(NULL)
{
  deriveTextureTarget();
  init();
}

Z3DTexture::Z3DTexture(const glm::ivec3 &dimensions, GLenum textureTarget, GLenum dataFormat, GLint internalformat,
                       GLenum dataType)
  : m_dimensions(dimensions)
  , m_textureTarget(textureTarget)
  , m_dataFormat(dataFormat)
  , m_internalFormat(internalformat)
  , m_dataType(dataType)
  , m_minFilter(GL_LINEAR)
  , m_magFilter(GL_LINEAR)
  , m_wrap(GL_CLAMP_TO_EDGE)
  , m_id(0)
  , m_data(NULL)
{
  init();
}

Z3DTexture::Z3DTexture(const glm::ivec3 &dimensions, GLenum textureTarget, GLenum dataFormat, GLint internalformat,
                       GLenum dataType, GLint minFilter, GLint magFilter, GLint wrap)
  : m_dimensions(dimensions)
  , m_textureTarget(textureTarget)
  , m_dataFormat(dataFormat)
  , m_internalFormat(internalformat)
  , m_dataType(dataType)
  , m_minFilter(minFilter)
  , m_magFilter(magFilter)
  , m_wrap(wrap)
  , m_id(0)
  , m_data(NULL)
{
  init();
}

Z3DTexture::~Z3DTexture()
{
  if (m_id)
    glDeleteTextures(1, &m_id);
}

void Z3DTexture::setFilter(GLint minFilter, GLint magFilter)
{
  bool generateMipmap = false;
  if ((m_minFilter == GL_LINEAR || m_minFilter == GL_NEAREST) &&
      (minFilter != GL_LINEAR && minFilter != GL_NEAREST))
    generateMipmap = true;
  m_minFilter = minFilter;
  m_magFilter = magFilter;
  bind();
  if (generateMipmap)
    glGenerateMipmap(m_textureTarget);
  applyFilter();
}

void Z3DTexture::setMinFilter(GLint minFilter)
{
  bool generateMipmap = false;
  if ((m_minFilter == GL_LINEAR || m_minFilter == GL_NEAREST) &&
      (minFilter != GL_LINEAR && minFilter != GL_NEAREST))
    generateMipmap = true;
  m_minFilter = minFilter;
  bind();
  if (generateMipmap)
    glGenerateMipmap(m_textureTarget);
  applyFilter();
}

void Z3DTexture::setMagFilter(GLint magFilter)
{
  m_magFilter = magFilter;
  bind();
  applyFilter();
}

void Z3DTexture::setWrap(GLint wrap)
{
  m_wrap = wrap;
  bind();
  applyWrap();
}

void Z3DTexture::uploadTexture()
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  bind();

  if (is1DTexture())
    glTexImage1D(m_textureTarget, 0, m_internalFormat,
                 m_dimensions.x, 0,
                 m_dataFormat, m_dataType, m_data);
  else if (is2DTexture())
    glTexImage2D(m_textureTarget, 0, m_internalFormat,
                 m_dimensions.x, m_dimensions.y, 0,
                 m_dataFormat, m_dataType, m_data);
  else if (is3DTexture())
    glTexImage3D(m_textureTarget, 0, m_internalFormat,
                 m_dimensions.x, m_dimensions.y, m_dimensions.z, 0,
                 m_dataFormat, m_dataType, m_data);

  if (useMipmap())
    glGenerateMipmap(m_textureTarget);

  CHECK_GL_ERROR;
}

bool Z3DTexture::is1DTexture() const
{
  return m_textureTarget == GL_TEXTURE_1D ||
      m_textureTarget == GL_PROXY_TEXTURE_1D;
}

bool Z3DTexture::is2DTexture() const
{
  GLenum all_2dtexture_targets[] = {
    GL_TEXTURE_2D,
    GL_TEXTURE_RECTANGLE,
    GL_PROXY_TEXTURE_2D,
    GL_TEXTURE_1D_ARRAY,
    GL_PROXY_TEXTURE_1D_ARRAY,
    GL_PROXY_TEXTURE_RECTANGLE,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    GL_PROXY_TEXTURE_CUBE_MAP};
  for (int i=0; i<13; ++i) {
    if (m_textureTarget == all_2dtexture_targets[i])
      return true;
  }
  return false;
}

bool Z3DTexture::is3DTexture() const
{
  return m_textureTarget == GL_TEXTURE_3D ||
      m_textureTarget == GL_PROXY_TEXTURE_3D ||
      m_textureTarget == GL_TEXTURE_2D_ARRAY ||
      m_textureTarget == GL_PROXY_TEXTURE_2D_ARRAY;
}

GLubyte *Z3DTexture::downloadTextureToBuffer(GLint dataFormat, GLenum dataType, size_t *bufferSize) const
{
  bind();

  GLubyte* data = new GLubyte[getBypePerPixel(dataFormat, dataType) * getNumPixels()];
  glGetTexImage(m_textureTarget, 0, dataFormat, dataType, data);

  if (bufferSize != NULL) {
    *bufferSize = getBypePerPixel(dataFormat, dataType) * getNumPixels();
  }
  return data;
}

int Z3DTexture::getTextureSizeOnGPU() const
{
  return getBypePerPixel(m_internalFormat) * getNumPixels();
}

void Z3DTexture::deriveTextureTarget()
{
  if (m_dimensions.z == 1) {
    if (m_dimensions.y == 1  || m_dimensions.x == 1)
      m_textureTarget = GL_TEXTURE_1D;
    else
      m_textureTarget = GL_TEXTURE_2D;
  } else {
    m_textureTarget = GL_TEXTURE_3D;
  }
}

void Z3DTexture::applyFilter()
{
  glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, m_magFilter);
  glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, m_minFilter);
}

void Z3DTexture::applyWrap()
{
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, m_wrap);
  if (m_textureTarget == GL_TEXTURE_2D || m_textureTarget == GL_TEXTURE_3D ||
      m_textureTarget == GL_TEXTURE_RECTANGLE)
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, m_wrap);
  if (m_textureTarget == GL_TEXTURE_3D)
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_R, m_wrap);
}

bool Z3DTexture::useMipmap() const
{
  if (m_minFilter == GL_LINEAR || m_minFilter == GL_NEAREST)
    return false;
  else
    return true;
}

void Z3DTexture::init()
{
  glGenTextures(1, &m_id);

  bind();
  applyFilter();
  applyWrap();
  CHECK_GL_ERROR;
}

int Z3DTexture::getBypePerPixel(GLint dataFormat, GLenum dataType)
{
  int numChannels = 0;
  switch (dataFormat) {
  case 1:
  case GL_COLOR_INDEX:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE:
  case GL_ALPHA:
  case GL_INTENSITY:
  case GL_LUMINANCE:
  case GL_DEPTH_COMPONENT:
  case GL_ALPHA_INTEGER_EXT:
    numChannels = 1;
    break;

  case 2:
  case GL_LUMINANCE_ALPHA:
    numChannels = 2;
    break;

  case 3:
  case GL_RGB:
  case GL_BGR:
    numChannels = 3;
    break;

  case 4:
  case GL_RGBA:
  case GL_BGRA:
  case GL_RGBA16:
  case GL_RGBA16F_ARB:
    numChannels = 4;
    break;

  default:
    LWARN() << "unknown data format";
  }

  int typeSize = 0;
  switch (dataType) {
  case GL_BYTE:
  case GL_UNSIGNED_BYTE:
  case GL_UNSIGNED_INT_8_8_8_8_REV:
  case GL_UNSIGNED_INT_8_8_8_8:
    typeSize = 1;
    break;

  case GL_SHORT:
  case GL_UNSIGNED_SHORT:
    typeSize = 2;
    break;

  case GL_INT:
  case GL_UNSIGNED_INT:
  case GL_FLOAT:
    typeSize = 4;
    break;

  default:
    LWARN() << "unknown data type";
  }

  return typeSize * numChannels;
}

int Z3DTexture::getBypePerPixel(GLint internalFormat)
{
  int bpp = 0;
  switch (internalFormat) {
  case 1:
  case GL_COLOR_INDEX:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE:
  case GL_ALPHA:
  case GL_R8:
  case GL_R8_SNORM:
  case GL_INTENSITY:
  case GL_LUMINANCE:
  case GL_DEPTH_COMPONENT:
  case GL_R3_G3_B2:
  case GL_RGBA2:
  case GL_R8I:
  case GL_R8UI:
    bpp = 1;
    break;

  case 2:
  case GL_LUMINANCE_ALPHA:
  case GL_INTENSITY16:
  case GL_R16:
  case GL_R16F:
  case GL_R16_SNORM:
  case GL_RG8:
  case GL_RG8_SNORM:
  case GL_DEPTH_COMPONENT16:
  case GL_RGBA4:
  case GL_R16I:
  case GL_R16UI:
  case GL_RG8I:
  case GL_RG8UI:
    bpp = 2;
    break;

  case GL_RGB:
  case GL_BGR:
  case GL_RGB8:
  case GL_RGB8I:
  case GL_RGB8UI:
  case GL_SRGB8:
  case GL_RGB8_SNORM:
  case GL_DEPTH_COMPONENT24:
    bpp = 3;
    break;

  case GL_RGBA:
  case GL_RGBA8:
  case GL_RGBA8_SNORM:
  case GL_BGRA:
  case GL_DEPTH_COMPONENT32:
  case GL_DEPTH_COMPONENT32F:
  case GL_R32F:
  case GL_RG16:
  case GL_RG16F:
  case GL_RG16_SNORM:
  case GL_SRGB8_ALPHA8:
  case GL_R32I:
  case GL_R32UI:
  case GL_RG16I:
  case GL_RG16UI:
  case GL_RGBA8I:
  case GL_RGBA8UI:
    bpp = 4;
    break;

  case GL_RGB16:
  case GL_RGB16I:
  case GL_RGB16UI:
  case GL_RGB16F:
  case GL_RGB16_SNORM:
    bpp = 6;
    break;

  case GL_RGBA16:
  case GL_RGBA16F:
  case GL_RGBA16I:
  case GL_RGBA16UI:
  case GL_RG32F:
  case GL_RG32I:
  case GL_RG32UI:
    bpp = 8;
    break;

  case GL_RGB32F:
  case GL_RGB32I:
  case GL_RGB32UI:
    bpp = 12;
    break;

  case GL_RGBA32I:
  case GL_RGBA32UI:
  case GL_RGBA32F:
    bpp = 16;
    break;

  default:
    LWARN() << "unknown internal format";
    break;
  }

  return bpp;
}

Z3DTextureUnitManager::Z3DTextureUnitManager()
  : m_maxTextureUnits(Z3DGpuInfoInstance.getMaxCombinedTextureImageUnits())
  , m_currentUnitNumber(-1)
{
}

Z3DTextureUnitManager::~Z3DTextureUnitManager()
{
}

void Z3DTextureUnitManager::nextAvailableUnit()
{
  ++m_currentUnitNumber;
  if (m_currentUnitNumber >= m_maxTextureUnits) {
    LERROR() << "No more avalable texture units!";
  }
}

void Z3DTextureUnitManager::activateCurrentUnit()
{
  if (m_currentUnitNumber < 0) {
    LERROR() << "Call nextAvailableUnit() to get a valid unit first!";
  } else if (m_currentUnitNumber >= m_maxTextureUnits) {
    LERROR() << "Exceed max number of texture units.";
  }
  glActiveTexture(getCurrentUnitEnum());
  CHECK_GL_ERROR;
}

GLint Z3DTextureUnitManager::getActiveTextureUnit()
{
  GLint i;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &i);
  return i;
}

