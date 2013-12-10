#ifndef Z3DTEXTURE_H
#define Z3DTEXTURE_H

#include "z3dgl.h"
#include "zglmutils.h"

class Z3DTexture
{
public:
  // default min and mag filter is GL_LINEAR, default wrap is GL_CLAMP_TO_EDGE

  // construct 1d, 2d or 3d texture, texture target will be derived from dimension
  // Result Z3DTexture target will be one of GL_TEXTURE_1D, GL_TEXTURE_2D or GL_TEXTURE_3D
  Z3DTexture(const glm::ivec3& dimensions, GLenum dataFormat, GLint internalformat, GLenum dataType);
  Z3DTexture(const glm::ivec3& dimensions, GLenum dataFormat, GLint internalformat, GLenum dataType,
             GLint minFilter, GLint magFilter, GLint wrap);
  // construct any texture with user provided textureTarget
  Z3DTexture(const glm::ivec3& dimensions, GLenum textureTarget, GLenum dataFormat, GLint internalformat,
             GLenum dataType);
  Z3DTexture(const glm::ivec3& dimensions, GLenum textureTarget, GLenum dataFormat, GLint internalformat,
             GLenum dataType, GLint minFilter, GLint magFilter, GLint wrap);
  ~Z3DTexture();

  // call this only if you want to upload something. ( usually openGL will allocate texture
  // memory ) Input data must match current dataFormat and dataType.
  // Z3DTexture will **not** take ownership of the input data
  // call this before uploadTexture
  void setData(void *data) { m_data = (GLvoid*)data; }

  void uploadTexture();
  void bind() const { glBindTexture(m_textureTarget, m_id); }

  GLuint getId() const { return m_id; }
  // Check if texture is in resident GL memory
  bool isResident() const { GLboolean res; return glAreTexturesResident(1, &m_id, &res) == GL_TRUE; }
  bool is1DTexture() const;
  bool is2DTexture() const;
  bool is3DTexture() const;

  // download texture to buffer. paramerter bufferSize is used to get returned buffer size (in byte).
  // returned buffer should be deleted by caller using delete[] buffer;
  GLubyte* downloadTextureToBuffer(GLint dataFormat, GLenum dataType, size_t *bufferSize = NULL) const;

  int getTextureSizeOnGPU() const;

  GLenum getTextureTarget() const { return m_textureTarget; }
  glm::ivec3 getDimensions() const { return m_dimensions;}
  int getWidth() const { return m_dimensions.x; }
  int getHeight() const { return m_dimensions.y; }
  int getDepth() const { return m_dimensions.z; }
  int getNumPixels() const { return m_dimensions.x * m_dimensions.y * m_dimensions.z; }
  GLint getDataFormat() const { return m_dataFormat; }
  GLint getInternalFormat() const { return m_internalFormat; }
  GLint getMinFilter() const { return m_minFilter; }
  GLint getMagFilter() const { return m_magFilter; }
  GLenum getDataType() const { return m_dataType; }

  void setFilter(GLint minFilter, GLint magFilter);
  void setMinFilter(GLint minFilter);
  void setMagFilter(GLint magFilter);
  void setWrap(GLint wrap);

  // changes made by the following four functions will take effect after next call of uploadTexture()
  void setDimensions(glm::ivec3 dimensions) { m_dimensions = dimensions; }
  void setDataFormat(GLint format) { m_dataFormat = format; }
  void setInternalFormat(GLint internalformat) { m_internalFormat = internalformat; }
  void setDataType(GLenum dataType) { m_dataType = dataType; }

protected:
  glm::ivec3 m_dimensions;
  GLenum m_textureTarget;
  GLenum m_dataFormat;
  GLint m_internalFormat;
  GLenum m_dataType;

  GLint m_minFilter;
  GLint m_magFilter;
  GLint m_wrap;

  GLuint m_id; // texture id

  GLvoid *m_data;

private:
  void deriveTextureTarget();
  void applyFilter();
  void applyWrap();
  bool useMipmap() const;
  void init();

  // calculates the bytes per pixel from dataFormat and dataType
  static int getBypePerPixel(GLint dataFormat, GLenum dataType);
  // calculates the bytes per pixel from the internal format
  static int getBypePerPixel(GLint internalFormat);
};

// provide unique texture units
// usage:

// Z3DTextureUnitManager *textureUnitManager = new Z3DTextureUnitManager();
// textureUnitManager->nextAvailableUnit();
// textureUnitManager->activateCurrentUnit();
// texture->bind();

// textureUnitManager->nextAvailableUnit();
// ...

// delete textureUnitManager;
class Z3DTextureUnitManager
{
public:
  Z3DTextureUnitManager();
  ~Z3DTextureUnitManager();

  void nextAvailableUnit();
  void activateCurrentUnit();
  GLint getCurrentUnitNumber() const { return m_currentUnitNumber; }
  GLenum getCurrentUnitEnum() const { return GL_TEXTURE0 + m_currentUnitNumber; }
  // clear assigned unit
  void reset() { m_currentUnitNumber = -1; }

  static GLint getActiveTextureUnit();

protected:
  int m_maxTextureUnits;   // total number of available units
  GLint m_currentUnitNumber;
};

#endif // Z3DTEXTURE_H
